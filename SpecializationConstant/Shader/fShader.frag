#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

//use specialization constant to set the color of the model
//each constant corresponds to one VkSpecializationMapEntry
layout(constant_id = 0) const float r=0.0f;
layout(constant_id = 1) const float g=0.0f;
layout(constant_id = 2) const float b=0.0f;


void main() 
{
    outColor = vec4(vec3(r,g,b) * fragColor, 1.0f);
}