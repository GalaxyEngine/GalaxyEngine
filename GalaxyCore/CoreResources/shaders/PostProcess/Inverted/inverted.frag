#version 450 core
out vec4 FragColor;
  
in vec2 uv;

struct Material
{
    sampler2D albedo;
};
uniform Material material;

void main()
{ 
    FragColor = vec4(vec3(1.0 - texture(material.albedo, uv)), 1.0);
}

