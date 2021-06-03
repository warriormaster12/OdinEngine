#version 460

layout (location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform CameraData
{
	mat4 view;
	mat4 projection;
}cameraData;


layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = inPos;

	//remove translation from view matrix
	mat4 outView = mat4(mat3(cameraData.view));

	vec4 pos = cameraData.projection * outView * vec4(inPos, 1.0);
	gl_Position = pos.xyww;
}
