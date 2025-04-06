#version 450

layout(set=0, binding=0) uniform texture2D sourceTex;
layout(set=1, binding=0) uniform sampler wrapSampler;

layout(location=0) in vec2 inTexcoord;

layout(location=0) out vec4 outResult;

void main()
{
	outResult = texture(sampler2D(sourceTex, wrapSampler), inTexcoord);
}