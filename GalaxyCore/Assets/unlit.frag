#version 330 core
out vec4 FragColor;

uniform vec4 Color;

void main()
{
    FragColor = vec4(Color.x, Color.y, Color.z, Color.w);
} 