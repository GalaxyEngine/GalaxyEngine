#version 330 core

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
uniform DirectionalLight directional;
uniform Camera camera;

uniform bool UseLights;

// Function to calculate directional light
vec4 CalculateDirectionalLight(vec3 lightDir, vec3 viewDir)
{
    // Lambertian reflection (diffuse)
    float diff = max(dot(normal, lightDir), 0.0);
    vec4 diffuseColor;

    if (material.enableTexture) {
        vec4 textureColor = texture(material.albedo, uv);
        diffuseColor = textureColor * directional.diffuse * diff;
    } else {
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
    FragColor = vec4(0.f, 0.f, 0.f, 1.0f);
    if (UseLights)
        discard;
    if (directional.enable){
        // Calculate the light direction
        vec3 lightDir = normalize(-directional.direction);

        // Calculate the view direction (camera to fragment)
        vec3 viewDir = normalize(camera.viewPos - pos);

        // Calculate final color using the directional light method
        FragColor = CalculateDirectionalLight(lightDir, viewDir);
    }
}