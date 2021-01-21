#version 460
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 WorldPos;
layout (location = 3) out vec3 Normal;

layout(set = 0, binding = 0) uniform  CameraBuffer{   
    mat4 view;
    mat4 proj;
	mat4 viewproj;
	vec4 camPos; // vec3
} cameraData;

struct ObjectData{
	mat4 model;
}; 

//all object matrices
layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{   

	ObjectData objects[];
} objectBuffer;

//push constants block
layout( push_constant ) uniform constants
{
 vec4 data;
 mat4 render_matrix;
} PushConstants;

void main() 
{	
	WorldPos = vec3(objectBuffer.objects[gl_InstanceIndex].model * vec4(vPosition, 1.0f));
	Normal = mat3(objectBuffer.objects[gl_InstanceIndex].model) * vNormal;
	outColor = vColor;
	texCoord = vTexCoord;

	gl_Position = cameraData.viewproj * vec4(WorldPos, 1.0f);
}

