#version 450 core

struct Material
{
    vec4 diffuse;
    bool hasAlbedo;
    sampler2D albedo;
};

out vec4 FragColor;

in vec2 uv;
in vec3 normal;

uniform Material material;

void main()
{
    if (material.hasAlbedo)
        FragColor = texture(material.albedo, uv);
    else
        FragColor = material.diffuse;

    if (FragColor.a <= 0.1f)
        discard;
} 