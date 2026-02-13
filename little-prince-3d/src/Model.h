#ifndef MODEL_H
#define MODEL_H

// model loading and skeletal animation
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "Shader.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "stb_image.h"

using namespace std;
using namespace glm;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

struct BoneInfo {
    int id;
    mat4 offset;
};

class Model {
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;

    string directory;
    bool gammaCorrection;

    // skeletal animation bones
    map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;
    mat4 m_GlobalInverseTransform;
    const aiScene* m_Scene = nullptr;
    Assimp::Importer m_Importer; 

    Model(string const &path, bool flipUVs = false, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path, flipUVs);
    }

    void Draw(Shader &shader) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    // custom texture override
    void SetTexture(unsigned int id, string type, string meshName = "") {
        for (auto &mesh : meshes) {
            if (!meshName.empty() && mesh.name != meshName) continue;
            // remove old textures of same type
            for (auto it = mesh.textures.begin(); it != mesh.textures.end(); ) {
                if (it->type == type) it = mesh.textures.erase(it);
                else ++it;
            }
            Texture t;
            t.id = id;
            t.type = type;
            mesh.textures.push_back(t);
        }
    }

    // skeletal animation bone matrices
    void GetAnimationTransforms(float timeInSeconds, vector<mat4>& transforms, bool loop = true) {
        transforms.resize(m_BoneCounter, mat4(1.0f));
        if (!m_Scene || m_Scene->mNumAnimations == 0) return;

        float duration = (float)m_Scene->mAnimations[0]->mDuration;
        float ticksPerSecond = (float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f);
        float timeInTicks = timeInSeconds * ticksPerSecond;
        float animationTime = 0.0f;
        // handle loop or once
        if (loop) {
            animationTime = (duration > 0.0f) ? fmod(timeInTicks, duration) : 0.0f;
        } else {
            animationTime = (timeInTicks < duration) ? timeInTicks : duration;
        }

        ReadNodeHierarchy(animationTime, m_Scene->mRootNode, mat4(1.0f), transforms);
    }

