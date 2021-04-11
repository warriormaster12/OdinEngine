#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;


layout(set = 0, binding = 0) uniform CameraData
{
	mat4 view;
	mat4 projection;
}cameraData;
struct ObjectData{
	mat4 model;
};
layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{   
	ObjectData objects[];
} objectBuffer;

void main()
{

	//output the position of each vertex
	gl_Position = cameraData.projection * cameraData.view * objectBuffer.objects[gl_InstanceIndex].model * vec4(inPosition, 1.0f);
	outColor = inColor;
}
