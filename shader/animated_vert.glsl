#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Normal;
layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform mat4 finalBonesMatrices[MAX_BONES];

uniform bool hasAnimation;

out vec2 texCoord;

out vec3 normal;

out vec3 FragPos;

void main() {
    vec4 totalPosition = vec4(0.0f);
    vec4 totalNormal = vec4(0.0f);

    if (hasAnimation) {
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
            if(BoneIDs[i] == -1)
                continue;
            if(BoneIDs[i] >= MAX_BONES) {
                totalPosition = vec4(Position, 1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[BoneIDs[i]] * vec4(Position, 1.0f);
            totalPosition += localPosition * Weights[i];

            vec3 localNormal = mat3(finalBonesMatrices[BoneIDs[i]]) * Normal.xyz;
            totalNormal.xyz += localNormal * Weights[i];
        }
    } else {
        totalPosition = vec4(Position, 1.0f);
        totalNormal = Normal;
    }

//    mat4 modelView = ViewMatrix * ModelMatrix;
    FragPos = vec3(ModelMatrix * totalPosition);
    normal = normalize(mat3(transpose(inverse(ModelMatrix))) * totalNormal.xyz);
    texCoord = TexCoord;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(FragPos, 1.0f);
}