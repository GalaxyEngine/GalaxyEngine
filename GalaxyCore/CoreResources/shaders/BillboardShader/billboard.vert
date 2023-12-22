#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNor;

out vec2 uv;
out vec3 normal;
out vec3 viewDir;

uniform mat4 MVP;
uniform vec3 CamUp;
uniform vec3 CamRight;
uniform float BillboardSize = 0.5f;

void main()
{
	vec3 wPos = CamUp * aPos.y * BillboardSize + CamRight * aPos.x * BillboardSize;
    gl_Position = MVP * vec4(wPos, 1.0f);

	normal = aNor;
	uv = aTex;
	viewDir = vec3(0.f);
}