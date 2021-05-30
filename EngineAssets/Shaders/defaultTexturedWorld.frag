#version 450

#extension GL_GOOGLE_include_directive : enable

#include "PBRFunctions.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout( push_constant ) uniform constants
{
    vec4 textures[8]; //int
} PushConstants;

layout(set = 1, binding = 1) uniform MaterialData
{
    vec4 albedo; //vec3
    vec4 metallic; //float
    vec4 roughness; //float
    vec4 ao; //float

    vec4 repeateCount; //int
}materialData;

struct PointLight
{
    vec4 position; //vec3
    vec4 color; //vec3
};

layout(std430, set = 0, binding = 1) buffer LightData
{
    vec4 lightCount; //int
    vec4 camPos; //vec3
    PointLight pLights[];
}lightData;

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
