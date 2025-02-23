#version 450

layout(set=0, binding=0, input_attachment_index = 0) uniform subpassInput index;

layout(location=0) out vec4 outResult;

void main()
{
	outResult = subpassLoad(index);
}