#version 450

#extension GL_GOOGLE_include_directive : enable

#include "test.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUv;
layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 1) uniform MaterialData
{
    vec4 color;
    vec4 repeateCount;
}materialData;

layout(set = 2, binding = 0) uniform sampler2D textureMaps[2];
void main()
{
    vec4 albedo = texture(textureMaps[0], inUv * int(materialData.repeateCount)) * materialData.color;
    vec4 emission = texture(textureMaps[1], inUv * int(materialData.repeateCount));
    emission *= vec4(1.0, 0.7333, 0.0, 1.0);
    //return color
    vec4 color = outputColor(albedo + emission);
	outFragColor = vec4(color.rgb,1.0);   
}
