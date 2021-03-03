#version 450 core
layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec3 TexCoords;


void main()
{    
    outFragColor = vec4(0.0, 0.9686, 1.0, 1.0);
}