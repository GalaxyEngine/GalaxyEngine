#version 330 core
out vec4 FragColor;
  
in vec2 uv;

uniform sampler2D Texture;

void main()
{ 
    FragColor = vec4(vec3(1.0 - texture(Texture, uv)), 1.0);
}