private:
    void loadModel(string const &path, bool flipUVs) {
        unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals;
        if (flipUVs) flags |= aiProcess_FlipUVs;
        m_Scene = m_Importer.ReadFile(path, flags);
        if (!m_Scene || m_Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_Scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << m_Importer.GetErrorString() << endl;
            return;
        }

        // directory = path.substr(0, path.find_last_of('/'));
        
        // try textures subfolder if local fails
        directory = path.substr(0, path.find_last_of('/'));
        if (directory.find("/source") != string::npos) {
            directory = directory.substr(0, directory.find("/source")) + "/textures";
        } else {
            // check for textures folder in model directory
            string texturesDir = directory + "/textures";
        }
        
        m_GlobalInverseTransform = inverse(AssimpToGlmMatrix(m_Scene->mRootNode->mTransformation));
        
        processNode(m_Scene->mRootNode, m_Scene);
    }

    void processNode(aiNode *node, const aiScene *scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        string meshName = mesh->mName.C_Str();
        
        // Debug: Print mesh name to help user find the correct names for SetTexture
        cout << "Processing mesh: " << meshName << endl;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            if (mesh->mTextureCoords[0]) {
                vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            } else
                vertex.TexCoords = vec2(0.0f, 0.0f);
            
            vertices.push_back(vertex);
        }

        // extract bone weights for skeletal animation
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            int boneID = -1;
            string boneName = mesh->mBones[i]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpToGlmMatrix(mesh->mBones[i]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            } else {
                boneID = m_BoneInfoMap[boneName].id;
            }

            auto weights = mesh->mBones[i]->mWeights;
            int numWeights = mesh->mBones[i]->mNumWeights;
            for (int weightIndex = 0; weightIndex < numWeights; weightIndex++) {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                
                // add bone influence to vertex
                for (int j = 0; j < MAX_BONE_INFLUENCE; j++) {
                    if (vertices[vertexId].m_BoneIDs[j] < 0) {
                        vertices[vertexId].m_Weights[j] = weight;
                        vertices[vertexId].m_BoneIDs[j] = boneID;
                        break;
                    }
                }
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        // diffuse / base color maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        // gltf uses base_color usually
        vector<Texture> baseColorMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_diffuse", scene);
        textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());

        // specular / metallic maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // gltf metallic-roughness handling
        vector<Texture> metallicMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_specular", scene);
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

        vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness", scene);
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
        
        // normal maps
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        return Mesh(vertices, indices, textures, meshName);
    }

    unsigned int TextureFromEmbedded(const aiTexture* embeddedTexture) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        
        int width, height, nrComponents;
        unsigned char* data = nullptr;
        if (embeddedTexture->mHeight == 0) {
            // compressed (jpg, png)
            data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth, &width, &height, &nrComponents, 0);
        } else {
            // uncompressed raw data
            data = reinterpret_cast<unsigned char*>(embeddedTexture->pcData);
            width = embeddedTexture->mWidth;
            height = embeddedTexture->mHeight;
            nrComponents = 4; // Assuming RGBA for raw
        }

        if (data) {
            GLenum format = GL_RGBA;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if (embeddedTexture->mHeight == 0) stbi_image_free(data);
        } else {
            // Suppress error log if data is null but it was expected? 
            // stbi_load_from_memory might return null for some formats.
        }
        return textureID;
    }

    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName, const aiScene* scene) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if(strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                Texture texture;
                const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
                if (embeddedTexture) {
                    texture.id = TextureFromEmbedded(embeddedTexture);
                } else {
                    texture.id = TextureFromFile(str.C_Str(), this->directory);
                }
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        
        // if glb has no diffuse, check fallbacks
        if (textures.empty() && type == aiTextureType_DIFFUSE) {
            // check emissive, lightmap, ambient for base color
            aiTextureType fallbacks[] = { aiTextureType_EMISSIVE, aiTextureType_LIGHTMAP, aiTextureType_AMBIENT };
            for (auto fType : fallbacks) {
                if (mat->GetTextureCount(fType) > 0) {
                    return loadMaterialTextures(mat, fType, typeName, scene);
                }
            }
        }

        return textures;
    }

    // bone matrices calculation
    void ReadNodeHierarchy(float animationTime, const aiNode* node, mat4 parentTransform, vector<mat4>& transforms) {
        string nodeName(node->mName.data);
        const aiAnimation* animation = m_Scene->mAnimations[0];
        mat4 nodeTransformation = AssimpToGlmMatrix(node->mTransformation);

        const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);
        if (nodeAnim) {
            // interpolate scale, rotation, translation
            mat4 scaling = scale(mat4(1.0f), InterpolateScaling(animationTime, nodeAnim));
            mat4 rotation = mat4_cast(InterpolateRotation(animationTime, nodeAnim));
            mat4 translation = translate(mat4(1.0f), InterpolateTranslation(animationTime, nodeAnim));
            nodeTransformation = translation * rotation * scaling;
        }

        mat4 globalTransformation = parentTransform * nodeTransformation;

        if (m_BoneInfoMap.find(nodeName) != m_BoneInfoMap.end()) {
            int boneIndex = m_BoneInfoMap[nodeName].id;
            transforms[boneIndex] = m_GlobalInverseTransform * globalTransformation * m_BoneInfoMap[nodeName].offset;
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ReadNodeHierarchy(animationTime, node->mChildren[i], globalTransformation, transforms);
        }
    }

    const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, string nodeName) {
        for (unsigned int i = 0; i < animation->mNumChannels; i++) {
            const aiNodeAnim* nodeAnim = animation->mChannels[i];
            if (string(nodeAnim->mNodeName.data) == nodeName) return nodeAnim;
        }
        return nullptr;
    }

    vec3 InterpolateScaling(float time, const aiNodeAnim* nodeAnim) {
        if (nodeAnim->mNumScalingKeys == 0) return vec3(1.0f);
        if (nodeAnim->mNumScalingKeys == 1) return vec3(nodeAnim->mScalingKeys[0].mValue.x, nodeAnim->mScalingKeys[0].mValue.y, nodeAnim->mScalingKeys[0].mValue.z);
        unsigned int index = 0;
        for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
            if (time < (float)nodeAnim->mScalingKeys[i + 1].mTime) { index = i; break; }
        }
        float t1 = (float)nodeAnim->mScalingKeys[index].mTime;
        float t2 = (float)nodeAnim->mScalingKeys[index + 1].mTime;
        float factor = (t2 - t1 > 0.0f) ? (time - t1) / (t2 - t1) : 0.0f;
        aiVector3D start = nodeAnim->mScalingKeys[index].mValue;
        aiVector3D end = nodeAnim->mScalingKeys[index + 1].mValue;
        return vec3(start.x + factor * (end.x - start.x), start.y + factor * (end.y - start.y), start.z + factor * (end.z - start.z));
    }

    quat InterpolateRotation(float time, const aiNodeAnim* nodeAnim) {
        if (nodeAnim->mNumRotationKeys == 0) return quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (nodeAnim->mNumRotationKeys == 1) return quat(nodeAnim->mRotationKeys[0].mValue.w, nodeAnim->mRotationKeys[0].mValue.x, nodeAnim->mRotationKeys[0].mValue.y, nodeAnim->mRotationKeys[0].mValue.z);
        unsigned int index = 0;
        for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
            if (time < (float)nodeAnim->mRotationKeys[i + 1].mTime) { index = i; break; }
        }
        float t1 = (float)nodeAnim->mRotationKeys[index].mTime;
        float t2 = (float)nodeAnim->mRotationKeys[index + 1].mTime;
        float factor = (t2 - t1 > 0.0f) ? (time - t1) / (t2 - t1) : 0.0f;
        aiQuaternion start = nodeAnim->mRotationKeys[index].mValue;
        aiQuaternion end = nodeAnim->mRotationKeys[index + 1].mValue;
        aiQuaternion res;
        aiQuaternion::Interpolate(res, start, end, factor);
        return quat(res.w, res.x, res.y, res.z);
    }

    vec3 InterpolateTranslation(float time, const aiNodeAnim* nodeAnim) {
        if (nodeAnim->mNumPositionKeys == 0) return vec3(0.0f);
        if (nodeAnim->mNumPositionKeys == 1) return vec3(nodeAnim->mPositionKeys[0].mValue.x, nodeAnim->mPositionKeys[0].mValue.y, nodeAnim->mPositionKeys[0].mValue.z);
        unsigned int index = 0;
        for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
            if (time < (float)nodeAnim->mPositionKeys[i + 1].mTime) { index = i; break; }
        }
        float t1 = (float)nodeAnim->mPositionKeys[index].mTime;
        float t2 = (float)nodeAnim->mPositionKeys[index + 1].mTime;
        float factor = (t2 - t1 > 0.0f) ? (time - t1) / (t2 - t1) : 0.0f;
        aiVector3D start = nodeAnim->mPositionKeys[index].mValue;
        aiVector3D end = nodeAnim->mPositionKeys[index + 1].mValue;
        return vec3(start.x + factor * (end.x - start.x), start.y + factor * (end.y - start.y), start.z + factor * (end.z - start.z));
    }

    static mat4 AssimpToGlmMatrix(aiMatrix4x4 from) {
        mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }
};

