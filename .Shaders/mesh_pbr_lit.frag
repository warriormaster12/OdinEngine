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

struct DirectionLight{
    vec4 direction; //vec3
    vec4 color; //vec3
    vec4 intensity; //float
};
struct PointLight
{
	vec4 position; // vec3
	vec4 color; // vec3
    vec4 radius; //float
    vec4 intensity; //float
};

layout(std430, set = 0, binding = 1)  readonly buffer SceneData{ 
    vec4 plightCount; //int
    DirectionLight dLight;
	PointLight pointLights[];
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
layout(set = 2, binding = 6) uniform sampler2D metallicMap;
layout(set = 2, binding = 7) uniform sampler2D roughnessMap;



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
    float k = r*r / 8.0;

    float num = NdotV;
    float denom = 1 / (NdotV* (1.0 - k) + k);

    return num * denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float nDotV, float nDotL, float rough)
{
    float ggx2  = GeometrySchlickGGX(nDotV, rough);
    float ggx1  = GeometrySchlickGGX(nDotL, rough);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 calcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float rough, float metal, vec3 F0,  float viewDistance);

// ----------------------------------------------------------------------------
vec3 calcDirLight(DirectionLight light, vec3 normal, vec3 viewDir, vec3 albedo, float rough, float metal, vec3 F0);
// ----------------------------------------------------------------------------
void main()
{
	vec4 albedo =  pow(texture(albedoMap, texCoord).rgba, vec4(2.2));
    vec4 emission = texture(emissionMap, texCoord).rgba * materialData.emissionColor * float(materialData.emissionPower);
    float ao = texture(aoMap, texCoord).r;
    float metallic;
    float roughness;
    if(texture(metalRoughnessMap, texCoord).rgba != vec4(0.0f))
    {
        metallic = texture(metalRoughnessMap, texCoord).b;
        roughness = texture(metalRoughnessMap, texCoord).g;
    }
    else
    {
        metallic = texture(metallicMap, texCoord).r;
        roughness = texture(roughnessMap, texCoord).r;
    }

    // this is for objects that have a texture loaded
    if (albedo.a > 0.1)
    {
        albedo *= materialData.albedo;
    }
    // this is for objects that have an empty texture
    else if (albedo.rgba == vec4(0.0f))
    {
        albedo += materialData.albedo;
    }

    if(ao > 0.1f)
    {
        ao *= float(materialData.ao);
    }
    else
    {
        ao += float(materialData.ao);
    }
    vec3 N = texture(normalMap, texCoord).xyz;
    if(N.x > 0.1f || N.y > 0.1f || N.z > 0.1f)
    {
        N = getNormalFromMap();
    }
    else
    {
        N = normalize(Normal);
    }
    if(metallic > 0.1)
    {
        metallic *= float(materialData.metallic);
    }
    else
    {
        metallic += float(materialData.metallic);
    }
    if(roughness > 0.1)
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
    vec3 radianceOut = calcDirLight(sceneData.dLight, N, V, albedo.rgb, roughness, metallic, F0);
    float viewDistance = length(vec3(cameraData.camPos) - WorldPos);
    for(int i = 0; i < int(sceneData.plightCount); ++i) 
    {
        // calculate per-light radiance
        radianceOut += calcPointLight(i, N, WorldPos, V, albedo.rgb, roughness, metallic, F0, viewDistance);
    }
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.025)* albedo.rgb;
    ambient *= ao;
    radianceOut += ambient;
    radianceOut += emission.rgb;
    vec3 color = radianceOut;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2)); 

    outFragColor = vec4(color, albedo.a);
}

vec3 calcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float rough, float metal, vec3 F0,  float viewDistance)
{
    //Point light basics
    vec3 position = sceneData.pointLights[index].position.xyz;
    vec3 color    = sceneData.pointLights[index].color.rgb * float(sceneData.pointLights[index].intensity);
    float radius  = float(sceneData.pointLights[index].radius);

    //Stuff common to the BRDF subfunctions 
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    //Attenuation calculation that is applied to all
    float distance    = length(position - fragPos);
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance) );
    vec3 radianceIn   = color * attenuation;

    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfway, rough);
    float G   = GeometrySmith(nDotV, nDotL, rough);
    vec3  F   = fresnelSchlick(max(dot(halfway,viewDir), 0.0), F0);

    //Finding specular and diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max(denominator, 0.0000001);
    // vec3 specular = numerator / denominator;

    vec3 radiance = (kD * (albedo / PI) + specular ) * radianceIn * nDotL;

    //we do not currently support shadows
    // //shadow stuff
    // vec3 fragToLight = fragPos - position;
    // float shadow = calcPointLightShadows(depthMaps[index], fragToLight, viewDistance);
    
    // radiance *= (1.0 - shadow);

    return radiance;
}

vec3 calcDirLight(DirectionLight light, vec3 normal, vec3 viewDir, vec3 albedo, float rough, float metal, vec3 F0)
{
    //Variables common to BRDFs
    vec3 lightDir = normalize(vec3(-light.direction));
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);
    vec3 radianceIn = light.color.rgb * float(light.intensity);

    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfway, rough);
    float G   = GeometrySmith(nDotV, nDotL, rough);
    vec3  F   = fresnelSchlick(max(dot(halfway,viewDir), 0.0), F0);

    //Finding specular and diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max (denominator, 0.0001);

    vec3 radiance = (kD * (albedo / PI) + specular ) * radianceIn * nDotL;

    //shadow isn't supported yet
    // radiance *= (1.0 - shadow);

    return radiance;
}
