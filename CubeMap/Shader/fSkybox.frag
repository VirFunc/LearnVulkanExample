#version 450
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding=1)uniform samplerCube cubeSampler;

void main() 
{
    outColor = texture(cubeSampler, normalize(inTexCoord));
}