#version 450 core

// Maximum number of each light type and constant PI
const int LightNumber = 8;
const float PI = 3.1415926535897932384626433832795;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    sampler2D albedo;
    bool hasAlbedo;
    sampler2D normalMap;
    bool hasNormalMap;
    sampler2D metallicMap;
    bool hasMetallicMap;
    float metallic;
    sampler2D roughnessMap;
    bool hasRoughnessMap;
    float roughness;
    sampler2D occlusionMap;
    bool hasOcclusionMap;
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
    samplerCube skybox;
};

out vec4 FragColor;

in vec3 pos;
in vec2 uv;
in vec3 normal;
in vec3 tangent;

uniform Material material;
uniform DirectionalLight directionals[LightNumber];
uniform PointLight points[LightNumber];
uniform SpotLight spots[LightNumber];
uniform Camera camera;

// Function declarations
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 calculateDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 calculatePointLight(PointLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 calculateSpotLight(SpotLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness, vec3 F0);

void main() {
    // Material properties
    vec4 albedoColor = material.hasAlbedo ? texture(material.albedo, uv) : material.diffuse;
    float metallic = material.hasMetallicMap ? texture(material.metallicMap, uv).r : material.metallic;
    float roughness = material.hasRoughnessMap ? texture(material.roughnessMap, uv).r : material.roughness;
    float ao = material.hasOcclusionMap ? texture(material.occlusionMap, uv).r : 1.0;

    // Normal mapping
    vec3 N = normalize(normal);
    if (material.hasNormalMap) {
        vec3 tangentNormal = texture(material.normalMap, uv).xyz * 2.0 - 1.0;
        vec3 T = normalize(tangent);
        vec3 B = cross(N, T);
        mat3 TBN = mat3(T, B, N);
        N = normalize(TBN * tangentNormal);
    }

    // View direction
    vec3 V = normalize(camera.viewPos - pos);
    vec3 R = reflect(-V, N);

    // Base reflectance
    vec3 F0 = mix(vec3(0.04), albedoColor.rgb, metallic);

    // Direct lighting
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < LightNumber; i++) {
        if (directionals[i].enable)
            Lo += calculateDirectionalLight(directionals[i], N, V, albedoColor.rgb, metallic, roughness, F0);
        if (points[i].enable)
            Lo += calculatePointLight(points[i], N, V, pos, albedoColor.rgb, metallic, roughness, F0);
        if (spots[i].enable)
            Lo += calculateSpotLight(spots[i], N, V, pos, albedoColor.rgb, metallic, roughness, F0);
    }

    // Ambient lighting (simple skybox reflection)
    vec3 ambient = texture(camera.skybox, R).rgb * albedoColor.rgb * ao * 0.1;

    // Combine
    vec3 color = ambient + Lo;

    // HDR and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}

// PBR Functions
vec3 fresnelSchlick(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Light calculation functions
vec3 calculateDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0) {
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(V + L);
    
    // BRDF calculations
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * light.diffuse * NdotL;
}

vec3 calculatePointLight(PointLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness, vec3 F0) {
    // Calculate light vector and distance
    vec3 L = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Half vector and BRDF calculations
    vec3 H = normalize(V + L);
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = light.diffuse * attenuation;
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calculateSpotLight(SpotLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness, vec3 F0) {
    // Calculate light vector and distance
    vec3 L = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Calculate spotlight intensity based on inner and outer cone angles
    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    attenuation *= intensity;
    
    // Optionally, you can early exit if outside the outer cone
    if (theta <= light.outerCutOff)
        return vec3(0.0);
    
    // BRDF calculations (same as for directional and point lights)
    vec3 H = normalize(V + L);
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = light.diffuse * attenuation;
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}
