#version 330 core

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

void main()
{
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

    FragColor = globalLight;
}