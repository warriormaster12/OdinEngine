#version 450

#extension GL_GOOGLE_include_directive : enable

#include "PBRFunctions.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 1) uniform MaterialData
{
    vec4 albedo; //vec3
    vec4 metallic; //float
    vec4 roughness; //float
    vec4 ao; //float

    vec4 repeateCount; //int
}materialData;

struct PointLight
{
    vec4 position; //vec3
    vec4 color; //vec3
};

layout(std430, set = 0, binding = 1) buffer LightData
{
    vec4 lightCount; //int
    vec4 camPos; //vec3
    PointLight pLights[];
}lightData;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 V = normalize(vec3(lightData.camPos) - inPosition);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, vec3(materialData.albedo), float(materialData.metallic));

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < int(lightData.lightCount); ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(vec3(lightData.pLights[i].position) - inPosition);
        vec3 H = normalize(V + L);
        float distance = length(vec3(lightData.pLights[i].position) - inPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vec3(lightData.pLights[i].color) * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, float(materialData.roughness));   
        float G   = GeometrySmith(N, V, L, float(materialData.roughness));      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = nominator / max(denominator, 0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - float(materialData.metallic);	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * vec3(materialData.albedo) / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * vec3(materialData.albedo) * float(materialData.ao);

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    outFragColor = vec4(color, 1.0);
}
