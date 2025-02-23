#version 450

layout(set=0, binding=0) uniform ViewProjMatrix {
	mat4 view;
	mat4 proj;
	mat4 world;
} ub0;


layout(location=0) in vec4 inPosition;

void main() {
	gl_Position = ub0.world * inPosition;
	gl_Position = ub0.view * gl_Position;
	gl_Position = ub0.proj * gl_Position;
}