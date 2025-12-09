#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    vec4 color = texture(screenTexture, TexCoord);

    // Luminance formula (weighted average for human eye perception)
    float gray = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;

    FragColor = vec4(vec3(gray), color.a);
}