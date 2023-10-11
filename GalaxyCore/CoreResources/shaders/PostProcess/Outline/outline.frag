#version 330 core
out vec4 FragColor;
  
in vec2 uv;

uniform sampler2D Texture;

void main()
{ 
    // outline thickness
    int w = 3;

    // if the pixel is black (we are on the silhouette)
    if (texture(Texture, uv).xyz == vec3(0.0f))
    {
        vec2 size = 1.0f / textureSize(Texture, 0);

        for (int i = -w; i <= +w; i++)
        {
            for (int j = -w; j <= +w; j++)
            {
                if (i == 0 && j == 0)
                {
                    continue;
                }

                vec2 offset = vec2(i, j) * size;

                // and if one of the pixel-neighbor is white (we are on the border)
                if (texture(Texture, uv + offset).xyz == vec3(1.0f))
                {
                    FragColor = vec4(vec3(1.0f), 1.0f);
                    return;
                }
            }
        }
    }

    discard;
}