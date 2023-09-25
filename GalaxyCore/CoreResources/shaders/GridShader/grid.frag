#version 330

uniform float near;
uniform float far;

in vec3 nearPoint;
in vec3 farPoint;

in mat4 fragView;
in mat4 fragProj;

out vec4 FragColor;

#define lineSize 5.f

vec4 grid(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord) * lineSize;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1) * lineSize;
    float minimumx = min(derivative.x, 1) * lineSize;
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -0.1f * minimumx && fragPos3D.x < 0.1f * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1f * minimumz && fragPos3D.z < 0.1f * minimumz)
        color.x = 1.0;
    return color;
}
float computeDepth(vec3 pos) {
    float far=gl_DepthRange.far; float near=gl_DepthRange.near;

    vec4 eye_space_pos = fragView * vec4(pos, 1);
    vec4 clip_space_pos = fragProj * eye_space_pos;

    float ndc_depth = clip_space_pos.z / clip_space_pos.w;

    return (((far-near) * ndc_depth) + near + far) / 2.0;
}

float computeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = fragProj * fragView * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    return linearDepth / far; // normalize
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    float linearDepth = computeLinearDepth(fragPos3D);
    float fading = max(0, (0.5 - linearDepth));

    FragColor = (grid(fragPos3D, 1, true) + grid(fragPos3D, 0.1, true))* float(t > 0); // adding multiple resolution for the grid
    FragColor.a *= fading;
    if (FragColor.a <= 0.f)
        discard;
}