#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUv;
layout (location = 0) out vec4 outFragColor;

layout(set = 2, binding = 0) uniform TriangleData
{
    vec4 color;
}triangleData;

layout(set = 2, binding = 1) uniform sampler2D textureMaps[2];
void main()
{
    
    vec4 albedo = texture(textureMaps[0], inUv);
    vec4 emission = texture(textureMaps[1], inUv);
    emission *= vec4(1.0, 0.7333, 0.0, 1.0);
    //return color
    vec4 color = albedo + emission;
	outFragColor = vec4(color.rgb,1.0);   
}
