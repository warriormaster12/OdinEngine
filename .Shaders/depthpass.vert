#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

#define SHADOW_MAP_CASCADE_COUNT 4

layout(push_constant) uniform PushConsts {
	vec4 position;
	uint cascadeIndex;
} pushConsts;

layout(set = 0, binding = 0) uniform LightMatrixData{   
	mat4[SHADOW_MAP_CASCADE_COUNT] cascadeViewProjMat;
} lightMatrixData;

layout (location = 0) out vec2 outUV;

void main()
{
	outUV = inUV;
	vec3 pos = inPos + pushConsts.position.xyz;
	gl_Position =  lightMatrixData.cascadeViewProjMat[pushConsts.cascadeIndex] * vec4(pos, 1.0);
}
