
#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec3 tangent;
in vec4 color;
in vec4 fragPosLight;

uniform sampler2D tex;

void main()
{ 
    FragColor = vec4(vec3(1.0 - texture(tex, texCoord)), 1.0);
}

