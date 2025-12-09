#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

out vec2 texCoord;
out vec3 FragPos;
out vec3 normal;
out mat3 TBN;

void main() {
    texCoord = TexCoord;

    // Fragment position in world space
    FragPos = vec3(ModelMatrix * vec4(Position, 1.0));

    // Normal transformation
    mat3 normalMatrix = mat3(transpose(inverse(ModelMatrix)));
    normal = normalize(normalMatrix * Normal);

    // Calculate TBN matrix
    vec3 T, B, N;
    N = normal;

    // Check if tangent is valid (non-zero)
    if (length(Tangent) < 0.01) {
        // Generate tangent from normal when tangents are missing
        vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, N));
        B = cross(N, T);
    } else {
        // Use provided tangent and bitangent
        T = normalize(normalMatrix * Tangent);
        B = normalize(normalMatrix * Bitangent);

        // Reorthogonalize T with respect to N
        T = normalize(T - dot(T, N) * N);

        vec3 B_new = cross(N, T);
        float handedness = dot(cross(N, T), B) > 0.0 ? 1.0 : -1.0;
        B = B_new * handedness;
    }

    TBN = mat3(T, B, N);

    // Final position
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);
}
