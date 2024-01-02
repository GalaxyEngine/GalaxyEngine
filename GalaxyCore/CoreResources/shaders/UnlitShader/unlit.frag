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

vec2 ParallaxOffset(vec2 texCoords, vec3 viewDir, float heightScale) {

    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(material.parallaxMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(material.parallaxMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    return currentTexCoords;
}

void main()
{
  vec3 viewDirection = normalize(viewDir);
  vec2 texCoords = uv;
  if (material.hasParallaxMap) 
  {
    texCoords = ParallaxOffset(texCoords, viewDirection, material.heightScale);
  }
  
  if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

  if (material.hasAlbedo) 
  {
    FragColor = texture(material.albedo, texCoords);
  } 
  else 
  {
    FragColor = material.diffuse;
  }
}