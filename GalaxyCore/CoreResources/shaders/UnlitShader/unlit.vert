#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNor;
layout (location = 3) in vec3 aTan;

out vec2 uv;
out vec3 viewDir; // Changed from viewPos to viewDir

uniform mat4 MVP;
uniform mat4 Model;
uniform vec3 ViewPos;

void main()
{

  vec3 worldPosition = vec3(Model * vec4(aPos, 1.0));
  vec3 T = normalize(mat3(Model) * aTan);
  vec3 N = normalize(mat3(Model) * aNor);
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);

  mat3 TBN = transpose(mat3(T, B, N));

  viewDir = normalize(TBN * (ViewPos - worldPosition));
  uv = aTex;
  gl_Position = MVP * vec4(aPos, 1.0);
}