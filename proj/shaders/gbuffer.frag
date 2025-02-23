#version 450

layout(location=0) in vec3 inNormal;
layout(location=1) in vec3 inTangent;
layout(location=2) in vec3 inBinormal;
layout(location=3) in vec2 inTexcoord;

layout(location=0) out vec4 outAlbedo;
layout(location=1) out vec4 outNormalDepth;
layout(location=2) out vec4 outRoughMetalVelocity;

void main() {
	outAlbedo = vec4(inNormal, 1.0f);
	outNormalDepth = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	outRoughMetalVelocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}