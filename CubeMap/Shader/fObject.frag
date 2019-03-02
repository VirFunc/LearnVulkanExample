#version 450
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_KHR_vulkan_glsl : enable

layout(binding=1)uniform samplerCube cubeSampler;

layout(location=0)in vec3 inNormal;
layout(location=1)in vec3 inFragPos;
layout(location=2)in vec3 inViewPos;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 normal=normalize(inNormal);
	vec3 viewDir=normalize(inViewPos-inFragPos);

	vec3 reflectDir=reflect(-viewDir,normal);
	outColor=texture(cubeSampler,reflectDir);
	//outColor=vec4(1.0f,0.0f,0.0f,1.0f);
}