#version 450

layout(set=0, binding=0) uniform ViewProjMatrix {
	mat4 view;
	mat4 proj;
	mat4 world;
} ub0;


layout(location=0) in vec4 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTangent;
layout(location=3) in vec2 inTexcoord;


layout(location=0) out vec3 outNormal;
layout(location=1) out vec3 outTangent;
layout(location=2) out vec3 outBinormal;
layout(location=3) out vec2 outTexcoord;

void main() {
	gl_Position = ub0.world * inPosition;
	gl_Position = ub0.view * gl_Position;
	gl_Position = ub0.proj * gl_Position;
	
	outNormal = normalize(mat3(ub0.world) * inNormal);
	outTangent = normalize(mat3(ub0.world) * inTangent);
	
	outBinormal = normalize(cross(outNormal, outTangent));
	
	outTexcoord = inTexcoord;
}