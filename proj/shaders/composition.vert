#version 450

layout(location=0) out vec2 outTexcoord;

void main() {
	const vec2 vertIndex = vec2(gl_VertexIndex / 2, gl_VertexIndex % 2);
	gl_Position = vec4(vertIndex * 4.0f - 1.0f, 0.0f, 1.0f);
	
	outTexcoord = vertIndex * 2;
}