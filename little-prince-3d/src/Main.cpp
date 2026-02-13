#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Util.h"

using namespace std;
using namespace glm;


int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
const float TARGET_FPS = 75.0f;
const float TARGET_FRAME_TIME = 1.0f / TARGET_FPS;

bool DEPTH_TEST = true;
int CULL_MODE = 0; // 0: back, 1: front, 2: off
float DELTA_TIME = 0.0f;
float LAST_FRAME = 0.0f;

// planet
float PLANET_ROTATION = 0.0f;
int CURRENT_PLANET_IDX = 0;
float PLANET_RADIUS = 10.0f;
float DAY_NIGHT_CYCLE = 0.0f;

// textures
unsigned int studentInfoTex = 0;
unsigned int roseCursorTex = 0;
unsigned int roseCursorPressedTex = 0;
unsigned int skyTexture = 0;
unsigned int planetTextures[8]; // textures for planets
unsigned int npcTextures[8];
unsigned int quoteTextures[8];
unsigned int foxQuoteTexture = 0;
unsigned int venusTexture = 0;

// models
Model* planetModel = nullptr;

// prince models
Model* princeIdle = nullptr;
Model* princeWalk = nullptr;
Model* princeJump = nullptr;
Model* princeGreet = nullptr;
Model* princeBow = nullptr;

// fox models
Model* foxIdle = nullptr;
Model* foxRun  = nullptr;
Model* foxJump = nullptr;
Model* foxSit  = nullptr;

// Prince sitting
Model* princeSitDown = nullptr;
Model* princeSitting = nullptr;
Model* princeSitUp   = nullptr;

Model* npcModels[8];

// prince pbr textures
unsigned int princeBaseColor = 0;
unsigned int princeNormal = 0;
unsigned int princeMetallic = 0;
unsigned int princeRoughness = 0;

// arrogant man textures
unsigned int arrogantManBaseColor = 0;
unsigned int arrogantManNormal = 0;
unsigned int arrogantManMetallic = 0;
unsigned int arrogantManRoughness = 0;

// drunkard textures
unsigned int drunkardBaseColor = 0;
unsigned int drunkardNormal = 0;
unsigned int drunkardMetallic = 0;
unsigned int drunkardRoughness = 0;

unsigned int drunkardHeadBaseColor = 0;
unsigned int drunkardHeadNormal = 0;
unsigned int drunkardHeadMetallic = 0;
unsigned int drunkardHeadRoughness = 0;

unsigned int drunkardCapBaseColor = 0;

unsigned int drunkardGlassesBaseColor = 0;
unsigned int drunkardGlassesNormal = 0;
unsigned int drunkardGlassesMetallic = 0;
unsigned int drunkardGlassesRoughness = 0;

unsigned int drunkardFlaskBaseColor = 0;
unsigned int drunkardFlaskNormal = 0;
unsigned int drunkardFlaskMetallic = 0;
unsigned int drunkardFlaskRoughness = 0;

unsigned int drunkardFlowerBaseColor = 0;
unsigned int drunkardFlowerNormal = 0;
unsigned int drunkardFlowerMetallic = 0;
unsigned int drunkardFlowerRoughness = 0;

unsigned int drunkardSockBaseColor = 0;
unsigned int drunkardSockNormal = 0;
unsigned int drunkardSockMetallic = 0;
unsigned int drunkardSockRoughness = 0;

// business man textures
unsigned int businessManSuitBaseColor = 0;
unsigned int businessManHeadBaseColor = 0;
unsigned int businessManShoesBaseColor = 0;
unsigned int businessManShoesNormal = 0;
unsigned int businessManEyeBaseColor = 0;
unsigned int businessManEyebrowBaseColor = 0;
unsigned int businessManTeethBaseColor = 0;
unsigned int businessManTongueBaseColor = 0;

// HUD VAO/VBO
unsigned int hudVAO, hudVBO;

// cloud VAO/VBO
unsigned int cloudVAO = 0, cloudVBO = 0;
int CLOUD_COUNT = 300; // Decreased from 600 to 300
struct Cloud {
    vec3 position;
    vec2 size;
    float rotation;
    int type; // 0: shape, 1: line
};
vector<Cloud> clouds;

// sphere and cube placeholders
unsigned int cubeVAO, cubeVBO;
unsigned int sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
int sphereIndicesCount = 0;

// star VAO/VBO
unsigned int starVAO = 0, starVBO = 0;
int STAR_COUNT = 1000;

// Quote scales from 2D project
constexpr float QUOTE_SCALE_X[8] = { 1.1f, 1.1f, 1.17f, 1.25f, 1.08f, 0.72f, 1.1f, 1.0f };
constexpr float QUOTE_SCALE_Y[8] = { 0.93f, 0.93f, 0.92f, 0.6f, 0.9f, 0.64f, 1.0f, 1.0f };
constexpr float QUOTE_WIDTHS[8] = { 310.0f, 310.0f, 310.0f, 310.0f, 310.0f, 310.0f, 310.0f, 310.0f };
constexpr float QUOTE_HEIGHTS[8] = { 130.0f, 130.0f, 130.0f, 160.0f, 130.0f, 130.0f, 130.0f, 130.0f };

// Decorations
Model* baobabModel = nullptr;
Model* baobab2Model = nullptr;
Model* roseModel = nullptr;
Model* wildPlantModel = nullptr;
Model* kingModel = nullptr;
Model* throneModel = nullptr;
Model* arrogantManModel = nullptr;
Model* drunkardModel = nullptr;
Model* businessManModel = nullptr;
Model* tableModel = nullptr;
Model* mirrorModel = nullptr;
Model* workingDeskModel = nullptr;
Model* oldManModel = nullptr;
Model* lampModel = nullptr;
Model* professorModel = nullptr;
Model* deskModel = nullptr;
Model* aviatorModel = nullptr;
Model* aircraftModel = nullptr;

struct Decoration {
    vec3 direction;
    float scale;
    Model* model;
};
vector<Decoration> planetDecorations[8];

float quoteAlpha = 0.0f;


// --- code ---

enum class CharacterState { IDLE, MOVING, JUMPING, GREETING, SITTING_DOWN, SITTING, SITTING_UP, BOWING };

struct Character {
    vec3 direction = vec3(0, 1, 0); // sphere position
    float yaw = 0.0f; // rotation around normal
    float speed = 3.5f;
    float orbitAngle = 0.0f; // camera orbit angle
    float modelScale = 0.1f; // model scale
    mat4 orientation = mat4(1.0f); // model orientation fix

    // animation and states
    CharacterState state = CharacterState::IDLE;
    float animTimer = 0.0f;
    float animationStartTime = 0.0f;
    bool hasBowedInThisOverlap = false;
    bool wasOverlapping = false; // no instant prince re-sit

    void update(float dt) {
        if (animTimer > 0) {
            animTimer -= dt;
            if (animTimer <= 0) {
                if (state == CharacterState::SITTING_DOWN) {
                    state = CharacterState::SITTING;
                    animationStartTime = (float)glfwGetTime();
                } else if (state == CharacterState::SITTING_UP) {
                    state = CharacterState::IDLE;
                    animationStartTime = (float)glfwGetTime();
                } else if (state == CharacterState::BOWING) {
                    state = CharacterState::IDLE;
                    animationStartTime = (float)glfwGetTime();
                } else {
                    state = CharacterState::IDLE;
                    animationStartTime = (float)glfwGetTime();
                }
                animTimer = 0;
            }
        }
    }

    mat4 getTransform(float radius, float planetRotY);

    void move(float forwardInput, float sideInput, float radius, float dt, const Camera &cam, bool restricted = false, vec3 otherPos = vec3(0), int currentPlanetIdx = 0) {
        if (state == CharacterState::JUMPING || state == CharacterState::GREETING || 
            state == CharacterState::SITTING_DOWN || state == CharacterState::SITTING_UP ||
            state == CharacterState::BOWING) return;

        if (abs(forwardInput) < 0.001f && abs(sideInput) < 0.001f) {
            if (state == CharacterState::MOVING) {
                state = CharacterState::IDLE;
                animationStartTime = (float)glfwGetTime();
            }
            return;
        }

        if (state == CharacterState::SITTING) {
            // stand up
            state = CharacterState::SITTING_UP;
            animTimer = 2.9f; 
            animationStartTime = (float)glfwGetTime();
            return;
        }

        if (state != CharacterState::MOVING) {
            state = CharacterState::MOVING;
            animationStartTime = (float)glfwGetTime();
        }
        
        vec3 up = direction;
        
        // movement relative to camera
        vec3 camFront = cam.Front;
        vec3 camRight = cam.Right;
        
        vec3 moveForward = normalize(camFront - dot(camFront, up) * up);
        vec3 moveRight = normalize(camRight - dot(camRight, up) * up);
        
        vec3 desiredMoveDir = forwardInput * moveForward + sideInput * moveRight;
        
        if (length(desiredMoveDir) > 0.001f) {
            desiredMoveDir = normalize(desiredMoveDir);

            if (restricted) {
                // move away from otherPos
                vec3 awayDir = normalize(direction - otherPos);
                if (dot(desiredMoveDir, awayDir) < 0.2f) { 
                    if (state == CharacterState::MOVING) {
                        state = CharacterState::IDLE;
                        animationStartTime = (float)glfwGetTime();
                    }
                    return; 
                }
            }
            
            // sphere movement
            float dist = speed * dt;
            float angle = dist / radius;
            vec3 rotationAxis = normalize(cross(up, desiredMoveDir));
            mat4 moveRot = rotate(mat4(1.0f), angle, rotationAxis);
            vec3 nextDirection = normalize(vec3(moveRot * vec4(direction, 0.0f)));

            // collision with decorations
            bool collision = false;
            for (auto &p : planetDecorations[currentPlanetIdx]) {
                    if (p.model == wildPlantModel || p.model == roseModel) continue;

                    float collisionRadius = p.scale * 10.0f;
                    // collision radius adjustments
                    if (p.model == kingModel) collisionRadius = 2.0f;
                    if (p.model == arrogantManModel) collisionRadius = 2.0f;
                    if (p.model == drunkardModel) collisionRadius = 2.0f;
                    if (p.model == businessManModel) collisionRadius = 2.0f;
                    if (p.model == oldManModel) collisionRadius = 2.0f;
                    if (p.model == professorModel) collisionRadius = 2.0f;
                    if (p.model == aviatorModel) collisionRadius = 2.0f;
                    if (p.model == workingDeskModel) collisionRadius = 1.2f;
                    if (p.model == deskModel) collisionRadius = 1.2f;
                    if (p.model == aircraftModel) collisionRadius = 1.5f;
                    if (p.model == lampModel) collisionRadius = 0.5f;
                    if (p.model == throneModel) collisionRadius = 1.0f;
                    if (p.model == tableModel) {
                        // table is only not included in collision for Drunkard (Planet 4)
                        if (currentPlanetIdx == 3) continue;
                        collisionRadius = 1.0f;
                    }

                    if (distance(nextDirection, p.direction) * radius < collisionRadius) {
                        collision = true;
                        break;
                    }
                }

            if (collision) {
                if (state == CharacterState::MOVING) {
                    state = CharacterState::IDLE;
                    animationStartTime = (float)glfwGetTime();
                }
                return;
            }

            direction = nextDirection;
            
            // character always looks in movement direction
            vec3 ref = (abs(up.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
            vec3 rightBasis = normalize(cross(up, ref));
            vec3 forwardBasis = cross(up, rightBasis);
            
            float targetYaw = degrees(atan2(dot(desiredMoveDir, rightBasis), dot(desiredMoveDir, forwardBasis)));
            yaw = -targetYaw;
        }
    }
};

Character prince, fox;


// --- helper methods ---

void initializeGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* setupWindow() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    SCREEN_WIDTH = mode->width;
    SCREEN_HEIGHT = mode->height;

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "The Little Prince 3D", monitor, NULL);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // hide system cursor, we draw our own
    return window;
}

