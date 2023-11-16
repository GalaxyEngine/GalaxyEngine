#version 330 core

struct Material
{
    vec4 diffuse;
    bool enableTexture;
    sampler2D albedo;
};

out vec4 FragColor;

in vec2 uv;
in vec3 normal;

uniform Material material;

void main()
{
    if (material.enableTexture)
        FragColor = texture(material.albedo, uv);
    else
        FragColor = material.diffuse;
} 