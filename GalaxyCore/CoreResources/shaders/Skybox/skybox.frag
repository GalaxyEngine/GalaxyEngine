#version 450 core

out vec4 FragColor;

in vec3 uv;

struct Material
{
    samplerCube skybox;
};

uniform Material material;

void main()
{
    FragColor = texture(material.skybox, uv);
}