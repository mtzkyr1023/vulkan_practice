#version 450

layout(set=0, binding=0, input_attachment_index = 0) uniform subpassInput index0;
layout(set=0, binding=1, input_attachment_index = 1) uniform subpassInput index1;
layout(set=0, binding=2, input_attachment_index = 2) uniform subpassInput index2;

layout(set=1, binding=0) uniform InvViewProjMatrix {
	mat4 invView;
	mat4 invProj;
} ub0;
layout(set=1, binding=1) uniform SceneInfo {
	vec4 lightVector;
	vec4 cameraPosition;
} ub1;

layout(location=0) out vec4 outResult;

const float PI = 3.141592653589f;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
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


void main()
{
	vec4 albedo = subpassLoad(index0);
	vec4 normalDepth = subpassLoad(index1);
	vec4 roughMetalVelocity = subpassLoad(index2);
	
	float depth = normalDepth.w * -2.0f + 1.0f;
	
	vec4 screenCoord = vec4(gl_FragCoord.xy, depth, 1.0f);
	
	vec4 viewPosition = ub0.invProj * screenCoord;
	vec4 worldPosition = ub0.invView * viewPosition;
	worldPosition /= worldPosition.w;
	
	vec3 N = normalDepth.xyz;
	vec3 V = normalize(ub1.cameraPosition.xyz - worldPosition.xyz);
	
	float roughness = roughMetalVelocity.x;
	float metalic = roughMetalVelocity.y;
	
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo.rgb, metalic);
	
	vec3 L = normalize(ub1.lightVector.xyz);
	vec3 H = normalize(V + L);
	
	float NDF = distributionGGX(N, H, roughness);
	float G = geometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0f) - kS;
	kD *= 1.0f - metalic;
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
	vec3 specular = numerator / denominator;
	
	float NdotL = max(dot(N, L), 0.0f);
	vec3 color = (kD * albedo.rgb / PI + specular) * NdotL;
	
	outResult = vec4(color, 0.0f);
}