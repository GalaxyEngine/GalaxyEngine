#version 330 core
out vec4 FragColor;

uniform vec4 Diffuse;

void main()
{
    FragColor = Diffuse;
}