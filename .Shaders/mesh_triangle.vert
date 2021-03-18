#version 460
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 WorldPos;
layout (location = 3) out vec3 Normal;
layout (location = 4) out vec3 outViewPos;

layout(set = 0, binding = 0) uniform  CameraBuffer{   
    mat4 view;
    mat4 proj;
	mat4 viewproj;
	vec4 camPos; // vec3
	mat4 lightSpace;
} cameraData;

struct ObjectData{
	mat4 model;
}; 

//all object matrices
layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{   

	ObjectData objects[];
} objectBuffer;

//push constants block

layout(push_constant) uniform PushConsts {
	vec4 position;
	uint cascadeIndex;
} pushConsts;

void main() 
{	
	outColor = vColor;
	Normal = vNormal;
	texCoord = vTexCoord;
	vec3 pos = vPosition + pushConsts.position.xyz;
	WorldPos = pos;
	outViewPos = (cameraData.view * vec4(pos.xyz, 1.0)).xyz;
	gl_Position = cameraData.viewproj * objectBuffer.objects[gl_InstanceIndex].model * vec4(pos.xyz, 1.0);
}
