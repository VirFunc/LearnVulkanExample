#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normal;

	vec4 viewPos;
}uniformBuffer;

layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;

layout(location=0)out vec3 outNormal;
layout(location=1)out vec3 outFragPos;
layout(location=2)out vec3 outViewPos;


void main() 
{

    gl_Position = uniformBuffer.proj * uniformBuffer.view * uniformBuffer.model * vec4(inPosition, 1.0f);

	outFragPos=mat3(uniformBuffer.model)*inPosition;
	outNormal=mat3(uniformBuffer.normal)*inNormal;
	outViewPos=vec3(uniformBuffer.viewPos);
}