#version 450

layout(set=0, binding=0, input_attachment_index = 0) uniform subpassInput index0;
layout(set=0, binding=1, input_attachment_index = 1) uniform subpassInput index1;
layout(set=0, binding=2, input_attachment_index = 2) uniform subpassInput index2;
layout(set=0, binding=3, input_attachment_index = 3) uniform subpassInput index3;

layout(set=1, binding=0) uniform InvViewProjMatrix {
	mat4 invView;
	mat4 invProj;
} ub0;
layout(set=1, binding=1) uniform SceneInfo {
	vec4 lightVector;
	vec4 cameraPosition;
	vec4 screenInfo;
} ub1;
layout(set=1, binding=2) uniform ShadowMatrix {
	mat4 shadowView;
	mat4 shadowProj;
	vec4 sceneInfo;
} ub2;

layout(set=2, binding=0) uniform texture2D shadowMap;
layout(set=2, binding=1) uniform textureCube cubeMap;
layout(set=3, binding=0) uniform sampler clampSampler;

layout(location=0) out vec4 outResult;

const float PI = 3.141592653589f;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float codTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float sampleShadowMap(vec3 worldPosition, float NdotL, vec2 offset);

void main()
{
	vec4 albedo = subpassLoad(index0);
	vec4 normalDepth = subpassLoad(index1);
	vec4 roughMetalVelocity = subpassLoad(index2);
	float ao = albedo.a;
	
	float depth = subpassLoad(index3).x;
	
	vec4 screenCoord = vec4(gl_FragCoord.xy / ub1.screenInfo.xy * 2.0f - 1.0f, depth, 1.0f);
	
	vec4 viewPosition = ub0.invProj * screenCoord;
	vec4 worldPosition = ub0.invView * viewPosition;
	worldPosition /= worldPosition.w;
	
	vec3 N = normalDepth.xyz;
	vec3 V = normalize(ub1.cameraPosition.xyz - worldPosition.xyz);
	vec3 R = reflect(-V, N);
	
	float roughness = roughMetalVelocity.x;
	float metalic = roughMetalVelocity.y;
	
	vec3 F0 = vec3(0.01f);
	F0 = mix(F0, albedo.rgb, metalic);
	
	vec3 L = normalize(ub1.lightVector.xyz);
	vec3 H = normalize(V + L);
	
	float NDF = distributionGGX(N, H, roughness);
	float G = geometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0f) - kS;
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
	vec3 specular = numerator / denominator;
	
	float shadow = 0.0f;
	
	float NdotL = max(dot(N, L), 0.0f);
	
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			shadow += sampleShadowMap(worldPosition.xyz, NdotL, vec2(i, j) / 2048.0f) / 9.0f;
		}
	}
	
	shadow = sampleShadowMap(worldPosition.xyz, NdotL, vec2(0.0f, 0.0f));
	
	int miplevel = textureQueryLevels(samplerCube(cubeMap, clampSampler));
	vec3 irradiance = textureLod(samplerCube(cubeMap, clampSampler), N, 1.0f * float(miplevel)).rgb;
	
	specular = textureLod(samplerCube(cubeMap, clampSampler), R, roughness * float(miplevel)).rgb;
	
	vec3 diffuse = irradiance * albedo.rgb;
	
	vec3 color = (kD * albedo.rgb / PI + specular) * vec3(4.0f) * NdotL * shadow;
    
	kS = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
	kD = vec3(1.0f) - kS;
	kD *= 1.0f - metalic;
	
    vec3 ambient = max((kD * diffuse), vec3(0.0f));
	
	color = ambient + specular * kS;
	
	outResult = vec4(color, 1.0f);
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;
	
	return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;
	
	float num = NdotV;
	float denom = NdotV * (1.0f - k) + k;
	
	return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

float sampleShadowMap(vec3 worldPosition, float NdotL, vec2 offset)
{
	vec4 shadowCoord = ub2.shadowView * vec4(worldPosition, 1.0f);
	shadowCoord = ub2.shadowProj * shadowCoord;
	shadowCoord /= shadowCoord.w;
	//shadowCoord.z /= (ub2.sceneInfo.w - ub2.sceneInfo.z);
	
	float z = shadowCoord.z;
	
	vec2 texcoord = shadowCoord.xy * vec2(0.5f, 0.5f) + vec2(0.5f, 0.5f);
	
	vec2 shadowMapDepth = texture(sampler2D(shadowMap, clampSampler), texcoord + offset).xy;
	
	float depthSq = shadowMapDepth.x * shadowMapDepth.x;
	float variance =  shadowMapDepth.y - depthSq;
	float md = z - shadowMapDepth.x;
	float p = variance / (variance + (md * md));
	
	float bias = max(0.0001f * (1.0f - NdotL), 0.00001f);
	
	float litfactor = shadowMapDepth.x <= z ? p : 0.0f;
	
	return litfactor;
}
