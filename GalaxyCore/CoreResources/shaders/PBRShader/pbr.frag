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

// TODO : Fix this shader
//------------------------------------------------------------
// Helper functions for PBR calculations
//------------------------------------------------------------

// Obtain per-fragment normal, using a normal map if available.
vec3 getNormal()
{
    vec3 N = normalize(normal);
    if (material.hasNormalMap)
    {
        // Construct TBN matrix: tangent, bitangent, and normal.
        vec3 T = normalize(tangent);
        vec3 B = normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);
        vec3 sampledNormal = texture(material.normalMap, uv).rgb;
        sampledNormal = sampledNormal * 2.0 - 1.0; // Remap from [0,1] to [-1,1]
        return normalize(TBN * sampledNormal);
    }
    return N;
}

// GGX normal distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;

    return num / max(denom, 0.0001);
}

// Schlick-GGX geometry function for one direction
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith's method to combine geometry function for both the view and light directions
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel equation using Schlick's approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//------------------------------------------------------------
// Main function
//------------------------------------------------------------
void main()
{
    // Obtain the normal (with normal mapping if available)
    vec3 N = getNormal();
    // View vector
    vec3 V = normalize(camera.viewPos - pos);

    // Retrieve base color (albedo)
    vec3 albedo = vec3(1.0);
    if (material.hasAlbedo)
        albedo = texture(material.albedo, uv).rgb;
    else
        albedo = material.diffuse.rgb; // fallback to diffuse color

    // Metallic and roughness factors
    float metallic = material.metallic;
    if (material.hasMetallicMap)
        metallic = texture(material.metallicMap, uv).r;
    // Use a default if not provided
    float roughness = material.roughness;
    if (material.hasRoughnessMap)
        roughness = texture(material.roughnessMap, uv).r;

    // Ambient occlusion (assumed full if not provided)
    float ao = 1.0;
    if (material.hasOcclusionMap)
        ao = texture(material.occlusionMap, uv).r;

    // Base reflectivity: for non-metals use 0.04, for metals use the albedo color.
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0); // outgoing radiance

    //------------------------------------------------------------
    // Directional Lights
    //------------------------------------------------------------
    for (int i = 0; i < LightNumber; ++i)
    {
        if (!directionals[i].enable)
            continue;
        // For directional lights, light direction is the inverse of the stored direction.
        vec3 L = normalize(-directionals[i].direction);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // Cook-Torrance BRDF components
            float D = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = D * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
            vec3 specular = numerator / denominator;

            // Diffuse term with energy conservation
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= (1.0 - metallic);

            // Use light.diffuse as the radiance color
            vec3 radiance = directionals[i].diffuse;
            Lo += (kD * albedo / 3.14159265 + specular) * radiance * NdotL;
        }
    }

    //------------------------------------------------------------
    // Point Lights
    //------------------------------------------------------------
    for (int i = 0; i < LightNumber; ++i)
    {
        if (!points[i].enable)
            continue;
        vec3 L = normalize(points[i].position - pos);
        vec3 H = normalize(V + L);
        float distance = length(points[i].position - pos);
        float attenuation = 1.0 / (points[i].constant + points[i].linear * distance + points[i].quadratic * (distance * distance));
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            float D = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = D * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= (1.0 - metallic);

            vec3 radiance = points[i].diffuse * attenuation;
            Lo += (kD * albedo / 3.14159265 + specular) * radiance * NdotL;
        }
    }

    //------------------------------------------------------------
    // Spot Lights
    //------------------------------------------------------------
    for (int i = 0; i < LightNumber; ++i)
    {
        if (!spots[i].enable)
            continue;
        vec3 L = normalize(spots[i].position - pos);
        vec3 H = normalize(V + L);
        float distance = length(spots[i].position - pos);
        float attenuation = 1.0 / (spots[i].constant + spots[i].linear * distance + spots[i].quadratic * (distance * distance));

        // Calculate spotlight intensity based on inner and outer cut-off
        float theta = dot(L, normalize(-spots[i].direction));
        float epsilon = spots[i].cutOff - spots[i].outerCutOff;
        float intensity = clamp((theta - spots[i].outerCutOff) / epsilon, 0.0, 1.0);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            float D = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = D * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= (1.0 - metallic);

            vec3 radiance = spots[i].diffuse * attenuation * intensity;
            Lo += (kD * albedo / 3.14159265 + specular) * radiance * NdotL;
        }
    }

    //------------------------------------------------------------
    // Ambient / Image-Based Lighting (IBL)
    //------------------------------------------------------------
    // A simple ambient term modulated by AO (here assumed 1.0 if not provided)
    vec3 ambient = vec3(0.03) * albedo * ao;
    // Environment reflection from the skybox (basic approximation)
    vec3 R = reflect(-V, N);
    vec3 envColor = texture(camera.skybox, R).rgb;
    // Blend the ambient color with the environment reflection based on metallic factor
    vec3 ambientIBL = mix(ambient, envColor, metallic);

    // Combine direct lighting (Lo) with ambient lighting
    vec3 color = ambientIBL + Lo;

    //------------------------------------------------------------
    // HDR Tonemapping and Gamma Correction
    //------------------------------------------------------------
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
