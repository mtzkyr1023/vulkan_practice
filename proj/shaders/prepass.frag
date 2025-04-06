#version 450

layout(set=1,binding=0) uniform texture2D albedoTex;
layout(set=2,binding=0) uniform sampler wrapSampler;

layout(location=0) in vec2 inTexcoord;

void main()
{
	vec4 albedo = texture(sampler2D(albedoTex, wrapSampler), inTexcoord);
	
	if (albedo.a - 0.5f < 0.0f)
		discard;
}