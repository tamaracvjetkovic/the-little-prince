#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIds;
layout (location = 4) in vec4 aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool isAnimated;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    
    if (isAnimated) {
        float weightSum = 0.0;
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if(aBoneIds[i] != -1 && aBoneIds[i] < MAX_BONES) {
                weightSum += aWeights[i];
            }
        }

        if (weightSum > 0.0) {
            for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
                if(aBoneIds[i] == -1) continue;
                if(aBoneIds[i] >= MAX_BONES) break;
                
                float weight = aWeights[i] / weightSum;
                vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0f);
                totalPosition += localPosition * weight;
                
                vec3 localNormal = mat3(finalBonesMatrices[aBoneIds[i]]) * aNormal;
                totalNormal += localNormal * weight;
            }
        } else {
            totalPosition = vec4(aPos, 1.0f);
            totalNormal = aNormal;
        }
    } else {
        totalPosition = vec4(aPos, 1.0f);
        totalNormal = aNormal;
    }

    FragPos = vec3(model * totalPosition);
    Normal = mat3(transpose(inverse(model))) * totalNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
