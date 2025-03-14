#version 450 core

// Maximum number of each light type
const int LightNumber = 8;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    sampler2D albedo;
    bool hasAlbedo;
    sampler2D normalMap;
    bool hasNormalMap;
    sampler2D parallaxMap;
    bool hasParallaxMap;
    float heightScale;
};

struct DirectionalLight {
    bool enable;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};

struct PointLight {
    bool enable;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 position;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    bool enable;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 position;
    float constant;
    float linear;
    float quadratic;

    vec3 direction;
    float cutOff;
    float outerCutOff;
};

struct Camera {
    vec3 viewPos;
    sampler2D depthMap;
};

out vec4 FragColor;

in vec3 pos;
in vec2 uv;
in vec3 normal;
in vec3 tangent;
in vec4 posLightSpace;

// Uniforms
uniform Material material;
uniform DirectionalLight directionals[LightNumber];
uniform PointLight points[LightNumber];
uniform SpotLight spots[LightNumber];
uniform Camera camera;

// Dummy uniform to ensure it's not optimized away.
uniform bool UseLights;

// Global variables to be computed in main.
vec3 finalNormal;
vec2 modUV; // modified texture coordinate (after parallax mapping)

//
// ParallaxMapping: offsets the texture coordinates based on the view direction (in tangent space)
//
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTangent)
{
    // Sample the height from the parallax map (using the red channel)
    float height = texture(material.parallaxMap, texCoords).r;
    // Compute a simple offset (basic approximation)
    vec2 offset = viewDirTangent.xy * (height * material.heightScale);
    return texCoords - offset;
}

//
// CalculateNormal: uses the normal map (if available) to return a perturbed normal.
// It receives the precomputed TBN matrix so that it works with the parallax-adjusted UV coordinates.
//
vec3 CalculateNormal(mat3 TBN)
{
    if (!material.hasNormalMap)
        return normalize(normal);
    
    // Sample the normal from the normal map using modUV.
    vec3 bumpNormal = texture(material.normalMap, modUV).xyz;
    // Remap from [0,1] to [-1,1]
    bumpNormal = 2.0 * bumpNormal - vec3(1.0);
    return normalize(TBN * bumpNormal);
}

//
// ShadowCalculation: computes a shadow factor using the light's depth map
//
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // If the fragment is outside the light's frustum, return no shadow.
    if(projCoords.z > 1.0)
        return 0.0;
    
    // Sample the closest depth from the shadow map.
    float closestDepth = texture(camera.depthMap, projCoords.xy).r;
    // Current depth in light space.
    float currentDepth = projCoords.z;
    // Compute a bias to reduce shadow acne.
    float bias = max(0.05 * (1.0 - dot(finalNormal, normalize(-directionals[0].direction))), 0.005);
    // If the current fragment is in shadow, return 1.0; otherwise, 0.0.
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

