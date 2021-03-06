#version 450 core
layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec3 TexCoords;


void main()
{   
    vec3 position = normalize(TexCoords);
    vec4 top = vec4(0.0, 0.3176, 1.0, 1.0);
    vec4 bottom = vec4(0.0, 0.851, 1.0, 1.0);
    outFragColor = vec4(mix(bottom, top, (position.y/0.6)));
}