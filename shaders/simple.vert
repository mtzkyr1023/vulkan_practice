#version 450

void main() {
	const vec2 vertIndex = vec2(gl_VertexIndex / 2, gl_VertexIndex % 2);
	gl_Position = vec4(vertIndex * 2.0f - 1.0f, 0.0f, 1.0f);
}