//
// CalculateDirectionalLight: computes directional light contribution.
// If applyShadow is true (e.g. for the main directional light), shadow mapping is applied.
//
vec4 CalculateDirectionalLight(DirectionalLight directional, bool applyShadow)
{
    vec3 lightDir = normalize(-directional.direction);
    vec3 viewDir = normalize(camera.viewPos - pos);

    // Diffuse component (Lambertian)
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec4 diffuseColor;
    if (material.hasAlbedo) {
        vec4 textureColor = texture(material.albedo, modUV);
        diffuseColor = textureColor * vec4(directional.diffuse, 1.0) * diff;
    } else {
        diffuseColor = material.diffuse * vec4(directional.diffuse, 1.0) * diff;
    }

    // Specular component
    vec3 reflectDir = reflect(-lightDir, finalNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * vec4(directional.specular, 1.0) * spec;

    // Ambient component
    vec4 ambientColor = material.ambient * vec4(directional.ambient, 1.0);
    if (material.hasAlbedo) {
        vec4 textureColor = texture(material.albedo, modUV);
        ambientColor = textureColor * ambientColor;
    }

    // Apply shadow factor only to diffuse and specular
    if (applyShadow) {
        float shadow = ShadowCalculation(posLightSpace);
        return ambientColor + (1.0 - shadow) * (diffuseColor + specularColor);
    } else {
        return ambientColor + diffuseColor + specularColor;
    }
}

vec4 CalculatePointLight(PointLight point)
{
    vec3 lightDir = normalize(point.position - pos);
    vec3 viewDir = normalize(camera.viewPos - pos);

    float distance = length(point.position - pos);
    float attenuation = 1.0 / (point.constant + point.linear * distance + point.quadratic * (distance * distance));

    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec4 diffuseColor;
    if (material.hasAlbedo) {
        vec4 textureColor = texture(material.albedo, modUV);
        diffuseColor = textureColor * vec4(point.diffuse, 1.0) * diff * attenuation;
    } else {
        diffuseColor = material.diffuse * vec4(point.diffuse, 1.0) * diff * attenuation;
    }

    vec3 reflectDir = reflect(-lightDir, finalNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * vec4(point.specular, 1.0) * spec * attenuation;

    vec4 ambientColor = material.ambient * vec4(point.ambient, 1.0) * attenuation;

    return ambientColor + diffuseColor + specularColor;
}

vec4 CalculateSpotLight(SpotLight spot)
{
    vec4 albedoColor;
    if (material.hasAlbedo) {
        albedoColor = texture(material.albedo, modUV);
    } else {
        albedoColor = material.diffuse;
    }
    
    // Ambient component
    vec4 ambient = vec4(spot.ambient, 1.0) * albedoColor;
    
    // Diffuse component
    float diff = max(dot(finalNormal, normalize(spot.position - pos)), 0.0);
    vec4 diffuse = vec4(spot.diffuse, 1.0) * diff * albedoColor;  
    
    // Specular component
    vec3 viewDir = normalize(camera.viewPos - pos);
    vec3 reflectDir = reflect(-normalize(spot.position - pos), finalNormal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specular = vec4(spot.specular, 1.0) * spec * material.specular;  
    
    // Spotlight (soft edges)
    vec3 lightDir = normalize(spot.position - pos);
    float theta = dot(lightDir, normalize(-spot.direction)); 
    float epsilon = spot.cutOff - spot.outerCutOff;
    float intensity = clamp((theta - spot.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // Attenuation
    float distance = length(spot.position - pos);
    float attenuation = 1.0 / (spot.constant + spot.linear * distance + spot.quadratic * (distance * distance));    
    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation;   
        
    return ambient + diffuse + specular;
}

//
// Main
//
void main()
{
    // Use dummy uniform to avoid optimization removal.
    if (UseLights) { }  

    // Compute the TBN matrix from the interpolated normal and tangent.
    vec3 N = normalize(normal);
    vec3 T = normalize(tangent);
    // Re-orthogonalize tangent:
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    // Transform view direction into tangent space.
    vec3 viewDir = normalize(camera.viewPos - pos);
    vec3 viewDirTangent = TBN * viewDir;

    // Adjust texture coordinates using parallax mapping if available.
    if (material.hasParallaxMap)
        modUV = ParallaxMapping(uv, viewDirTangent);
    else
        modUV = uv;

    // Compute per-fragment normal (using normal map if available)
    finalNormal = CalculateNormal(TBN);

    // Alpha test: if albedo alpha is 0, discard.
    if (material.hasAlbedo && texture(material.albedo, modUV).a == 0.0)
        discard;
    
    // Check if any light is enabled. If not, output an unlit color.
    bool anyLight = false;
    for (int i = 0; i < LightNumber; i++) {
        if (directionals[i].enable || points[i].enable || spots[i].enable) {
            anyLight = true;
            break;
        }
    }
    if (!anyLight) {
        vec4 unlitColor;
        if (material.hasAlbedo)
            unlitColor = texture(material.albedo, modUV);
        else
            unlitColor = material.diffuse;
        FragColor = unlitColor;
        return;
    }
    
    // Accumulate contributions from each light type.
    vec4 globalLight = vec4(0.0);
    // For directional lights, apply shadow mapping to the first one.
    for (int i = 0; i < LightNumber; i++) {
        if (directionals[i].enable) {
            if (i == 0)
                globalLight += CalculateDirectionalLight(directionals[i], true);
            else
                globalLight += CalculateDirectionalLight(directionals[i], false);
        }
    }
    for (int i = 0; i < LightNumber; i++) {
        if (points[i].enable)
            globalLight += CalculatePointLight(points[i]);
    }
    for (int i = 0; i < LightNumber; i++) {
        if (spots[i].enable)
            globalLight += CalculateSpotLight(spots[i]);
    }

    FragColor = globalLight;

    // Final alpha test.
    if (FragColor.a <= 0.0)
        discard;
}
