#version 450

layout (set = 0, binding = 1) uniform sampler2D colorMap;

layout (location = 0) in vec2 inUV;

// void main() 
// {	
// 	float alpha = texture(colorMap, inUV).a;
// 	if (alpha < 0.5) {
// 		discard;
// 	}
// } 
