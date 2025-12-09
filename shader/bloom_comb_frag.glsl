#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float bloomStrength = 0.04;

void main() {
    vec3 scene = texture(sceneTexture, TexCoord).rgb;
    vec3 bloom = texture(bloomTexture, TexCoord).rgb;

    // Additive blending
    vec3 result = scene + bloom * bloomStrength;

    FragColor = vec4(result, 1.0);
}