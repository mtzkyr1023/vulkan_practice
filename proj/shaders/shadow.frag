#version 450

layout(set=0, binding=0) uniform ViewProjMatrix {
	mat4 view;
	mat4 proj;
	mat4 world;
	vec4 sceneInfo;
} ub0;

layout(set=1,binding=0) uniform texture2D albedoTex;
layout(set=2,binding=0) uniform sampler wrapSampler;

layout(location=0) in vec2 inTexcoord;
layout(location=1) in float inLinearDepth;

layout(location=0) out vec2 shadowOutput;

void main()
{
	vec4 albedo = texture(sampler2D(albedoTex, wrapSampler), inTexcoord);
	
	float z = gl_FragCoord.z;
	
	shadowOutput = vec2(z, z * z);
	
	if (albedo.a - 0.5f < 0.0f)
		discard;
}