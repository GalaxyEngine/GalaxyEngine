#version 450 core

layout(location = 0) out vec4 FragColor;

struct Material {
    vec4 diffuse;
    bool hovered;
};

uniform Material material;

void main()
{
    vec4 col = material.diffuse;
    if (material.hovered)
        col = vec4(col.rgb * 0.3 + vec3(1.0, 1.0, 0.0) * 0.7, 1.0);
    FragColor = col;
}
