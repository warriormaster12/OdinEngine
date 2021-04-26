#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUv;
layout (location = 0) out vec4 outFragColor;

layout(set = 2, binding = 0) uniform TriangleData
{
    vec4 color;
}triangleData;

layout(set = 2, binding = 1) uniform sampler2D albedoMap;
void main()
{
    
    vec4 albedo = texture(albedoMap, inUv);
    //return color
	outFragColor = vec4(albedo.rgb,1.0f);   
}
