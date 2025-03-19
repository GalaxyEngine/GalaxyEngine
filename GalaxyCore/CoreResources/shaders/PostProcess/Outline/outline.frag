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
    vec4 centerColor = texture(material.albedo, uv);

    float outlineThickness = 5; 

    // Check the neighboring pixels to see if they are black
    vec2 offset = outlineThickness / textureSize(material.albedo, 0);
    vec4 leftColor = texture(material.albedo, uv - vec2(offset.x, 0));
    vec4 rightColor = texture(material.albedo, uv + vec2(offset.x, 0));
    vec4 topColor = texture(material.albedo, uv + vec2(0, offset.y));
    vec4 bottomColor = texture(material.albedo, uv - vec2(0, offset.y));

    // If any of the neighboring pixels are black, draw an outline
    if (centerColor == vec4(1.0) && (leftColor == vec4(0.0) || rightColor == vec4(0.0) || topColor == vec4(0.0) || bottomColor == vec4(0.0))) {
        FragColor = vec4(1.0);  // White outline
    } else {
        FragColor = vec4(0.0);  // Black background
    }
}