float getPlanetBump(float sectorAngle, float stackAngle, int planetIdx) {
    float bump = 0.0f;
    if (planetIdx == 0) { // 1) a lot smaller bumps, only on some places
        bump = 0.04f * powf(sinf(sectorAngle * 6.0f) * cosf(stackAngle * 3.0f), 4.0f);
    } else if (planetIdx == 1) { // 2) is okay (very small)
        bump = 0.01f * sinf(sectorAngle * 20.0f) * cosf(stackAngle * 20.0f);
    } else if (planetIdx == 2) { // 3) a little bit smaller moderate bumps
        bump = 0.04f * sinf(sectorAngle * 8.0f) * cosf(stackAngle * 4.0f) + 
               0.015f * cosf(sectorAngle * 15.0f + stackAngle * 10.0f);
    } else if (planetIdx == 3) { // 4) same as 3 but on less places
        float raw = sinf(sectorAngle * 8.0f) * cosf(stackAngle * 4.0f);
        bump = 0.04f * powf(fmaxf(0.0f, raw), 2.0f);
    } else if (planetIdx == 4) { // 5) almost flat
        bump = 0.003f * sinf(sectorAngle * 10.0f);
    } else if (planetIdx == 5) { // 6) similar to 1
        bump = 0.035f * powf(sinf(sectorAngle * 5.0f) * cosf(stackAngle * 2.5f), 2.0f);
    } else if (planetIdx == 6) { // 7) almost flat
        bump = 0.003f * cosf(stackAngle * 10.0f);
    } else if (planetIdx == 7) { // 8) earth-like bumps
        bump = 0.02f * sinf(sectorAngle * 4.0f) * cosf(stackAngle * 2.0f) +
               0.01f * sinf(sectorAngle * 12.0f) * cosf(stackAngle * 6.0f) +
               0.005f * cosf(sectorAngle * 30.0f);
    }
    return bump;
}

