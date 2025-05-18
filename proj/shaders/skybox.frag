#version 450

layout(set=1, binding=0) uniform textureCube cubeMap;
layout(set=2, binding=0) uniform sampler clampSampler;


layout(location=0) in vec3 inNormal;

layout(location=0) out vec4 outResult;

void main()
{
	vec3 color = texture(samplerCube(cubeMap, clampSampler), inNormal).rgb;
	outResult = vec4(color, 1.0f);
}