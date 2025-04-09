#version 450

layout(set=0, binding=0) uniform ViewProjMatrix {
	mat4 view;
	mat4 proj;
	mat4 world;
	vec4 sceneInfo;
} ub0;


layout(set=0, binding=1) uniform CameraViewProjMatrix {
	mat4 view;
	mat4 proj;
	mat4 world;
} ub1;


layout(location=0) in vec4 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTangent;
layout(location=3) in vec2 inTexcoord;

layout(location=0) out vec2 outTexcoord;
layout(location=1) out float outLinearDepth;

void main() {
	gl_Position = ub0.world * inPosition;
	gl_Position = ub0.view * gl_Position;
	gl_Position = ub0.proj * gl_Position;
	
	outTexcoord = inTexcoord;
	
	outLinearDepth = gl_Position.z / gl_Position.w;
}