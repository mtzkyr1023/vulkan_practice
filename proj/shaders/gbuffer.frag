#version 450

layout(set=1,binding=0) uniform texture2D albedoTex;
layout(set=1,binding=1) uniform texture2D normalTex;
layout(set=1,binding=2) uniform texture2D pbrTex;
layout(set=2,binding=0) uniform sampler wrapSampler;

layout(location=0) in vec3 inNormal;
layout(location=1) in vec3 inTangent;
layout(location=2) in vec3 inBinormal;
layout(location=3) in vec2 inTexcoord;

layout(location=0) out vec4 outAlbedo;
layout(location=1) out vec4 outNormalDepth;
layout(location=2) out vec4 outRoughMetalVelocity;

void main()
{
	outAlbedo = texture(sampler2D(albedoTex, wrapSampler), inTexcoord);
	vec3 normal = texture(sampler2D(normalTex, wrapSampler), inTexcoord).xyz * 2.0f - vec3(1.0f);
	normal = normalize(inTangent * normal.x + inBinormal * normal.y + inNormal * normal.z);
	outNormalDepth = vec4(normal, gl_FragCoord.z);
	outRoughMetalVelocity = vec4(texture(sampler2D(pbrTex, wrapSampler), inTexcoord).yz, 0.0f, 0.0f);
	
	
	if (outAlbedo.a - 0.5f < 0.0f)
		discard;
}