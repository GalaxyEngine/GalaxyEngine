#version 450 core

struct Material
{
  vec4 diffuse;
  bool hasAlbedo;
  sampler2D albedo;
  bool hasParallaxMap;
  sampler2D parallaxMap;
  float heightScale;
};

out vec4 FragColor;

in vec2 uv;
in vec3 viewDir;

uniform Material material;

vec2 ParallaxOffset(vec2 texCoords) {
    float height = texture(material.parallaxMap, texCoords).r;

    vec2 pOffset = (viewDir.xy / viewDir.z) * (height * material.heightScale);

    return texCoords - pOffset;
}

void main()
{
  vec3 viewDirection = normalize(viewDir);
  vec2 texCoords = uv;
  if (material.hasParallaxMap) 
  {
    texCoords = ParallaxOffset(texCoords);
  
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
  }

  if (material.hasAlbedo) 
  {
    FragColor = texture(material.albedo, texCoords);
  } 
  else 
  {
    FragColor = material.diffuse;
  }
}