inline unsigned int TextureFromFile(const char *path, const string &directory, bool gamma) {
    string filename = string(path);
    
    // handle absolute paths
    size_t last_slash = filename.find_last_of("\\/");
    if (last_slash != string::npos) {
        filename = filename.substr(last_slash + 1);
    }
    
    string fullPath = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        // try textures subfolder if not found
        if (directory.find("/textures") == string::npos) {
            string texturesPath = directory + "/textures/" + filename;
            data = stbi_load(texturesPath.c_str(), &width, &height, &nrComponents, 0);
            if (data) fullPath = texturesPath;
        }
    }

    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        // try .png fallback
        if (filename.find(".png") == string::npos) {
            string pngFallback = fullPath + ".png";
            data = stbi_load(pngFallback.c_str(), &width, &height, &nrComponents, 0);
            
            // check /textures subfolder for .png
            if (!data && directory.find("/textures") == string::npos) {
                pngFallback = directory + "/textures/" + filename + ".png";
                data = stbi_load(pngFallback.c_str(), &width, &height, &nrComponents, 0);
            }

            if (data) {
                GLenum format;
                if (nrComponents == 1) format = GL_RED;
                else if (nrComponents == 3) format = GL_RGB;
                else if (nrComponents == 4) format = GL_RGBA;
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                stbi_image_free(data);
                return textureID;
            }
        }
        cout << "Texture failed to load at path: " << fullPath << " (Original was: " << path << ")" << endl;
        stbi_image_free(data);
    }
    return textureID;
}
#endif
