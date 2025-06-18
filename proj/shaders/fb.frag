#version 450

layout(set=0, binding=0) uniform texture2D sourceTex;
layout(set=1, binding=0) uniform sampler wrapSampler;

layout(location=0) in vec2 inTexcoord;

layout(location=0) out vec4 outResult;

void main()
{
	vec3 color = texture(sampler2D(sourceTex, wrapSampler), inTexcoord).rgb;
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	vec3 num = color * (a * color + b);
	vec3 denom = color * (c * color + d) + e;


	color = clamp(num / denom, vec3(0.0f), vec3(1.0f));
	
	outResult = vec4(color, 1.0f);
}