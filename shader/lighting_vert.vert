#version 330 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

out vec2 texCoord;
out vec3 FragPos;    // Position in world space
out vec3 Normal_worldspace;

void main() {
    // World space position
    FragPos = vec3(ModelMatrix * vec4(Position, 1.0));

    // Normal in world space (need normal matrix for non-uniform scaling)
    mat3 normalMatrix = mat3(transpose(inverse(ModelMatrix)));
    Normal_worldspace = normalize(normalMatrix * Normal);

    //texture coordinates
    texCoord = TexCoord;

    // Final position
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(FragPos, 1.0);
}