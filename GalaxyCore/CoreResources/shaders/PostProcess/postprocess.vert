#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec3 tangent;
out vec4 color;
out vec4 fragPosLight;

void main()
{
    gl_Position = vec4(aPos.xy, 0.0, 1.0); 
    texCoord = aTexCoords;
}  