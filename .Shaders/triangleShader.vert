#version 450 

layout (location = 0) in vec3 inPosition;


layout(set = 1, binding = 0) uniform CameraData
{
	mat4 view;
	mat4 projection;
	mat4 model;
}cameraData;

void main()
{

	//output the position of each vertex
	gl_Position = cameraData.projection * cameraData.view * cameraData.model * vec4(inPosition, 1.0f);
}