mat4 Character::getTransform(float radius, float planetRotY) {
    mat4 planetM = rotate(mat4(1.0f), radians(planetRotY), vec3(0, 1, 0));
    
    vec3 up = direction;

    // calculate bump at current position
    float stackAngle = asinf(direction.z);
    float sectorAngle = atan2f(direction.y, direction.x);
    if (sectorAngle < 0) sectorAngle += 2.0f * M_PI; // normal to [0, 2pi]

    float bump = getPlanetBump(sectorAngle, stackAngle, CURRENT_PLANET_IDX);
    float actualRadius = radius * (1.0f + bump);

    vec3 ref = (abs(up.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 right = normalize(cross(up, ref));
    vec3 forward = cross(up, right);
    
    // OpenGL right-handed system (x=right, y=up, z=forward)
    mat4 rotYaw = rotate(mat4(1.0f), radians(yaw), up);
    vec3 actualForward = vec3(rotYaw * vec4(forward, 0.0f));
    vec3 actualRight = normalize(cross(up, actualForward));
    
    mat4 localM(1.0f);
    localM[0] = vec4(actualRight, 0.0f);
    localM[1] = vec4(up, 0.0f);
    localM[2] = vec4(actualForward, 0.0f);
    localM[3] = vec4(direction * (actualRadius + 0.1f), 1.0f); 
    
    return planetM * localM * orientation * scale(mat4(1.0f), vec3(modelScale));
}

void initSphereVAO(float radius, int sectors, int stacks, int planetIdx) {
    if (sphereVAO != 0) {
        glDeleteVertexArrays(1, &sphereVAO);
        glDeleteBuffers(1, &sphereVBO);
        glDeleteBuffers(1, &sphereEBO);
    }

    vector<float> vertices;
    vector<unsigned int> indices;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * (M_PI / stacks);
        float xy_base = cosf(stackAngle);
        float z_base = sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * (2 * M_PI / sectors);
            
            float bump = getPlanetBump(sectorAngle, stackAngle, planetIdx);
            float currentRadius = radius * (1.0f + bump);

            float x = currentRadius * xy_base * cosf(sectorAngle);
            float y = currentRadius * xy_base * sinf(sectorAngle);
            float z = currentRadius * z_base;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // normals
            vertices.push_back(x / currentRadius);
            vertices.push_back(y / currentRadius);
            vertices.push_back(z / currentRadius);
            // UV
            vertices.push_back((float)j / sectors);
            vertices.push_back((float)i / stacks);
        }
    }

            for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    sphereIndicesCount = (int)indices.size();

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void initPlaceholderVAO() {
    float vertices[] = {
        // Pozicije           // Normale           // Teksturne koordinate
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void drawPlaceholderCube() {
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void initStars() {
    vector<float> starVertices;
    for (int i = 0; i < STAR_COUNT; i++) {
        // random spherical coordinates
        float theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);
        float phi = acosf(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        float r = 500.0f; // Far away

        float x = r * sinf(phi) * cosf(theta);
        float y = r * sinf(phi) * sinf(theta);
        float z = r * cosf(phi);

        starVertices.push_back(x);
        starVertices.push_back(y);
        starVertices.push_back(z);
        
        // random brightness/size variation
        starVertices.push_back(0.5f + 0.5f * ((float)rand() / RAND_MAX));
    }

    glGenVertexArrays(1, &starVAO);
    glGenBuffers(1, &starVBO);
    glBindVertexArray(starVAO);
    glBindBuffer(GL_ARRAY_BUFFER, starVBO);
    glBufferData(GL_ARRAY_BUFFER, starVertices.size() * sizeof(float), starVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}


void initClouds() {
    float vertices[] = {
        // positions         // texture coords (CCW winding)
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
    };

    if (cloudVAO == 0) {
        glGenVertexArrays(1, &cloudVAO);
        glGenBuffers(1, &cloudVBO);
        glBindVertexArray(cloudVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
    }

    clouds.clear();
    for (int i = 0; i < CLOUD_COUNT; i++) {
        Cloud c;
        // uniform distribution on a sphere (same as stars)
        float theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);
        float phi = acosf(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        float r = 250.0f + 100.0f * ((float)rand() / RAND_MAX);

        // standard spherical to cartesian (Y as pole for "up" feel)
        c.position = vec3(r * sinf(phi) * cosf(theta), r * cosf(phi), r * sinf(phi) * sinf(theta));
        
        c.type = 0; // Only puffy shapes now
        c.size.x = (50.0f + 100.0f * ((float)rand() / RAND_MAX)); 
        c.size.y = c.size.x * (0.15f + 0.25f * ((float)rand() / RAND_MAX)); 
        
        c.rotation = 0.0f; // Keep clouds horizontal
        clouds.push_back(c);
    }
}

void initHUD() {
    float vertices[] = {
        // pozicije   // UVx
        0.0f, 1.0f,   0.0f, 1.0f,
        0.0f, 0.0f,   0.0f, 0.0f,
        1.0f, 0.0f,   1.0f, 0.0f,

        0.0f, 1.0f,   0.0f, 1.0f,
        1.0f, 0.0f,   1.0f, 0.0f,
        1.0f, 1.0f,   1.0f, 1.0f
    };
    glGenVertexArrays(1, &hudVAO);
    glGenBuffers(1, &hudVBO);
    glBindVertexArray(hudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void drawHUDQuad(Shader &s, unsigned int tex, vec2 pos, vec2 size, float alphaValue = 1.0f) {
    if (tex == 0) return;
    mat4 model = mat4(1.0f);
    model = translate(model, vec3(pos, 0.0f));
    model = scale(model, vec3(size, 1.0f));
    s.setMat4("model", model);
    s.setFloat("alpha", alphaValue);
    s.setBool("useTint", false);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    s.setInt("hudTexture", 0);

    glBindVertexArray(hudVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // toggle depth test (on Z)
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        DEPTH_TEST = !DEPTH_TEST;
        if (DEPTH_TEST) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            cout << "Depth test: ON" << endl;
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            cout << "Depth test: OFF" << endl;
        }
    }

    // toggle face culling (on C)
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        CULL_MODE = (CULL_MODE == 0) ? 2 : 0; // Toggle samo između BACK (0) i OFF (2)
        if (CULL_MODE == 0) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            cout << "Face culling: ON (BACK)" << endl;
        } else {
            glDisable(GL_CULL_FACE);
            cout << "Face culling: OFF" << endl;
        }
    }

    // switch planet (1-8)
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8 && action == GLFW_PRESS) {
        CURRENT_PLANET_IDX = key - GLFW_KEY_1;
        initSphereVAO(PLANET_RADIUS, 64, 64, CURRENT_PLANET_IDX);
        cout << "Switched to planet: " << CURRENT_PLANET_IDX + 1 << endl;
    }
}

// -------- main loop --------

int main() {
    initializeGLFW();
    GLFWwindow* window = setupWindow();
    if (!window) return -1;

    glfwSetKeyCallback(window, key_callback);

    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initHUD();
    initStars();
    initClouds();
    initPlaceholderVAO();
    initSphereVAO(PLANET_RADIUS, 64, 64, CURRENT_PLANET_IDX);

    // load textures
    studentInfoTex = loadImageToTexture("res/text/student-info-350-467.png");
    roseCursorTex = loadImageToTexture("res/cursor/rose-1.png");
    roseCursorPressedTex = loadImageToTexture("res/cursor/rose-2.png");
    skyTexture = loadImageToTexture("res/misc/sky.png");
    venusTexture = loadImageToTexture("res/planets/2.jpeg");
    
    for (int i = 0; i < 8; i++) {
        // planet texture fallbacks
        string planetPath;
        if (i == 0) planetPath = "res/planets/1.png";
        else if (i == 1) planetPath = "res/planets/2.png";
        else if (i == 2) planetPath = "res/planets/3.png";
        else if (i == 3) planetPath = "res/planets/4.png";
        else if (i == 4) planetPath = "res/planets/5.png";
        else if (i == 5) planetPath = "res/planets/6.png";
        else if (i == 6) planetPath = "res/planets/7.png";
        else if (i == 7) planetPath = "res/planets/8.png";
        else planetPath = "res/planets/3.png"; // Fallback
        
        planetTextures[i] = loadImageToTexture(planetPath.c_str());

        // npc and quote textures
        string npcPath = "res/npc/" + to_string(i + 1) + ".png";
        npcTextures[i] = loadImageToTexture(npcPath.c_str());
        
        string quotePath = "res/quotes/" + to_string(i + 1) + ".png";
        quoteTextures[i] = loadImageToTexture(quotePath.c_str());
    }
    foxQuoteTexture = loadImageToTexture("res/quotes/fox.png");

    // arrogant man texture loading
    arrogantManBaseColor = loadImageToTexture("res/npc/3/arrogant-man/textures/man_Base_color.jpg");
    arrogantManNormal = loadImageToTexture("res/npc/3/arrogant-man/textures/man_Normal_OpenGL.jpg");
    arrogantManMetallic = loadImageToTexture("res/npc/3/arrogant-man/textures/man_Metallic.jpg");
    arrogantManRoughness = loadImageToTexture("res/npc/3/arrogant-man/textures/man_Roughness.jpg");

    unsigned int arrogantManEyeBaseColor = loadImageToTexture("res/npc/3/arrogant-man/textures/Trong_Basecolor.tga.png");
    unsigned int arrogantManEyeNormal = loadImageToTexture("res/npc/3/arrogant-man/textures/Trong_Normal.png");
    unsigned int arrogantManEyeRoughness = loadImageToTexture("res/npc/3/arrogant-man/textures/Trong_rougness.png");

    drunkardBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_vmtr_c.png");
    drunkardNormal = loadImageToTexture("res/npc/4/drunkard/textures/josef_vmtr_n.png");
    drunkardMetallic = loadImageToTexture("res/npc/4/drunkard/textures/josef_vmtr_s.png");
    drunkardRoughness = loadImageToTexture("res/npc/4/drunkard/textures/josef_vmtr_r.png");

    drunkardHeadBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_head_vmtr_c.png");
    drunkardHeadNormal = loadImageToTexture("res/npc/4/drunkard/textures/josef_head_vmtr_n.png");
    drunkardHeadMetallic = loadImageToTexture("res/npc/4/drunkard/textures/josef_head_vmtr_s.png");
    drunkardHeadRoughness = loadImageToTexture("res/npc/4/drunkard/textures/josef_head_vmtr_r.png");

    drunkardCapBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_cap_vmtr_c.png");
    unsigned int drunkardCapNormal = 0; // Missing: josef_cap_vmtr_n.png
    unsigned int drunkardCapMetallic = 0; // Missing: josef_cap_vmtr_s.png

    drunkardGlassesBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_glasses_2_vmtr_c.png");
    drunkardGlassesNormal = loadImageToTexture("res/npc/4/drunkard/textures/josef_glasses_2_vmtr_n.png");
    drunkardGlassesMetallic = loadImageToTexture("res/npc/4/drunkard/textures/josef_glasses_2_vmtr_s.png");
    drunkardGlassesRoughness = loadImageToTexture("res/npc/4/drunkard/textures/josef_glasses_2_vmtr_r.png");

    drunkardFlaskBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_hipflask_vmtr_c.png");
    drunkardFlaskNormal = loadImageToTexture("res/npc/4/drunkard/textures/josef_hipflask_vmtr_n.png");
    drunkardFlaskMetallic = loadImageToTexture("res/npc/4/drunkard/textures/josef_hipflask_vmtr_s.png");
    drunkardFlaskRoughness = loadImageToTexture("res/npc/4/drunkard/textures/josef_hipflask_vmtr_r.png");

    drunkardFlowerBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_flower_vmtr_c.png");
    drunkardFlowerNormal = loadImageToTexture("res/npc/4/drunkard/textures/josef_flower_vmtr_n.png");
    drunkardFlowerMetallic = loadImageToTexture("res/npc/4/drunkard/textures/josef_flower_vmtr_s.png");
    drunkardFlowerRoughness = loadImageToTexture("res/npc/4/drunkard/textures/josef_flower_vmtr_r.png");

    drunkardSockBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/josef_sock_vmtr_c.png");
    drunkardSockNormal = loadImageToTexture("res/npc/4/drunkard/textures/josef_sock_vmtr_n.png");
    drunkardSockMetallic = loadImageToTexture("res/npc/4/drunkard/textures/josef_sock_vmtr_s.png");
    drunkardSockRoughness = loadImageToTexture("res/npc/4/drunkard/textures/josef_sock_vmtr_r.png");

    unsigned int eyeBaseColor = loadImageToTexture("res/npc/4/drunkard/textures/eyes_brown.png");

    // business man texture loading
    businessManSuitBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/male_elegantsuit01_diffuse.png");
    businessManHeadBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/middleage_lightskinned_male_diffuse.png");
    businessManShoesBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/Shoes_Oxford.png");
    businessManShoesNormal = loadImageToTexture("res/npc/5/bussines-man/textures/Shoes_Oxford_NRM.png");
    businessManEyeBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/brown_eye.png");
    businessManEyebrowBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/eyebrow008.png");
    businessManTeethBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/teeth.png");
    businessManTongueBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/tongue01_diffuse.png");
    unsigned int businessManHairBaseColor = loadImageToTexture("res/npc/5/bussines-man/textures/male02_diffuse_black.png");

    // planet 1 decorations
    baobabModel = new Model("res/decoration/1/baobab.glb", true);
    roseModel = new Model("res/decoration/1/rose.glb", true);
    wildPlantModel = new Model("res/decoration/1/wild_plant.glb", true);
    kingModel = new Model("res/npc/2/king.glb", true);
    throneModel = new Model("res/decoration/2/throne.glb", true);
    arrogantManModel = new Model("res/npc/3/arrogant-man/source/Talking.fbx");
    if (arrogantManBaseColor != 0) arrogantManModel->SetTexture(arrogantManBaseColor, "texture_diffuse");
    if (arrogantManNormal != 0)    arrogantManModel->SetTexture(arrogantManNormal, "texture_normal");
    if (arrogantManMetallic != 0)  arrogantManModel->SetTexture(arrogantManMetallic, "texture_specular");
    if (arrogantManRoughness != 0) arrogantManModel->SetTexture(arrogantManRoughness, "texture_roughness");

    drunkardModel = new Model("res/npc/4/drunkard/source/drunkard.fbx");
    auto applyDrunkardTextures = [&](string meshName, unsigned int diffuse, unsigned int normal, unsigned int specular, unsigned int roughness) {
        if (diffuse != 0) drunkardModel->SetTexture(diffuse, "texture_diffuse", meshName);
        if (normal != 0)  drunkardModel->SetTexture(normal, "texture_normal", meshName);
        if (specular != 0) drunkardModel->SetTexture(specular, "texture_specular", meshName);
        if (roughness != 0) drunkardModel->SetTexture(roughness, "texture_roughness", meshName);
    };

    // if mesh filtering fails (e.g. names don't match exactly), set a global fallback
    if (drunkardBaseColor != 0) drunkardModel->SetTexture(drunkardBaseColor, "texture_diffuse");
    if (drunkardNormal != 0)    drunkardModel->SetTexture(drunkardNormal, "texture_normal");
    if (drunkardMetallic != 0)  drunkardModel->SetTexture(drunkardMetallic, "texture_specular");
    if (drunkardRoughness != 0) drunkardModel->SetTexture(drunkardRoughness, "texture_roughness");

    // override texture directories
    drunkardModel->directory = "res/npc/4/drunkard/textures";
    if (arrogantManModel) arrogantManModel->directory = "res/npc/3/arrogant-man/textures";

    applyDrunkardTextures("polySurfaceMesh", drunkardFlaskBaseColor, drunkardFlaskNormal, drunkardFlaskMetallic, drunkardFlaskRoughness); // hipflask
    applyDrunkardTextures("polySurfaceMesh.001", drunkardGlassesBaseColor, drunkardGlassesNormal, drunkardGlassesMetallic, drunkardGlassesRoughness); // glasses
    applyDrunkardTextures("polySurfaceMesh.002", drunkardCapBaseColor, drunkardCapNormal, drunkardCapMetallic, 0); // cap
    applyDrunkardTextures("polySurfaceMesh.003", drunkardSockBaseColor, drunkardSockNormal, drunkardSockMetallic, drunkardSockRoughness); // sock
    applyDrunkardTextures("polySurfaceMesh.004", drunkardBaseColor, drunkardNormal, drunkardMetallic, drunkardRoughness); // body
    applyDrunkardTextures("polySurfaceMesh.005", drunkardBaseColor, 0, 0, 0); // body/misc?
    applyDrunkardTextures("polySurfaceMesh.006", drunkardFlowerBaseColor, drunkardFlowerNormal, drunkardFlowerMetallic, drunkardFlowerRoughness); // flower
    applyDrunkardTextures("polySurfaceMesh.007", eyeBaseColor, 0, 0, 0); // eyes
    applyDrunkardTextures("polySurfaceMesh.008", drunkardBaseColor, 0, 0, 0); // hair?
    applyDrunkardTextures("polySurfaceMesh.009", drunkardHeadBaseColor, drunkardHeadNormal, drunkardHeadMetallic, drunkardHeadRoughness); // head
    
    businessManModel = new Model("res/npc/5/bussines-man/working.fbx");
    businessManModel->directory = "res/npc/5/bussines-man/textures";
    auto applyBusinessManTextures = [&](string meshName, unsigned int diffuse, unsigned int normal) {
        if (diffuse != 0) businessManModel->SetTexture(diffuse, "texture_diffuse", meshName);
        if (normal != 0)  businessManModel->SetTexture(normal, "texture_normal", meshName);
    };
    // default fallback
    if (businessManSuitBaseColor != 0) businessManModel->SetTexture(businessManSuitBaseColor, "texture_diffuse");

    // manual mesh mapping for business man (FBX structure)
    applyBusinessManTextures("male_elegantsuit01", businessManSuitBaseColor, 0);
    applyBusinessManTextures("man_dressed_in_suit", businessManHeadBaseColor, 0);
    applyBusinessManTextures("high-poly", businessManHeadBaseColor, 0);
    applyBusinessManTextures("shoes_oxford_male", businessManShoesBaseColor, businessManShoesNormal);
    applyBusinessManTextures("brown_eye", businessManEyeBaseColor, 0);
    applyBusinessManTextures("eyebrow008", businessManEyebrowBaseColor, 0);
    applyBusinessManTextures("teeth_base", businessManTeethBaseColor, 0);
    applyBusinessManTextures("tongue01", businessManTongueBaseColor, 0);
    applyBusinessManTextures("mhair02", businessManHairBaseColor, 0);

    // Additional Fallbacks for other possible Business Man exports
    applyBusinessManTextures("middleage_lightskinned_male", businessManHeadBaseColor, 0);
    applyBusinessManTextures("Shoes_Oxford", businessManShoesBaseColor, businessManShoesNormal);
    applyBusinessManTextures("teeth", businessManTeethBaseColor, 0);
    applyBusinessManTextures("male02", businessManHairBaseColor, 0);
    applyBusinessManTextures("body", businessManHeadBaseColor, 0);
    applyBusinessManTextures("suit", businessManSuitBaseColor, 0);
    applyBusinessManTextures("shoes", businessManShoesBaseColor, businessManShoesNormal);
    applyBusinessManTextures("eyes", businessManEyeBaseColor, 0);
    applyBusinessManTextures("hair", businessManHairBaseColor, 0);

    // arrogant man mesh mapping
    if (arrogantManModel) {
        arrogantManModel->SetTexture(arrogantManBaseColor, "texture_diffuse", "body");
        arrogantManModel->SetTexture(arrogantManNormal, "texture_normal", "body");
        arrogantManModel->SetTexture(arrogantManMetallic, "texture_specular", "body");
        arrogantManModel->SetTexture(arrogantManRoughness, "texture_roughness", "body");
        
        arrogantManModel->SetTexture(arrogantManBaseColor, "texture_diffuse", "out");
        arrogantManModel->SetTexture(arrogantManBaseColor, "texture_diffuse", "in1");

        if (arrogantManEyeBaseColor != 0) arrogantManModel->SetTexture(arrogantManEyeBaseColor, "texture_diffuse", "eye");
        if (arrogantManEyeNormal != 0)    arrogantManModel->SetTexture(arrogantManEyeNormal, "texture_normal", "eye");
        if (arrogantManEyeRoughness != 0) arrogantManModel->SetTexture(arrogantManEyeRoughness, "texture_roughness", "eye");
    }

    tableModel = new Model("res/decoration/4/table/source/table.glb", true);
    mirrorModel = new Model("res/decoration/3/mirror.glb", true);
    workingDeskModel = new Model("res/decoration/5/working_desk.glb", true);
    lampModel = new Model("res/decoration/6/lamp1.glb", true);

    planetDecorations[0].push_back({ normalize(vec3(0.5, 0.8, 0.3)), 0.15f, baobabModel }); 
    planetDecorations[0].push_back({ normalize(vec3(0.05, 0.2, 0.9)), 0.15f, baobabModel }); 
    planetDecorations[0].push_back({ normalize(vec3(0.1, -0.9, 0.1)), 0.15f, baobabModel }); 
    planetDecorations[0].push_back({ normalize(vec3(-0.7, 0.1, -0.4)), 0.15f, baobabModel }); 
    planetDecorations[0].push_back({ normalize(vec3(0.8, -0.3, -0.5)), 0.15f, baobabModel }); 

    planetDecorations[0].push_back({ normalize(vec3(0.15f, 1.0f, 0.45f)), 0.3f, roseModel }); // moved further from prince spawn normalize(vec3(0, 1, 0.2f))
    
    // a lot of wild plants
    for (int i = 0; i < 30; i++) {
        float theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);
        float phi = acosf(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        vec3 dir = normalize(vec3(sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta)));
        planetDecorations[0].push_back({ dir, 0.12f + 0.05f * ((float)rand() / RAND_MAX), wildPlantModel });
    }

    // planet 2 decorations (king)
    vec3 kingPos = normalize(vec3(0.1f, 1.0f, 1.0f));
    planetDecorations[1].push_back({ kingPos, 3.5f, kingModel });

    // throne to his side
    vec3 kingUp = kingPos;
    vec3 kingRef = (abs(kingUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 kingRight = normalize(cross(kingUp, kingRef));
    vec3 throneDir = normalize(kingPos + kingRight * 0.3f); 
    planetDecorations[1].push_back({ throneDir, 0.009f, throneModel });

    // decorations for planet 3
    // Arrogant Man should be in the same position and direction as king but on third planet
    planetDecorations[2].push_back({ kingPos, 0.1f, arrogantManModel });

    // add mirror next to him
    vec3 arrogantUp = kingPos;
    vec3 arrogantRef = (abs(arrogantUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 arrogantRight = normalize(cross(arrogantUp, arrogantRef));
    vec3 mirrorDir = normalize(kingPos + arrogantRight * 0.35f);
    planetDecorations[2].push_back({ mirrorDir, 0.018f, mirrorModel });

    // decorations for planet 4
    planetDecorations[3].push_back({ kingPos, 0.00018f, drunkardModel });

    // table next to him on the right
    vec3 drunkardUp = kingPos;
    vec3 drunkardRef = (abs(drunkardUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 drunkardRight = normalize(cross(drunkardUp, drunkardRef));
    // offset for table
    vec3 tableDir = normalize(kingPos + drunkardRight * 0.45f);
    planetDecorations[3].push_back({ tableDir, 0.8f, tableModel });

    // decorations for planet 5
    vec3 businessManPos = normalize(vec3(0.05f, 1.0f, 0.6f));
    planetDecorations[4].push_back({ businessManPos, 0.013f, businessManModel });

    // working desk in front of him
    vec3 businessUp = businessManPos;
    vec3 businessRef = (abs(businessUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 businessRight = normalize(cross(businessUp, businessRef));
    vec3 businessFront = normalize(cross(businessRight, businessUp));
    
    // rotate front vector 130 degrees to his left (around surface normal)
    mat4 businessRot = rotate(mat4(1.0f), radians(130.0f), businessUp);
    vec3 businessFrontNew = normalize(vec3(businessRot * vec4(businessFront, 0.0f)));
    
    vec3 deskDir = normalize(businessManPos + businessFrontNew * 0.12f);
    planetDecorations[4].push_back({ deskDir, 0.3f, workingDeskModel });

    // planet 6 decorations (old man)
    oldManModel = new Model("res/npc/6/old-man-idle/source/working.fbx");
    oldManModel->directory = "res/npc/6/old-man-idle/textures";
    unsigned int oldManBaseColor = loadImageToTexture("res/npc/6/old-man-idle/textures/Diffuse.png");
    unsigned int oldManNormal = loadImageToTexture("res/npc/6/old-man-idle/textures/Normals.png");
    unsigned int oldManMetallic = loadImageToTexture("res/npc/6/old-man-idle/textures/Metallic.png");
    if (oldManBaseColor != 0) oldManModel->SetTexture(oldManBaseColor, "texture_diffuse");
    if (oldManNormal != 0)    oldManModel->SetTexture(oldManNormal, "texture_normal");
    if (oldManMetallic != 0)  oldManModel->SetTexture(oldManMetallic, "texture_specular");

    planetDecorations[5].clear();
    planetDecorations[5].push_back({ kingPos, 0.016f, oldManModel });

    // lamp in front of him
    vec3 oldManUp = kingPos;
    vec3 oldManRef = (abs(oldManUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 oldManRight = normalize(cross(oldManUp, oldManRef));
    vec3 oldManFront = normalize(cross(oldManRight, oldManUp));
    mat4 oldManRot = rotate(mat4(1.0f), radians(130.0f), oldManUp);
    vec3 oldManFrontNew = normalize(vec3(oldManRot * vec4(oldManFront, 0.0f)));
    vec3 oldManRightNew = normalize(vec3(oldManRot * vec4(oldManRight, 0.0f)));

    vec3 lampDir = normalize(kingPos + oldManFrontNew * 0.1f - oldManRightNew * 0.09f);
    planetDecorations[5].push_back({ lampDir, 3.0f, lampModel });

    // planet 7 decorations (professor)
    professorModel = new Model("res/npc/7/professor-flitwick/source/professor.fbx");
    professorModel->directory = "res/npc/7/professor-flitwick/textures";
    unsigned int profFace = loadImageToTexture("res/npc/7/professor-flitwick/textures/Flitwik_face.png");
    unsigned int profDress = loadImageToTexture("res/npc/7/professor-flitwick/textures/Flitwik_dress.png");
    unsigned int profHair = loadImageToTexture("res/npc/7/professor-flitwick/textures/Flitwik_hair.png");
    if (profFace != 0) professorModel->SetTexture(profFace, "texture_diffuse", "Face");
    if (profDress != 0) professorModel->SetTexture(profDress, "texture_diffuse", "Dress");
    if (profHair != 0) professorModel->SetTexture(profHair, "texture_diffuse", "Hair");
    
    deskModel = new Model("res/decoration/5/working_desk.glb", true);

    vec3 professorPos = businessManPos; // same position as business man
    planetDecorations[6].clear();
    planetDecorations[6].push_back({ professorPos, 0.0006f, professorModel });

    // desk in front of him
    vec3 profUp = professorPos;
    vec3 profRef = (abs(profUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 profRight = normalize(cross(profUp, profRef));
    vec3 profFront = normalize(cross(profRight, profUp));
    mat4 profRot = rotate(mat4(1.0f), radians(130.0f), profUp);
    vec3 profFrontNew = normalize(vec3(profRot * vec4(profFront, 0.0f)));
    
    vec3 profDeskDir = normalize(professorPos + profFrontNew * 0.12f);
    planetDecorations[6].push_back({ profDeskDir, 0.3f, deskModel });

    // planet 8 decorations (aviator)
    aviatorModel = new Model("res/npc/8/aviator/source/aviator.fbx");
    aviatorModel->directory = "res/npc/8/aviator/textures";
    unsigned int aviatorHHL = loadImageToTexture("res/npc/8/aviator/textures/hhl_01_co.png");
    unsigned int aviatorMouth = loadImageToTexture("res/npc/8/aviator/textures/mouth_co.png");
    unsigned int aviatorPilot = loadImageToTexture("res/npc/8/aviator/textures/sov_pilot_0_co.png");
    unsigned int aviatorEqip = loadImageToTexture("res/npc/8/aviator/textures/sov_pilot_eqip_0_co.png");
    
    if (aviatorHHL != 0) aviatorModel->SetTexture(aviatorHHL, "texture_diffuse");
    if (aviatorHHL != 0) aviatorModel->SetTexture(aviatorHHL, "texture_diffuse", "hhl_01");
    if (aviatorMouth != 0) aviatorModel->SetTexture(aviatorMouth, "texture_diffuse", "mouth");
    if (aviatorPilot != 0) aviatorModel->SetTexture(aviatorPilot, "texture_diffuse", "sov_pilot_0");
    if (aviatorEqip != 0) aviatorModel->SetTexture(aviatorEqip, "texture_diffuse", "sov_pilot_eqip_0");
    
    // Additional mapping based on Assimp logs (meshes starting with lib_sov_pilot)
    if (aviatorPilot != 0) {
        aviatorModel->SetTexture(aviatorPilot, "texture_diffuse", "lib_sov_pilot.003");
        aviatorModel->SetTexture(aviatorPilot, "texture_diffuse", "lib_sov_pilot.018");
    }
    if (aviatorEqip != 0) {
        aviatorModel->SetTexture(aviatorEqip, "texture_diffuse", "lib_sov_pilot.016");
        aviatorModel->SetTexture(aviatorEqip, "texture_diffuse", "lib_sov_pilot.002");
    }
    if (aviatorHHL != 0) {
        aviatorModel->SetTexture(aviatorHHL, "texture_diffuse", "lib_sov_pilot.017");
    }
    
    aircraftModel = new Model("res/decoration/8/aircraft.glb", true);
    
    vec3 aviatorPos = businessManPos; // same position as business man
    planetDecorations[7].clear();
    planetDecorations[7].push_back({ aviatorPos, 0.00017f, aviatorModel }); // same scale as old man

    // aircraft in front of him
    vec3 aviatorUp = aviatorPos;
    vec3 aviatorRef = (abs(aviatorUp.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 aviatorRight = normalize(cross(aviatorUp, aviatorRef));
    vec3 aviatorFront = normalize(cross(aviatorRight, aviatorUp));
    mat4 aviatorRot = rotate(mat4(1.0f), radians(130.0f), aviatorUp);
    vec3 aviatorFrontNew = normalize(vec3(aviatorRot * vec4(aviatorFront, 0.0f)));
    vec3 aviatorRightNew = normalize(vec3(aviatorRot * vec4(aviatorRight, 0.0f)));
    
    vec3 aircraftDir = normalize(aviatorPos + aviatorFrontNew * 0.15f - aviatorRightNew * 0.09f);
    planetDecorations[7].push_back({ aircraftDir, 0.015f, aircraftModel });

    // add wild plants to planet 8
    for (int i = 0; i < 30; i++) {
        float theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);
        float phi = acosf(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        vec3 dir = normalize(vec3(sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta)));
        planetDecorations[7].push_back({ dir, 0.12f + 0.05f * ((float)rand() / RAND_MAX), wildPlantModel });
    }

    // shaders
    Shader litShader("shader/lit.vert", "shader/lit.frag");
    Shader hudShader("shader/hud.vert", "shader/hud.frag");
    Shader starShader("shader/stars.vert", "shader/stars.frag");
    Shader skyShader("shader/hud.vert", "shader/sky.frag");
    Shader cloudShader("shader/hud.vert", "shader/cloud.frag");


    // load prince models and textures
    princeBaseColor = loadImageToTexture("res/prince/textures/Amigo_low_Default_BaseColor.png");
    princeNormal = loadImageToTexture("res/prince/textures/Amigo_low_Default_Normal.png");
    princeMetallic = loadImageToTexture("res/prince/textures/Amigo_low_Default_Metallic.png");
    princeRoughness = loadImageToTexture("res/prince/textures/Amigo_low_Default_Roughness.png");
    
    // prince models loading
    princeIdle  = new Model("res/prince/little_prince_idle.fbx");
    princeWalk  = new Model("res/prince/little_prince_walking.fbx");
    princeJump  = new Model("res/prince/little_prince_jump.fbx");
    princeGreet = new Model("res/prince/little_prince_greet.fbx");
    princeBow   = new Model("res/prince/little_prince_bow.fbx");
    
    // apply prince textures
    auto applyPrinceTextures = [](Model* m) {
        if (!m) return;
        if (princeBaseColor != 0) m->SetTexture(princeBaseColor, "texture_diffuse");
        if (princeNormal != 0)    m->SetTexture(princeNormal, "texture_normal");
        if (princeMetallic != 0)  m->SetTexture(princeMetallic, "texture_specular");
        if (princeRoughness != 0) m->SetTexture(princeRoughness, "texture_roughness");
    };

    applyPrinceTextures(princeIdle);
    applyPrinceTextures(princeWalk);
    applyPrinceTextures(princeJump);
    applyPrinceTextures(princeGreet);
    applyPrinceTextures(princeBow);

    // load model for fox
    foxIdle = new Model("res/fox/fox_idle.glb", true);
    foxRun  = new Model("res/fox/fox_run.glb", true);
    foxJump = new Model("res/fox/fox_jump.glb", true);
    foxSit  = new Model("res/fox/fox_sit.glb", true);

    // load model for prince - sitting
    princeSitDown = new Model("res/prince/sitting/SIT_DOWN.fbx");
    princeSitting = new Model("res/prince/sitting/SITTING.fbx");
    princeSitUp   = new Model("res/prince/sitting/SIT_UP.fbx");

    applyPrinceTextures(princeSitDown);
    applyPrinceTextures(princeSitting);
    applyPrinceTextures(princeSitUp);

    Camera princeCam, foxCam;
    fox.speed = 6.0f; // the fox is faster than little prince!
    
    // initial positions - moved from poles to avoid glitches
    prince.direction = normalize(vec3(-0.04f, 1, 0.2f));
    fox.direction = normalize(vec3(0.18f, 1, 0.2f)); // fox to the right of prince
    
    // model scale and orientation fixes (some look towards +z)
    prince.modelScale = 0.003f; 
    prince.orientation = translate(mat4(1.0f), vec3(0, -0.1f, 0));
    
    fox.modelScale = 0.5f; 
    fox.orientation = translate(mat4(1.0f), vec3(0, -0.1f, 0));

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        DELTA_TIME = currentFrame - LAST_FRAME;
        LAST_FRAME = currentFrame;

        // --- input (from keyboard) ---
        prince.update(DELTA_TIME);
        fox.update(DELTA_TIME);

        // overlap check
        bool overlap = distance(prince.direction, fox.direction) * PLANET_RADIUS < 1.0f;
        
        bool npcOverlap = false;
        bool inExtendedNpcExclusion = false;
        bool currentInKingOverlap = false;
        bool nearTable = false;
        if (CURRENT_PLANET_IDX > 0) {
            for (auto &p : planetDecorations[CURRENT_PLANET_IDX]) {
                // skip small plants overlap
                if (p.model == wildPlantModel || p.model == roseModel) continue;

                float collisionRadius = p.scale * 10.0f;
                if (p.model == kingModel) collisionRadius = 2.0f;
                if (p.model == arrogantManModel) collisionRadius = 2.0f;
                if (p.model == drunkardModel) collisionRadius = 2.0f;
                if (p.model == businessManModel) collisionRadius = 2.0f;
                if (p.model == oldManModel) collisionRadius = 2.0f;
                if (p.model == workingDeskModel) collisionRadius = 1.2f;
                if (p.model == lampModel) collisionRadius = 0.5f;
                if (p.model == throneModel) collisionRadius = 1.0f;
                if (p.model == tableModel) collisionRadius = 1.0f;
                if (p.model == mirrorModel) collisionRadius = 1.0f;

                float overlapRadius = 2.5f; // Slightly larger than collision
                if (p.model == kingModel) overlapRadius = 2.5f;
                if (p.model == arrogantManModel) overlapRadius = 2.5f;
                if (p.model == drunkardModel) overlapRadius = 2.5f;
                if (p.model == businessManModel) overlapRadius = 2.5f;
                if (p.model == oldManModel) overlapRadius = 2.5f;
                if (p.model == professorModel) overlapRadius = 2.5f;
                if (p.model == aviatorModel) overlapRadius = 2.5f;
                if (p.model == workingDeskModel) overlapRadius = 1.8f;
                if (p.model == deskModel) overlapRadius = 1.8f;
                if (p.model == aircraftModel) overlapRadius = 3.0f;
                if (p.model == lampModel) overlapRadius = 1.0f;
                if (p.model == throneModel) overlapRadius = 1.5f;
                if (p.model == tableModel) overlapRadius = 1.5f;
                if (p.model == mirrorModel) overlapRadius = 1.5f;

                float dist = distance(prince.direction, p.direction) * PLANET_RADIUS;
                if (dist < overlapRadius) {
                    // props don't trigger quotes
                    if (p.model != throneModel && p.model != tableModel && p.model != mirrorModel && p.model != workingDeskModel && p.model != lampModel && p.model != deskModel && p.model != aircraftModel) {
                        npcOverlap = true;
                    }
                    if (p.model == tableModel) {
                        nearTable = true;
                    }
                    
                    // bow when meeting the king
                    if (p.model == kingModel) {
                        currentInKingOverlap = true;
                        if (!prince.hasBowedInThisOverlap) {
                            if (prince.state != CharacterState::BOWING && 
                                prince.state != CharacterState::JUMPING &&
                                prince.state != CharacterState::SITTING_DOWN &&
                                prince.state != CharacterState::SITTING &&
                                prince.state != CharacterState::SITTING_UP) {
                                
                                prince.state = CharacterState::BOWING;
                                prince.animTimer = 3.33f; 
                                prince.animationStartTime = (float)glfwGetTime();
                                prince.hasBowedInThisOverlap = true;
                            }
                        }
                    }
                }
                if (dist < collisionRadius * 2.0f) {
                    inExtendedNpcExclusion = true;
                }
            }
        }
        
        if (!currentInKingOverlap) {
            prince.hasBowedInThisOverlap = false;
        }

        if (((overlap && !inExtendedNpcExclusion) || npcOverlap) && !nearTable) {
            quoteAlpha += DELTA_TIME; // fade in quote
            if (quoteAlpha > 1.0f) quoteAlpha = 1.0f;
        } else {
            quoteAlpha -= DELTA_TIME; // fade out quote
            if (quoteAlpha < 0.0f) quoteAlpha = 0.0f;
        }

        // sitting logic for Fox/Prince overlap
        if (overlap && !inExtendedNpcExclusion) {
            if (!prince.wasOverlapping) {
                if (prince.state != CharacterState::SITTING_DOWN && 
                    prince.state != CharacterState::SITTING && 
                    prince.state != CharacterState::SITTING_UP &&
                    prince.state != CharacterState::JUMPING) {
                    
                    prince.state = CharacterState::SITTING_DOWN;
                    prince.animTimer = 3.03f;
                    prince.animationStartTime = (float)glfwGetTime();
                    fox.animationStartTime = (float)glfwGetTime(); // Reset fox animation too
                }
            }
            prince.wasOverlapping = true;
        } else {
            prince.wasOverlapping = false;
        }

        // reset positions (delete)
        if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            prince.direction = normalize(vec3(-0.04f, 1, 0.2f));
            fox.direction = normalize(vec3(0.18f, 1, 0.2f));
            prince.state = CharacterState::IDLE;
            fox.state = CharacterState::IDLE;
        }

        // prince jump (space)
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (prince.state == CharacterState::SITTING) {
                prince.state = CharacterState::SITTING_UP;
                prince.animTimer = 2.0f;
                prince.animationStartTime = (float)glfwGetTime();
            } else if (prince.state != CharacterState::JUMPING && 
                       prince.state != CharacterState::SITTING_DOWN && 
                       prince.state != CharacterState::SITTING_UP) {
                
                // If already "resetting" (IDLE with timer), don't restart it
                // but we need to know we WANT to jump after 0.1s.
                // Simple approach: if moving, set state to IDLE for 0.1s.
                // In update(), if it was IDLE and timer hits 0, it stays IDLE.
                // So we can use a small trick: if we press SPACE while moving, 
                // we set to IDLE for 0.1s, and we'll check if we should jump then.
                if (prince.state == CharacterState::MOVING) {
                    prince.state = CharacterState::IDLE;
                    prince.animTimer = 0.1f;
                    prince.animationStartTime = (float)glfwGetTime();
                } else {
                    prince.state = CharacterState::JUMPING;
                    prince.animTimer = 2.0f;
                    prince.animationStartTime = (float)glfwGetTime();
                }
            }
        }

        // fox jump (ENTER)
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && fox.state != CharacterState::JUMPING) {
            if (fox.state == CharacterState::MOVING) {
                fox.state = CharacterState::IDLE;
                fox.animTimer = 0.1f;
                fox.animationStartTime = (float)glfwGetTime();
            } else {
                fox.state = CharacterState::JUMPING;
                fox.animTimer = 2.0f;
                fox.animationStartTime = (float)glfwGetTime();
            }
        }

        // prince controls (W-A-S-D)
        float princeForward = 0, princeSide = 0;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) princeForward += 1;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) princeForward -= 1;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) princeSide -= 1;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) princeSide += 1;
        prince.move(princeForward, princeSide, PLANET_RADIUS, DELTA_TIME, princeCam, false, vec3(0), CURRENT_PLANET_IDX);

        // fox controls (UP-DOWN-LEFT-RIGHT arrow keys)
        float foxForward = 0, foxSide = 0;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) foxForward += 1;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) foxForward -= 1;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) foxSide -= 1;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) foxSide += 1;
        fox.move(foxForward, foxSide, PLANET_RADIUS, DELTA_TIME, foxCam, overlap, prince.direction, CURRENT_PLANET_IDX);

        // planet rotation (J/L)
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) PLANET_ROTATION -= 45.0f * DELTA_TIME;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) PLANET_ROTATION += 45.0f * DELTA_TIME;

        // camera orbit
        // Q/E for prince
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) prince.orbitAngle -= 2.0f * DELTA_TIME;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) prince.orbitAngle += 2.0f * DELTA_TIME;
        // [ and ] for fox 
        if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) fox.orbitAngle -= 2.0f * DELTA_TIME;
        if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) fox.orbitAngle += 2.0f * DELTA_TIME;

        // day/night cycle
        // night for 4s, cycle is 20s
        DAY_NIGHT_CYCLE += DELTA_TIME * 0.05f; 
        if (DAY_NIGHT_CYCLE > 1.0f) DAY_NIGHT_CYCLE -= 1.0f;
        
        // sky colors cycle
        const vec3 cNight = vec3(0.0f, 0.0f, 0.0f);      // Pure black sky
        const vec3 cMorning = vec3(0.8f, 0.4f, 0.6f);    // Soft pink
        const vec3 cDay = vec3(0.35f, 0.75f, 1.0f);       // Brighter, more vibrant blue
        const vec3 cEvening = vec3(0.5f, 0.2f, 0.05f);   // Even more subdued sunset orange
        const vec3 cSunsetPink = vec3(0.5f, 0.15f, 0.35f); // Subdued Pinkish sunset
        const vec3 cTwilightBlue = vec3(0.02f, 0.02f, 0.15f); // Deep blueish transition
        
        const vec3 cSkyBlue = vec3(0.35f, 0.75f, 1.0f);
        const vec3 cSkyPink = vec3(0.8f, 0.5f, 0.7f);

        vec3 skyColor;
        float intensity = 0.0f; // intensity for sun light
        float starAlpha = 0.0f;
        float cloudAlpha = 0.0f;
        vec3 topSkyColor = cSkyBlue;
        vec3 bottomSkyColor = cSkyPink;
        
        bool forceNight = (CURRENT_PLANET_IDX == 5);
        
        if (DAY_NIGHT_CYCLE < 0.3f || forceNight) { // Night
            skyColor = cNight;
            starAlpha = 1.0f;
            intensity = 0.0f;
            cloudAlpha = 0.0f;
            topSkyColor = vec3(0.01f, 0.01f, 0.05f); // Very dark blue top
            bottomSkyColor = cNight;
        } else if (DAY_NIGHT_CYCLE < 0.5f) { // Night to Morning
            float t = (DAY_NIGHT_CYCLE - 0.3f) / 0.2f;
            skyColor = mix(cNight, cMorning, t);
            starAlpha = 1.0f - t;
            intensity = t * 0.5f;
            cloudAlpha = 0.0f; // No clouds during morning transition
            topSkyColor = mix(vec3(0.01f, 0.01f, 0.05f), cSkyBlue, t);
            bottomSkyColor = mix(cNight, cSkyPink, t);
        } else if (DAY_NIGHT_CYCLE < 0.6f) { // Morning to Day
            float t = (DAY_NIGHT_CYCLE - 0.5f) / 0.1f;
            skyColor = mix(cMorning, cDay, t);
            starAlpha = 0.0f;
            intensity = 0.5f + t * 0.4f; // Fades to 0.9
            cloudAlpha = t; // Fade in clouds as it becomes Day
            topSkyColor = cSkyBlue;
            bottomSkyColor = cSkyPink;
        } else if (DAY_NIGHT_CYCLE < 0.7f) { // Before Noon
            float t = (DAY_NIGHT_CYCLE - 0.6f) / 0.1f;
            skyColor = cDay;
            starAlpha = 0.0f;
            intensity = 0.9f + t * 0.1f; // Peak at 1.0 (Noon)
            cloudAlpha = 1.0f; 
            topSkyColor = cSkyBlue;
            bottomSkyColor = cSkyPink;
        } else if (DAY_NIGHT_CYCLE < 0.8f) { // After Noon
            float t = (DAY_NIGHT_CYCLE - 0.7f) / 0.1f;
            skyColor = cDay;
            starAlpha = 0.0f;
            intensity = 1.0f - t * 0.1f; // Declines to 0.9
            cloudAlpha = 1.0f; 
            topSkyColor = cSkyBlue;
            bottomSkyColor = cSkyPink;
        } else if (DAY_NIGHT_CYCLE < 0.87f) { // Day to Evening (Orange)
            float t = (DAY_NIGHT_CYCLE - 0.8f) / 0.07f;
            skyColor = mix(cDay, cEvening, t);
            starAlpha = 0.0f;
            intensity = mix(0.9f, 0.5f, t);
            cloudAlpha = 1.0f - t * 0.3f; // Gradually fade clouds (1.0 -> 0.7)
            // Gradient Sunset: Top stays pinkish/blueish longer, bottom becomes orange
            topSkyColor = mix(cSkyBlue, cSunsetPink, t); 
            bottomSkyColor = mix(cSkyPink, cEvening, t);
        } else if (DAY_NIGHT_CYCLE < 0.94f) { // Evening to Pinkish
            float t = (DAY_NIGHT_CYCLE - 0.87f) / 0.07f;
            skyColor = mix(cEvening, cSunsetPink, t);
            starAlpha = 0.0f;
            intensity = 0.5f - t * 0.25f;
            cloudAlpha = 0.7f - t * 0.4f; // Gradually fade clouds (0.7 -> 0.3)
            topSkyColor = mix(cSunsetPink, cTwilightBlue, t);
            bottomSkyColor = mix(cEvening, cSunsetPink, t);
        } else { // Pinkish to Blueish/Night
            float t = (DAY_NIGHT_CYCLE - 0.94f) / 0.06f;
            skyColor = mix(cSunsetPink, cTwilightBlue, t);
            if (t > 0.5f) {
                float t2 = (t - 0.5f) / 0.5f;
                skyColor = mix(cTwilightBlue, cNight, t2);
            }
            starAlpha = t;
            intensity = 0.25f * (1.0f - t);
            cloudAlpha = 0.3f * (1.0f - t); // Fade out completely at night
            topSkyColor = mix(cTwilightBlue, cNight, t);
            bottomSkyColor = mix(cSunsetPink, cNight, t);
        }
        
        // moonlight effect: during night, intensity is low but ambient should be bluish
        vec3 ambientColor = vec3(0.4f * intensity + 0.1f);
        vec3 diffuseColor = vec3(0.9f * intensity);
            // blueish twilight before night
            if (starAlpha > 0.3f) {
                float moonFactor = (starAlpha - 0.3f) / 0.7f;
                ambientColor += vec3(0.05f, 0.05f, 0.25f) * moonFactor; 
                diffuseColor += vec3(0.1f, 0.1f, 0.4f) * moonFactor; 
            }

        mat4 planetRotationM = rotate(mat4(1.0f), radians(PLANET_ROTATION), vec3(0, 1, 0));
        vec3 princeWorldPos = vec3(planetRotationM * vec4(prince.direction * PLANET_RADIUS, 1.0));
        vec3 foxWorldPos = vec3(planetRotationM * vec4(fox.direction * PLANET_RADIUS, 1.0));
        
        // camera follows the target
        princeCam.Follow(princeWorldPos, prince.orbitAngle);
        foxCam.Follow(foxWorldPos, fox.orbitAngle);

        // --- rendering/drawing ---

        // background color
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // depth test and face culling
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        if (CULL_MODE == 0) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else if (CULL_MODE == 1) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } else {
            glDisable(GL_CULL_FACE);
        }

        // Dynamic light direction based on DAY_NIGHT_CYCLE
        // Sun rises in the East (+X), peaks at Noon (+Y), sets in West (-X)
        // We'll map the cycle to an angle. 0.0 is Night, 0.7 is Noon.
        float lightAngle = (DAY_NIGHT_CYCLE - 0.7f) * 2.0f * M_PI;
        vec3 lightDirection = normalize(vec3(sin(lightAngle), -cos(lightAngle), -0.3f));

        litShader.use();
        litShader.setFloat("globalAlpha", DEPTH_TEST ? 1.0f : 0.5f);
        litShader.setVec3("dirLight.direction", lightDirection);
        litShader.setVec3("dirLight.ambient", ambientColor); 
        litShader.setVec3("dirLight.diffuse", diffuseColor);
        litShader.setVec3("dirLight.specular", vec3(1.0f * intensity));

        mat4 projection = perspective(radians(45.0f), (float)fbWidth / 2 / (float)fbHeight, 0.1f, 1000.0f);
        auto renderScene = [&](Camera &cam) {
            // procedural sky
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            skyShader.use();
            skyShader.setMat4("projection", mat4(1.0f));
            skyShader.setMat4("view", mat4(1.0f));
            skyShader.setFloat("time", (float)glfwGetTime());
            skyShader.setVec3("topColor", topSkyColor);
            skyShader.setVec3("bottomColor", bottomSkyColor);
            skyShader.setFloat("cloudAlpha", cloudAlpha);
            
            glBindVertexArray(hudVAO);
            mat4 bgModel = translate(mat4(1.0f), vec3(-1.0f, -1.0f, 0.0f));
            bgModel = scale(bgModel, vec3(2.0f, 2.0f, 1.0f));
            skyShader.setMat4("model", bgModel);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            // render stars
            if (starAlpha > 0.01f) {
                starShader.use();
                starShader.setMat4("view", cam.GetViewMatrix());
                starShader.setMat4("projection", projection);
                starShader.setFloat("time", (float)glfwGetTime());
                starShader.setFloat("starAlpha", starAlpha);
                
                glEnable(GL_PROGRAM_POINT_SIZE);
                glBindVertexArray(starVAO);
                glDrawArrays(GL_POINTS, 0, STAR_COUNT);
                glDisable(GL_PROGRAM_POINT_SIZE);
            }

            // render clouds
            if (cloudAlpha > 0.01f) {
                glDisable(GL_CULL_FACE); // disable culling for clouds
                glDepthMask(GL_FALSE);   // disable depth writing for transparent clouds
                cloudShader.use();
                cloudShader.setMat4("projection", projection);
                cloudShader.setMat4("view", cam.GetViewMatrix());
                cloudShader.setFloat("alpha", cloudAlpha);
                cloudShader.setFloat("time", (float)glfwGetTime());

                glBindVertexArray(cloudVAO);
                for (auto &c : clouds) {
                    mat4 model = mat4(1.0f);
                    model = translate(model, c.position);
                    
                    // billboard (facing the camera)
                    mat4 viewM = cam.GetViewMatrix();
                    model[0][0] = viewM[0][0]; model[0][1] = viewM[1][0]; model[0][2] = viewM[2][0];
                    model[1][0] = viewM[0][1]; model[1][1] = viewM[1][1]; model[1][2] = viewM[2][1];
                    model[2][0] = viewM[0][2]; model[2][1] = viewM[1][2]; model[2][2] = viewM[2][2];
                    
                    model = rotate(model, radians(c.rotation), vec3(0, 0, 1));
                    model = scale(model, vec3(c.size, 1.0f));
                    
                    cloudShader.setMat4("model", model);
                    cloudShader.setInt("cloudType", c.type);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
                glDepthMask(GL_TRUE); // restore depth writing
                if (CULL_MODE != 2) glEnable(GL_CULL_FACE); // restore culling
            }

            litShader.use();
            litShader.setMat4("view", cam.GetViewMatrix());
            litShader.setVec3("viewPos", cam.Position);

            // planet rendering
            litShader.setBool("isAnimated", false);
            mat4 planetBaseM = rotate(mat4(1.0f), radians(PLANET_ROTATION), vec3(0, 1, 0));
            
            // ccw winding for all objects
            glFrontFace(GL_CCW); 

            if (planetModel) {
                litShader.setMat4("model", scale(planetBaseM, vec3(PLANET_RADIUS)));
                planetModel->Draw(litShader);
            } else if (sphereVAO != 0) {
                // planet placeholder (sphere with bumps)
                litShader.setMat4("model", planetBaseM);
                litShader.setInt("texture_diffuse1", 0);
                glActiveTexture(GL_TEXTURE0);
                
                // texture from selection 1-8
                if (planetTextures[CURRENT_PLANET_IDX] != 0) 
                    glBindTexture(GL_TEXTURE_2D, planetTextures[CURRENT_PLANET_IDX]);
                else 
                    glBindTexture(GL_TEXTURE_2D, venusTexture); // ultra fallback
                
                glBindVertexArray(sphereVAO);
                glDrawElements(GL_TRIANGLES, sphereIndicesCount, GL_UNSIGNED_INT, 0);
            }

            // render decorations
            for (auto &p : planetDecorations[CURRENT_PLANET_IDX]) {
                if (p.model || CURRENT_PLANET_IDX >= 3) {
                    mat4 pModel = planetBaseM;
                    float stackAngle = asinf(p.direction.z);
                    float sectorAngle = atan2f(p.direction.y, p.direction.x);
                    if (sectorAngle < 0) sectorAngle += 2.0f * M_PI;
                    float bump = getPlanetBump(sectorAngle, stackAngle, CURRENT_PLANET_IDX);
                    float actualRadius = PLANET_RADIUS * (1.0f + bump);
                    
                    // align with surface normal
                    vec3 up = p.direction;
                    vec3 ref = (abs(up.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
                    vec3 right = normalize(cross(up, ref));
                    vec3 forward = cross(right, up);
                    
                    mat4 align = mat4(1.0f);
                    align[0] = vec4(right, 0.0f);
                    align[1] = vec4(up, 0.0f);
                    align[2] = vec4(forward, 0.0f);

                    float verticalOffset = 0.1f;
                    
                    if (CURRENT_PLANET_IDX == 0) {
                        align = rotate(align, radians(-90.0f), vec3(1, 0, 0));
                        if (p.model == baobabModel || p.model == wildPlantModel) verticalOffset = -0.02f;
                        if (p.model == roseModel) {
                            align = mat4(1.0f);
                            align[0] = vec4(right, 0.0f);
                            align[1] = vec4(up, 0.0f);
                            align[2] = vec4(forward, 0.0f);
                            align = rotate(align, radians(270.0f), vec3(1, 0, 0));
                            verticalOffset = -0.01f;
                            glDisable(GL_CULL_FACE);
                        }
                    } else if (CURRENT_PLANET_IDX == 1) {
                        verticalOffset = -0.1f;
                        if (p.model == kingModel) {
                            align = rotate(align, radians(270.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(270.0f), vec3(1, 0, 0));
                            verticalOffset = 1.5f;
                            glDisable(GL_CULL_FACE); 
                        }
                        if (p.model == throneModel) {
                            align = rotate(align, radians(0.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(360.0f), vec3(1, 0, 0));
                            verticalOffset = -0.5f;
                            glDisable(GL_CULL_FACE); 
                        }
                    } else if (CURRENT_PLANET_IDX == 2) {
                        verticalOffset = -0.1f;
                        if (p.model == arrogantManModel) {
                            align = rotate(align, radians(0.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                            verticalOffset = -0.2f;
                            glDisable(GL_CULL_FACE); 
                        }
                    } else if (CURRENT_PLANET_IDX == 3) {
                        verticalOffset = -0.1f;
                        if (p.model == drunkardModel) {
                            // Match Arrogant Man's rotation
                            align = rotate(align, radians(0.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                            verticalOffset = -0.4f;
                            glEnable(GL_CULL_FACE);
                        }
                        if (p.model == tableModel) {
                            // Match Drunkard's rotation (Arrogant Man's base)
                            align = rotate(align, radians(0.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                            verticalOffset = 0.05f;
                            glDisable(GL_CULL_FACE);
                        }
                    } else if (CURRENT_PLANET_IDX == 4) {
                        verticalOffset = -0.1f;
                        if (p.model == businessManModel) {
                            align = rotate(align, radians(130.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                        }
                        if (p.model == workingDeskModel) {
                            align = rotate(align, radians(230.0f), vec3(0, 1, 0));
                            verticalOffset = -0.35f;
                        }
                    } else if (CURRENT_PLANET_IDX == 5) {
                        verticalOffset = 0.0f;
                        if (p.model == oldManModel) {
                            align = rotate(align, radians(130.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                            verticalOffset = -0.05f;
                        }
                        if (p.model == lampModel) {
                            align = rotate(align, radians(130.0f), vec3(0, 1, 0));
                            verticalOffset = 2.3f;
                        }
                    } else if (CURRENT_PLANET_IDX == 6) {
                        verticalOffset = -0.1f;
                        if (p.model == professorModel) {
                            align = rotate(align, radians(130.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                        }
                        if (p.model == deskModel) {
                            align = rotate(align, radians(230.0f), vec3(0, 1, 0));
                            verticalOffset = -0.35f;
                        }
                    } else if (CURRENT_PLANET_IDX == 7) {
                        verticalOffset = 0.0f;
                        if (p.model == aviatorModel) {
                            align = rotate(align, radians(130.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(0.0f), vec3(1, 0, 0));
                            verticalOffset = -0.05f; // same as old man
                        }
                        if (p.model == aircraftModel) {
                            align = rotate(align, radians(130.0f), vec3(0, 1, 0));
                            align = rotate(align, radians(-90.0f), vec3(1, 0, 0));
                            verticalOffset = 0.0f;
                        }
                        if (p.model == wildPlantModel) {
                            align = rotate(align, radians(-90.0f), vec3(1, 0, 0));
                            verticalOffset = -0.02f;
                        }
                    } else {
                        verticalOffset = 0.5f;
                    }

                    pModel = translate(pModel, p.direction * (actualRadius + verticalOffset));
                    pModel = pModel * align;
                    pModel = scale(pModel, vec3(p.scale));
                    
                    bool lampOn = true;
                    if (CURRENT_PLANET_IDX == 5) {
                        float t = (float)glfwGetTime();
                        // flickering: 2s on, 2s off
                        if (fmod(t, 4.0f) > 2.0f) {
                            lampOn = false;
                        }
                    }

                    if (lampOn && (p.model == lampModel || p.model == oldManModel || p.model == princeIdle || p.model == princeWalk || p.model == princeJump || p.model == princeGreet || p.model == princeBow || p.model == princeSitDown || p.model == princeSitting || p.model == princeSitUp || p.model == foxIdle || p.model == foxRun || p.model == foxJump || p.model == foxSit)) {
                        litShader.setBool("usePointLight", true);
                        
                        // we need the lamp's world position for the point light even when rendering the old man
                        // lamp point light setup
                        mat4 lModel = planetBaseM;
                        lModel = translate(lModel, lampDir * (actualRadius + 2.3f));
                        lModel = lModel * rotate(mat4(1.0f), radians(130.0f), vec3(0, 1, 0));
                        lModel = scale(lModel, vec3(3.0f));
                        
                        vec3 lampTopWorld = vec3(lModel * vec4(0.0f, 0.45f, 0.0f, 1.0f));
                        
                        litShader.setVec3("pointLight.position", lampTopWorld);
                        litShader.setFloat("pointLight.constant", 1.0f);
                        litShader.setFloat("pointLight.linear", 0.09f);
                        litShader.setFloat("pointLight.quadratic", 0.032f);
                        litShader.setVec3("pointLight.ambient", vec3(0.2f, 0.2f, 0.1f));
                        litShader.setVec3("pointLight.diffuse", vec3(1.0f, 1.0f, 0.8f));
                        litShader.setVec3("pointLight.specular", vec3(1.0f, 1.0f, 1.0f));
                    } else {
                        litShader.setBool("usePointLight", false);
                    }

                    litShader.setMat4("model", pModel);
                    
                    if (p.model) {
                        if (p.model == arrogantManModel || p.model == drunkardModel || p.model == businessManModel || p.model == oldManModel || p.model == professorModel || p.model == aviatorModel) {
                            litShader.setBool("isAnimated", true);
                            vector<mat4> nBones;
                            float animTime = (float)glfwGetTime();
                            p.model->GetAnimationTransforms(animTime, nBones, true);
                            litShader.setMat4Array("finalBonesMatrices", nBones);
        
                            // bind PBR samplers to valid textures
                            litShader.setInt("texture_diffuse1", 0);
                            litShader.setInt("texture_specular1", 1);
                            litShader.setInt("texture_normal1", 2);
                            litShader.setInt("texture_roughness1", 3);
                        } else {
                            litShader.setBool("isAnimated", false);
                        }
                        
                        if (p.model == wildPlantModel) glDisable(GL_CULL_FACE);
                        
                        p.model->Draw(litShader);
                        
                        if (p.model == wildPlantModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == roseModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == kingModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == throneModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == arrogantManModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == drunkardModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == businessManModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == oldManModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == professorModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == aviatorModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == tableModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == workingDeskModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == deskModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == aircraftModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);
                        if (p.model == lampModel && CULL_MODE != 2) glEnable(GL_CULL_FACE);

                        // draw light at lamp top
                        if (p.model == lampModel && lampOn) {
                            mat4 lightModel = translate(pModel, vec3(0.0f, 0.45f, 0.0f));
                            lightModel = scale(lightModel, vec3(0.02f));
                            litShader.setMat4("model", lightModel);
                            litShader.setBool("isAnimated", false);
                            litShader.setBool("usePointLight", false); // light source itself doesn't need point light
                            // emission effect
                            drawPlaceholderCube();
                        }
                    } else {
                        // placeholder for npc
                        litShader.setBool("isAnimated", false);
                        litShader.setInt("texture_diffuse1", 0);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, studentInfoTex); // generic texture
                        drawPlaceholderCube();
                    }
                }
            }

            // prince rendering
            mat4 princeM = prince.getTransform(PLANET_RADIUS, PLANET_ROTATION);
            litShader.setMat4("model", princeM);
            
            Model* pCurrent = princeIdle;
            if (prince.state == CharacterState::MOVING) pCurrent = princeWalk;
            else if (prince.state == CharacterState::JUMPING) pCurrent = princeJump;
            else if (prince.state == CharacterState::GREETING) pCurrent = princeGreet;
            else if (prince.state == CharacterState::SITTING_DOWN) pCurrent = princeSitDown;
            else if (prince.state == CharacterState::SITTING) pCurrent = princeSitting;
            else if (prince.state == CharacterState::SITTING_UP) pCurrent = princeSitUp;
            else if (prince.state == CharacterState::BOWING) pCurrent = princeBow;

            if (pCurrent) {
                // point light handling for Prince on Planet 6
                bool lampOn = true;
                if (CURRENT_PLANET_IDX == 5) {
                    if (fmod((float)glfwGetTime(), 4.0f) > 2.0f) lampOn = false;
                }
                
                if (CURRENT_PLANET_IDX == 5 && lampOn) {
                    litShader.setBool("usePointLight", true);
                    mat4 lModel = planetBaseM;
                    lModel = translate(lModel, lampDir * (PLANET_RADIUS * (1.0f + getPlanetBump(atan2f(lampDir.y, lampDir.x), asinf(lampDir.z), 5)) + 2.3f));
                    lModel = lModel * rotate(mat4(1.0f), radians(130.0f), vec3(0, 1, 0));
                    lModel = scale(lModel, vec3(3.0f));
                    vec3 lampTopWorld = vec3(lModel * vec4(0.0f, 0.45f, 0.0f, 1.0f));
                    
                    litShader.setVec3("pointLight.position", lampTopWorld);
                    litShader.setFloat("pointLight.constant", 1.0f);
                    litShader.setFloat("pointLight.linear", 0.09f);
                    litShader.setFloat("pointLight.quadratic", 0.032f);
                    litShader.setVec3("pointLight.ambient", vec3(0.2f, 0.2f, 0.1f));
                    litShader.setVec3("pointLight.diffuse", vec3(1.0f, 1.0f, 0.8f));
                    litShader.setVec3("pointLight.specular", vec3(1.0f, 1.0f, 1.0f));
                } else {
                    litShader.setBool("usePointLight", false);
                }

                // skeletal animation
                litShader.setBool("isAnimated", true);
                vector<mat4> pBones;
                float animTime = (float)glfwGetTime() - prince.animationStartTime;
                
                bool loop = true;
                if (prince.state == CharacterState::JUMPING || 
                    prince.state == CharacterState::GREETING || 
                    prince.state == CharacterState::SITTING_DOWN || 
                    prince.state == CharacterState::SITTING_UP ||
                    prince.state == CharacterState::BOWING) {
                    loop = false;
                }
                
                pCurrent->GetAnimationTransforms(animTime, pBones, loop);
                litShader.setMat4Array("finalBonesMatrices", pBones);
                pCurrent->Draw(litShader);
            } else {
                litShader.setBool("isAnimated", false);
                // placeholder cube if no model
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, studentInfoTex); 
                drawPlaceholderCube();
            }

            // fox rendering
            mat4 foxM = fox.getTransform(PLANET_RADIUS, PLANET_ROTATION);
            litShader.setMat4("model", foxM);

            Model* fCurrent = foxIdle;
            if (overlap) fCurrent = foxSit;
            else if (fox.state == CharacterState::MOVING) fCurrent = foxRun;
            else if (fox.state == CharacterState::JUMPING) fCurrent = foxJump;

            if (fCurrent) {
                // point light handling for fox on Planet 6
                bool lampOn = true;
                if (CURRENT_PLANET_IDX == 5) {
                    if (fmod((float)glfwGetTime(), 4.0f) > 2.0f) lampOn = false;
                }
                
                if (CURRENT_PLANET_IDX == 5 && lampOn) {
                    litShader.setBool("usePointLight", true);
                    mat4 lModel = planetBaseM;
                    lModel = translate(lModel, lampDir * (PLANET_RADIUS * (1.0f + getPlanetBump(atan2f(lampDir.y, lampDir.x), asinf(lampDir.z), 5)) + 2.3f));
                    lModel = lModel * rotate(mat4(1.0f), radians(130.0f), vec3(0, 1, 0));
                    lModel = scale(lModel, vec3(3.0f));
                    vec3 lampTopWorld = vec3(lModel * vec4(0.0f, 0.45f, 0.0f, 1.0f));
                    
                    litShader.setVec3("pointLight.position", lampTopWorld);
                    litShader.setFloat("pointLight.constant", 1.0f);
                    litShader.setFloat("pointLight.linear", 0.09f);
                    litShader.setFloat("pointLight.quadratic", 0.032f);
                    litShader.setVec3("pointLight.ambient", vec3(0.2f, 0.2f, 0.1f));
                    litShader.setVec3("pointLight.diffuse", vec3(1.0f, 1.0f, 0.8f));
                    litShader.setVec3("pointLight.specular", vec3(1.0f, 1.0f, 1.0f));
                } else {
                    litShader.setBool("usePointLight", false);
                }

                // skeletal animation
                litShader.setBool("isAnimated", true);
                vector<mat4> fBones;
                float animTime = (float)glfwGetTime() - fox.animationStartTime;
                
                bool loop = true;
                if (fox.state == CharacterState::JUMPING) {
                    loop = false;
                }
                if (overlap) {
                    loop = true;
                }
                
                fCurrent->GetAnimationTransforms(animTime, fBones, loop);
                litShader.setMat4Array("finalBonesMatrices", fBones);
                fCurrent->Draw(litShader);
            } else {
                litShader.setBool("isAnimated", false);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, foxQuoteTexture);
                drawPlaceholderCube();
            }
        };

        // split-screen: left is fox
        glViewport(0, 0, fbWidth / 2, fbHeight);
        litShader.setMat4("projection", projection);
        renderScene(foxCam);

        // split-screen: right is prince
        glViewport(fbWidth / 2, 0, fbWidth / 2, fbHeight);
        renderScene(princeCam);

        // hud and 2d overlay (fullscreen)
        glViewport(0, 0, fbWidth, fbHeight);
        
        // hud blending, disable depth write and culling
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        
        hudShader.use();
        mat4 ortho = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
        hudShader.setMat4("projection", ortho);
        hudShader.setMat4("view", mat4(1.0f)); // Reset view for 2D HUD
        
        // student-info (top left corner)
        drawHUDQuad(hudShader, studentInfoTex, vec2(10, SCREEN_HEIGHT - 467 - 10), vec2(420, 566), 0.7f);
        
        // render quote with fading (top right corner)
        if (quoteAlpha > 0.01f) {
            unsigned int tex = 0;
            float qWidth = 0, qHeight = 0;

            if (overlap && !inExtendedNpcExclusion) {
                tex = foxQuoteTexture;
                qWidth = 310.0f * 1.1f; 
                qHeight = 130.0f * 0.9f;
            } else if (npcOverlap) {
                tex = quoteTextures[CURRENT_PLANET_IDX];
                qWidth = QUOTE_WIDTHS[CURRENT_PLANET_IDX] * QUOTE_SCALE_X[CURRENT_PLANET_IDX];
                qHeight = QUOTE_HEIGHTS[CURRENT_PLANET_IDX] * QUOTE_SCALE_Y[CURRENT_PLANET_IDX];
            }

            if (tex != 0) {
                drawHUDQuad(hudShader, tex, vec2(SCREEN_WIDTH - qWidth - 60, SCREEN_HEIGHT - qHeight - 60), vec2(qWidth - 10, qHeight - 20), quoteAlpha);
            }
        }

        // rose cursor
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        unsigned int currentRose = roseCursorTex;
        // rose cursor color change on click
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            currentRose = roseCursorPressedTex;
        }
        // rose cursor size 64x64, offset for hotspot 3,3
        drawHUDQuad(hudShader, currentRose, vec2((float)mx - 3, (float)SCREEN_HEIGHT - (float)my - 61), vec2(64, 64));

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS limiter (75 FPS)
        float frameTime = (float)glfwGetTime() - currentFrame;
        if (frameTime < TARGET_FRAME_TIME) {
            this_thread::sleep_for(chrono::duration<float>(TARGET_FRAME_TIME - frameTime));
        }
    }

    // cleanup
    if (planetModel) delete planetModel;
    if (princeIdle) delete princeIdle;
    if (princeWalk) delete princeWalk;
    if (princeJump) delete princeJump;
    if (princeGreet) delete princeGreet;
    if (princeBow) delete princeBow;
    if (foxIdle) delete foxIdle;
    if (foxRun) delete foxRun;
    if (foxJump) delete foxJump;
    if (foxSit) delete foxSit;
    if (princeSitDown) delete princeSitDown;
    if (princeSitting) delete princeSitting;
    if (princeSitUp) delete princeSitUp;
    for(int i=0; i<8; i++) if (npcModels[i]) delete npcModels[i];
    
    if (baobabModel) delete baobabModel;
    if (roseModel) delete roseModel;
    if (wildPlantModel) delete wildPlantModel;
    if (kingModel) delete kingModel;
    if (throneModel) delete throneModel;
    if (arrogantManModel) delete arrogantManModel;
    if (drunkardModel) delete drunkardModel;
    if (tableModel) delete tableModel;
    if (workingDeskModel) delete workingDeskModel;
    if (oldManModel) delete oldManModel;
    if (professorModel) delete professorModel;
    if (deskModel) delete deskModel;
    if (lampModel) delete lampModel;
    if (mirrorModel) delete mirrorModel;
    if (aviatorModel) delete aviatorModel;
    if (aircraftModel) delete aircraftModel;

    glfwTerminate();
    return 0;
}
