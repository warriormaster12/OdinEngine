#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec3 outNormal;


layout(set = 0, binding = 0) uniform CameraData
{
	mat4 view;
	mat4 projection;
	mat4 projViewMatrix;
}cameraData;
struct ObjectData{
	mat4 model;
};
layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{   
	ObjectData objects[];
} objectBuffer;

void main()
{
	outUv = inUv;
	outPosition = vec3(objectBuffer.objects[gl_InstanceIndex].model * vec4(inPosition, 1.0));
	outNormal = mat3(objectBuffer.objects[gl_InstanceIndex].model) * inNormal;
	//output the position of each vertex
	gl_Position = cameraData.projViewMatrix * vec4(outPosition, 1.0);
}
