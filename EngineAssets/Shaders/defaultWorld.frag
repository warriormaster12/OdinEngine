#version 450

#extension GL_GOOGLE_include_directive : enable

#include "test.glsl"

layout (location = 0) out vec4 outFragColor;

layout(set = 2, binding = 0) uniform MaterialData
{
    vec4 color;
    vec4 repeateCount;
}materialData;

void main()
{
    vec4 albedo;
    albedo += materialData.color;
    vec4 emission;
    emission += vec4(1.0, 0.7333, 0.0, 1.0);
    //return color
    vec4 color = outputColor(albedo);
	outFragColor = vec4(color.rgb,1.0);   
}
