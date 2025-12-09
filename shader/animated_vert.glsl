#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

layout(location = 5) in mat4 InstanceMatrix;
layout(location = 9) in float InstanceTextureIndex;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform mat4 finalBonesMatrices[MAX_BONES];

uniform bool hasAnimation;
uniform bool useInstancing;

out vec2 texCoord;

out vec3 normal;

out vec3 FragPos;

flat out int textureIndex;

void main() {
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0f);

    if (hasAnimation) {
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
            if(BoneIDs[i] == -1)
                continue;
            if(BoneIDs[i] >= MAX_BONES) {
                break;
            }
            vec4 localPosition = finalBonesMatrices[BoneIDs[i]] * vec4(Position, 1.0);
            totalPosition += localPosition * Weights[i];

            vec3 localNormal = mat3(finalBonesMatrices[BoneIDs[i]]) * Normal;
            totalNormal += localNormal * Weights[i];
        }
    } else {
        totalPosition = vec4(Position, 1.0);
        totalNormal = Normal;
    }

    mat4 finalModelMatrix = useInstancing ? InstanceMatrix : ModelMatrix;

    vec4 worldPos = finalModelMatrix * totalPosition;

    FragPos = worldPos.xyz;
    normal = normalize(mat3(transpose(inverse(finalModelMatrix))) * totalNormal);
    texCoord = TexCoord;
    textureIndex = int(InstanceTextureIndex);

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}