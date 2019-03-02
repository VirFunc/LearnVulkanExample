#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normal;
}mats;

layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 outTexCoord;

void main() 
{
	outTexCoord = vec3(inPosition.xy,inPosition.z-1.0f);
    gl_Position = mats.proj * mats.view * mats.model * vec4(inPosition.xy,inPosition.z-1.0f, 1.0f);
}