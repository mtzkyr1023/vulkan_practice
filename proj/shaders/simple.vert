#version 450

layout(set=0, binding=0) uniform ViewProjMatrix {
	mat4 view;
	mat4 proj;
} ub0;

void main() {
	const vec2 vertIndex = vec2(gl_VertexIndex / 2, gl_VertexIndex % 2);
	gl_Position = vec4(vertIndex * 2.0f - 1.0f, 0.0f, 1.0f);

	gl_Position = gl_Position * ub0.view;
	gl_Position = gl_Position * ub0.proj;
}