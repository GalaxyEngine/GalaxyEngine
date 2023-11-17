#version 450 core
out vec4 FragColor;
  
in vec2 uv;

uniform sampler2D Texture;

void main()
{ 
    vec4 centerColor = texture(Texture, uv);

    float outlineThickness = 5; 

    // Check the neighboring pixels to see if they are black
    vec2 offset = outlineThickness / textureSize(Texture, 0);
    vec4 leftColor = texture(Texture, uv - vec2(offset.x, 0));
    vec4 rightColor = texture(Texture, uv + vec2(offset.x, 0));
    vec4 topColor = texture(Texture, uv + vec2(0, offset.y));
    vec4 bottomColor = texture(Texture, uv - vec2(0, offset.y));

    // If any of the neighboring pixels are black, draw an outline
    if (centerColor == vec4(1.0) && (leftColor == vec4(0.0) || rightColor == vec4(0.0) || topColor == vec4(0.0) || bottomColor == vec4(0.0))) {
        FragColor = vec4(1.0);  // White outline
    } else {
        FragColor = vec4(0.0);  // Black background
    }
}
