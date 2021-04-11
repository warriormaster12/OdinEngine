#version 450

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 outFragColor;

layout(set = 2, binding = 0) uniform TriangleData
{
    vec4 color;
}triangleData;

void main()
{
    //return color
	outFragColor = vec4(inColor * vec3(triangleData.color),1.0f);   
}
