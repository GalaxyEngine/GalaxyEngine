#version 450 core

out vec4 FragColor;

in vec2 uv;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, vec3(uv, 0));
}