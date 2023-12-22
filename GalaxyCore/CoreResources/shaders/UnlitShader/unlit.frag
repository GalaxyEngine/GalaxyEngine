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
in vec3 normal;
in vec3 viewDir;

uniform Material material;

vec2 ParallaxOffset(vec2 texCoords, vec3 view, float heightScale) {
    // Fetch the height at the current texture coordinate
    float height = texture(material.parallaxMap, texCoords).r;
    // Scale the view direction by the height and the height scaling factor
    vec2 p = view.xy / view.z * (height * heightScale);
    // Offset the texture coordinates by the calculated parallax offset
    return texCoords - p;
}

void main()
{
    vec2 texCoords = uv;
    if (material.hasParallaxMap) {
        // Apply parallax mapping to the texture coordinates
        texCoords = ParallaxOffset(texCoords, -viewDir, material.heightScale);
    }

    if (material.hasAlbedo) {
        FragColor = texture(material.albedo, texCoords);
    } else {
        FragColor = material.diffuse;
    }
}