#version 450 core

const int LightNumber = 8;

struct Material
{
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

struct Camera
{
    vec3 viewPos;
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

uniform bool UseLights;

vec3 finalNormal;

// ----------------------- Normal --------------------------------------
vec3 CalculateNormal()
{
    if (!material.hasNormalMap)
        return normal;

    vec3 norm = normalize(normal);
    vec3 tang = normalize(tangent);
    // Re-orthogonalize tangent.
    tang = normalize(tang - dot(tang, norm) * norm);
    vec3 Bitangent = cross(tang, norm);
    vec3 BumpMapNormal = texture(material.normalMap, uv).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(tang, Bitangent, norm);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

// ----------------------- Lights --------------------------------------
vec4 CalculateDirectionalLight(DirectionalLight directional)
{
    vec3 lightDir = normalize(-directional.direction);
    vec3 viewDir = normalize(camera.viewPos - pos);

    // Lambertian reflection (diffuse)
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec4 diffuseColor;

    if (material.hasAlbedo) {
        vec4 textureColor = texture(material.albedo, uv);
        diffuseColor = textureColor * vec4(directional.diffuse, 1) * diff;
    } 
    else {
        diffuseColor = material.diffuse * vec4(directional.diffuse, 1) * diff;
    }

    // Specular reflection
    vec3 reflectDir = reflect(-lightDir, finalNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * vec4(directional.specular, 1) * spec;

    // Ambient light
    vec4 ambientColor = material.ambient * vec4(directional.ambient, 1);

    // Final color
    return ambientColor + diffuseColor + specularColor;
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
        vec4 textureColor = texture(material.albedo, uv);
        diffuseColor = textureColor * vec4(point.diffuse, 1) * diff * attenuation;
    } 
    else {
        diffuseColor = material.diffuse * vec4(point.diffuse, 1) * diff * attenuation;
    }

    vec3 reflectDir = reflect(-lightDir, finalNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * vec4(point.specular, 1) * spec * attenuation;

    vec4 ambientColor = material.ambient * vec4(point.ambient, 1) * attenuation;

    return ambientColor + diffuseColor + specularColor;
}

vec4 CalculateSpotLight(SpotLight spot)
{
    vec4 diffuseColor;
    if (material.hasAlbedo) {
        diffuseColor = texture(material.albedo, uv);
    } 
    else {
        diffuseColor = material.diffuse;
    }

    // ambient
    vec4 ambient = vec4(spot.ambient, 1) * diffuseColor;
    
    // diffuse 
    vec3 norm = normalize(finalNormal);
    vec3 lightDir = normalize(spot.position - pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = vec4(spot.diffuse, 1) * diff * diffuseColor;  
    
    // specular
    vec3 viewDir = normalize(camera.viewPos - pos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec4 specular = vec4(spot.specular, 1) * spec * material.specular;  
    
    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-spot.direction)); 
    float epsilon = (spot.cutOff - spot.outerCutOff);
    float intensity = clamp((theta - spot.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // attenuation
    float distance    = length(spot.position - pos);
    float attenuation = 1.0 / (spot.constant + spot.linear * distance + spot.quadratic * (distance * distance));    
    ambient  *= attenuation; 
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec4 result = ambient + diffuse + specular;
    return result;
}

// ----------------------- Main --------------------------------------
void main()
{
    // if UseLights is never used it will not be detected.
    // this is why we do this :
    if (UseLights)
        discard;

    finalNormal = CalculateNormal();

    if (material.hasAlbedo && texture(material.albedo, uv).a == 0.f)
        discard;
    
    vec4 globalLight = vec4(0.f, 0.f, 0.f, 1.f);
    for (int i = 0; i < LightNumber; i++)
    {
        if (directionals[i].enable)
        {
            globalLight += CalculateDirectionalLight(directionals[i]);
        }
    }
    for (int i = 0; i < LightNumber; i++)
    {
        if (points[i].enable)
        {
            globalLight += CalculatePointLight(points[i]);
        }
    }
    for (int i = 0; i < LightNumber; i++)
    {
        if (spots[i].enable)
        {
            globalLight += CalculateSpotLight(spots[i]);
        }
    }

    FragColor = globalLight;

    if (FragColor.a <= 0.0f)
        discard;
}