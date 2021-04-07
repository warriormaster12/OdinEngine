#version 450
layout (location = 0) in vec3 aPos;

layout (location = 0) out vec3 TexCoords;

layout(set = 0, binding = 0) uniform  CameraBuffer{   
    mat4 view;
    mat4 proj;
} cameraData;

void main()
{
    TexCoords = aPos;
    //we remove translation from view matrix 
    vec4 pos = cameraData.proj * mat4(mat3(cameraData.view)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  
