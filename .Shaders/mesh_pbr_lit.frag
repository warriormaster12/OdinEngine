//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 WorldPos;
layout (location = 3) in vec3 Normal;
//output write
layout (location = 0) out vec4 outFragColor;


layout(set = 0, binding = 0) uniform  CameraBuffer{   
    mat4 view;
    mat4 proj;
	mat4 viewproj;
	vec4 camPos; // vec3
} cameraData;

struct Light
{
	vec4 lightPositions[2]; // vec3
	vec4 lightColors[2]; // vec3
};

layout(set = 0, binding = 1) uniform  SceneData{   
	Light lightData;
	
} sceneData;

layout(set = 2, binding = 0) uniform MaterialData{
	vec4 albedo; // vec4
	vec4 metallic; // float
	vec4 roughness; // float
	vec4 ao; // float

    vec4 emissionColor; //vec3
    vec4 emissionPower; // float
} materialData;

layout(set = 2, binding = 1) uniform sampler2D albedoMap;
layout(set = 2, binding = 2) uniform sampler2D aoMap;
layout(set = 2, binding = 3) uniform sampler2D normalMap;
layout(set = 2, binding = 4) uniform sampler2D emissionMap;
layout(set = 2, binding = 5) uniform sampler2D metalRoughnessMap;


const float PI = 3.14159265359;
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, texCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
	vec4 albedo =  pow(texture(albedoMap, texCoord).rgba, vec4(2.2));
    vec3 emission = texture(emissionMap, texCoord).rgb * vec3(materialData.emissionColor) * float(materialData.emissionPower);
    float ao = texture(aoMap, texCoord).r;
    float metallic = texture(metalRoughnessMap, texCoord).b;
    float roughness = texture(metalRoughnessMap, texCoord).g;

    // this is for objects that have a texture loaded
    if (albedo.r != 0.0f || albedo.g != 0.0f || albedo.b != 0.0f)
    {
        albedo *= materialData.albedo;
        if (albedo.a < 0.1)
        {
            discard;
        }

    }

    // this is for objects that have an empty texture
    else if (albedo.rgba == vec4(0.0f))
    {
        albedo += materialData.albedo;
    }

    if(ao != 0.0f)
    {
        ao *= float(materialData.ao);
    }
    else
    {
        ao += float(materialData.ao);
    }
    vec3 N;
    if(texture(normalMap, texCoord).xyz != vec3(0.0f))
    {
        N = getNormalFromMap();
    }
    else
    {
        N = normalize(Normal);
    }
    if(metallic != 0.0f)
    {
        metallic *= float(materialData.metallic);
    }
    else
    {
        metallic += float(materialData.metallic);
    }
    if(roughness != 0.0f)
    {
        roughness *= float(materialData.roughness);
    }
    else
    {
        roughness += float(materialData.roughness);  
    }
    
    vec3 V = normalize(vec3(cameraData.camPos) - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, vec3(albedo), metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 2; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(vec3(sceneData.lightData.lightPositions[i]) - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(vec3(sceneData.lightData.lightPositions[i]) - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vec3(sceneData.lightData.lightColors[i]) * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
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
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * vec3(albedo) / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * vec3(albedo) * ao;

    vec3 color = ambient + Lo + emission;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    outFragColor = vec4(color, 1.0f);
}