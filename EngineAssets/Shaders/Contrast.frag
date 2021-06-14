#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D samplerColor;

const float contrast = 0.3;
void main() 
{
    outFragColor = texture(samplerColor, inUV);
    outFragColor.rgb = (outFragColor.rgb - 0.5) * (1.0 + contrast) + 0.5;
}  
