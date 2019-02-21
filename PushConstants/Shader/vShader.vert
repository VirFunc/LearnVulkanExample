#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
}MVP;

layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 fragColor;

#define PUSH_CONSTANT_COUNT 3
layout(push_constant) uniform pushConstants{
	vec4 color1;
	vec4 color2;
	vec4 color3;
}pushConsts;

void main() 
{
    gl_Position = MVP.proj * MVP.view * MVP.model * vec4(inPosition, 1.0f);
    fragColor = vec3(pushConsts.color3);
}