#version 450

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform TriangleData
{
    vec4 color;
}triangleData;

void main()
{
    //return color
	outFragColor = vec4(vec3(triangleData.color),1.0f);   
}
