#version 450 core

const int LightNumber = 8;

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    bool enableTexture;
    sampler2D albedo;
};

struct DirectionalLight {
    bool enable;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 direction;
};

struct PointLight {
    bool enable;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec3 position;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    bool enable;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

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

uniform Material material;
uniform DirectionalLight directionals[LightNumber];
uniform PointLight points[LightNumber];
uniform SpotLight spots[LightNumber];
uniform Camera camera;

uniform bool UseLights;

// Function to calculate directional light
vec4 CalculateDirectionalLight(DirectionalLight directional)
{
    vec3 lightDir = normalize(-directional.direction);
    vec3 viewDir = normalize(camera.viewPos - pos);

    // Lambertian reflection (diffuse)
    float diff = max(dot(normal, lightDir), 0.0);
    vec4 diffuseColor;

    if (material.enableTexture) {
        vec4 textureColor = texture(material.albedo, uv);
        diffuseColor = textureColor * directional.diffuse * diff;
    } 
    else {
        diffuseColor = material.diffuse * directional.diffuse * diff;
    }

    // Specular reflection
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * directional.specular * spec;

    // Ambient light
    vec4 ambientColor = material.ambient * directional.ambient;

    // Final color
    return ambientColor + diffuseColor + specularColor;
}

vec4 CalculatePointLight(PointLight point)
{
    vec3 lightDir = normalize(point.position - pos);
    vec3 viewDir = normalize(camera.viewPos - pos);

    float distance = length(point.position - pos);
    float attenuation = 1.0 / (point.constant + point.linear * distance + point.quadratic * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec4 diffuseColor;

    if (material.enableTexture) {
        vec4 textureColor = texture(material.albedo, uv);
        diffuseColor = textureColor * point.diffuse * diff * attenuation;
    } 
    else {
        diffuseColor = material.diffuse * point.diffuse * diff * attenuation;
    }

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * point.specular * spec * attenuation;

    vec4 ambientColor = material.ambient * point.ambient * attenuation;

    return ambientColor + diffuseColor + specularColor;
}

vec4 CalculateSpotLight(SpotLight spot)
{
    vec3 lightDir = normalize(spot.position - pos);
    vec3 viewDir = normalize(camera.viewPos - pos);

    float distance = length(spot.position - pos);
    float attenuation = 1.0 / (spot.constant + spot.linear * distance + spot.quadratic * (distance * distance));

    float spotEffect = dot(normalize(-lightDir), normalize(spot.direction));
    float spotIntensity = smoothstep(spot.outerCutOff, spot.cutOff, spotEffect);

    float diff = max(dot(normal, lightDir), 0.0);
    vec4 diffuseColor;

    if (material.enableTexture) {
        vec4 textureColor = texture(material.albedo, uv);
        diffuseColor = textureColor * spot.diffuse * diff * attenuation * spotIntensity;
    } 
    else {
        diffuseColor = material.diffuse * spot.diffuse * diff * attenuation * spotIntensity;
    }

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularColor = material.specular * spot.specular * spec * attenuation * spotIntensity;

    vec4 ambientColor = material.ambient * spot.ambient * attenuation * spotIntensity;

    return ambientColor + diffuseColor + specularColor;
}

void main()
{
    // if UseLights is never used it will not be detected.
    // this is why we do this :
    if (UseLights)
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
}