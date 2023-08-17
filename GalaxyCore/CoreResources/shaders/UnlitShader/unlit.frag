#version 330 core
out vec4 FragColor;

in vec2 uv;
in vec3 normal;

uniform vec4 Diffuse;
uniform bool EnableTexture;
uniform sampler2D Texture;

void main()
{
    if (EnableTexture)
        FragColor = texture(Texture, uv);
    else
        FragColor = vec4(Diffuse.x, Diffuse.y, Diffuse.z, Diffuse.w);
} 