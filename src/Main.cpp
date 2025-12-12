#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <iostream>
#include <string>
#include "Util.h"

using namespace std;


struct vec2 {
    float x, y;
};

// -------- Global constants --------

int screenWidth = 800;
int screenHeight = 800;
float aspectRatio = 1.0f;

GLFWcursor* cursor;
GLFWcursor* cursorPressed;

// planets
int currentPlanet = 0; // 0 to 7
float planetRotation = 0.0f;
constexpr float PLANET_ROTATION_SPEED = -0.003f; // clock rotation

// PI / 2 = on top
float princeAngle = M_PI / 2.0f;
float foxAngle = M_PI / 2.0f + 0.5f;

enum Direction { DIR_RIGHT, DIR_LEFT, DIR_NONE };
Direction princeMoveDir = DIR_NONE;
Direction foxMoveDir = DIR_NONE;

// characters speed
constexpr float SPEED_PRINCE_RIGHT = 0.01f;
constexpr float SPEED_PRINCE_LEFT = 0.008f;
constexpr float SPEED_FOX_RIGHT = SPEED_PRINCE_RIGHT + 0.007f;
constexpr float SPEED_FOX_LEFT = SPEED_PRINCE_LEFT + 0.007f;

// characters orbit radius
constexpr float PRINCE_ORBIT_RADIUS = 0.4f;
constexpr float FOX_ORBIT_RADIUS = 0.32f;

constexpr float NPC_ORBIT_RADIUS = 0.48f;
constexpr float NPC_SCALE_X[8] = {
    0.13f, 0.45f, 0.38f, 0.38f,
    0.57f, 0.6f, 0.32f, 0.52f
};
constexpr float NPC_SCALE_Y[8] = {
    0.13f, 0.45f, 0.38f, 0.38f,
    0.57f, 0.6f, 0.32f, 0.52f
};
constexpr float NPC_Y_OFFSET[8] = {
    -0.14f, -0.12f, -0.09f, -0.05f,
    -0.07f, -0.04f, -0.08f, -0.16f
};

// quote
constexpr float QUOTE_SCALE_X[8] = {
    1.1f, 1.1f, 1.17f, 1.25f,
    1.08f, 0.72f, 1.1f, 1.0f
};
constexpr float QUOTE_SCALE_Y[8] = {
    0.93f, 0.93f, 0.92f, 0.6f,
    0.9f, 0.64f, 1.0f, 1.0f
};

// planet orbit radius
constexpr float PLANET_ORBIT_RADIUS = 0.65f;
constexpr float PLANET_SCALE[8] = {
    1.0f, 1.15f, 1.02f, 1.22f,
    1.02f, 1.0f, 1.02f, 1.05f
};

// stars
constexpr int NUM_STARS = 20;
float starPositions[NUM_STARS * 2] = {
    // top
    -0.8f,  0.5f,
    -0.5f, -0.1f,
    -0.3f,  0.4f,
    -0.1f,  0.7f,
     0.2f, -0.3f,
     0.4f,  0.3f,
     0.6f,  0.0f,
     0.7f,  0.2f,
     0.8f,  0.8f,
     0.9f,  0.1f,

    // bottom
    -0.9f, -0.6f,
    -0.7f, -0.8f,
    -0.4f, -0.5f,
     0.0f, -0.7f,
     0.2f, -0.9f,
     0.4f, -0.6f,
     0.6f, -0.8f,
    -0.2f, -0.3f,
     0.3f, -0.4f,
     0.8f, -0.2f,
};

unsigned int rectShader;
unsigned int colorShader;

unsigned int VAOrect, VBOrect;
unsigned int VAOstar, VBOstar;
unsigned int VAOnight, VBOnight;

// textures
unsigned int planetTextures[8];

unsigned int princeStandRight, princeStandLeft;
unsigned int princeWalkRight[2], princeWalkLeft[2];

unsigned int foxStandRight, foxStandLeft;
unsigned int foxWalkRight[2], foxWalkLeft[2];

unsigned int princeHoldingLeftTexture, princeHoldingRightTexture;

unsigned int npcTextures[8];
unsigned int quoteTextures[8];
unsigned int foxQuoteTexture;
unsigned int skyTexture;
unsigned int studentInfoTexture;

bool flipped = false;

float foxPosX = 0.0f;              // NEW: fox center position in NDC
float foxPosY = 0.0f;
bool showFoxQuote = false;

// -------- Helper functions --------

void squish_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /*if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        // uS se kreće između dva stanja: normalnog i spljeskanog
        if (uS == 1.0f) {
            uS = 0.5f;
            hasHat = false;
        }
        else {
            uS = 1.0f;
        }
    }
    // Pošto je svakako potrebno pamtiti vreme kada je skok započet, možemo je postaviti na negativnu vrednost ako želimo da signaliziramo da četvorougao nije u skoku
    // Proverom da li je kliknut Space ali istovremeno da li skok traje, obezbeđuje se da telo ne može skočiti ukoliko je već u procesu skakanja
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && initialJumpTime < 0) {
        initialJumpTime = glfwGetTime();
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS && uS == 1.0f) {
        hasHat = !hasHat;
    }*/
}

void center_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwSetCursor(window, cursorPressed);

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int w, h;
        glfwGetWindowSize(window, &w, &h);

        // Normalizacija da bi se pozicija izražena u pikselima namapirala na OpenGL-ov prozor sa opsegom (-1, 1)
        float xposNorm = float(xpos) / float(w) * 2.0f - 1.0f;
        float yposNorm = - (float(ypos) / float(h) * 2.0f - 1.0f);

        float dx = xposNorm - foxPosX;
        float dy = yposNorm - foxPosY;
        float dist2 = dx * dx + dy * dy;

        const float FOX_CLICK_RADIUS = 0.15f;
        if (dist2 <= FOX_CLICK_RADIUS * FOX_CLICK_RADIUS) {
            showFoxQuote = !showFoxQuote;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        glfwSetCursor(window, cursor);
    }
}


// -------- Drawing functions --------

void drawDayNightTint(float skyR, float skyG, float skyB, float nightFactor) {
    glUseProgram(colorShader);

    glUniform2f(glGetUniformLocation(colorShader, "uPos"), 0.0f, 0.0f);

    if (nightFactor < 0.0f) nightFactor = 0.0f;
    if (nightFactor > 1.0f) nightFactor = 1.0f;

    float alphaDay = 0.6f;
    float alphaNight = 0.95f;
    float alpha = alphaDay + (alphaNight - alphaDay) * nightFactor;

    glUniform1f(glGetUniformLocation(colorShader, "uA"), alpha);

    glUniform3f(glGetUniformLocation(colorShader, "uTintColor"), skyR, skyG, skyB);
    glUniform1f(glGetUniformLocation(colorShader, "uTintAmount"), 1.0f); // fully use tint

    glBindVertexArray(VAOnight);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawStars(float* positions, float factor) {
    if (factor <= 0.0f) return; // invisible

    glUseProgram(colorShader);
    glUniform1f(glGetUniformLocation(colorShader, "uTintAmount"), 0.0f);

    if (factor > 1.0f) factor = 1.0f;
    glUniform1f(glGetUniformLocation(colorShader, "uA"), factor);

    glBindVertexArray(VAOstar);
    for (int i = 0; i < NUM_STARS; ++i) {
        glUniform2f(
            glGetUniformLocation(colorShader, "uPos"),
            positions[2 * i],
            positions[2 * i + 1]
        );
        glDrawArrays(GL_TRIANGLE_FAN, 0, 12);
    }
}

void drawTexturedRect(unsigned int texture, vec2 pos, vec2 scale, float rotation, bool flipped = false, float alpha = 1.0f) {
    glUseProgram(rectShader);

    glUniform2f(glGetUniformLocation(rectShader, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(rectShader, "uScale"), scale.x, scale.y);
    glUniform1f(glGetUniformLocation(rectShader, "uRotation"), rotation);
    glUniform1i(glGetUniformLocation(rectShader, "flipped"), flipped);
    glUniform1f(glGetUniformLocation(rectShader, "uAlpha"), alpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(rectShader, "uTex0"), 0);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawSky() {
    vec2 pos = { 0.0f, 0.0f };
    vec2 scale = { 2.0f, 2.0f };  // fullscreen, to cover [-1, 1]
    drawTexturedRect(skyTexture, pos, scale, 0.0f, false);
}

void drawStudentInfo() {
    const float texWidth = 350.0f;
    const float texHeight = 430.0f;

    const float marginPx = 10.0f;

    // top-left corer [-1, 1]
    float width = (texWidth / screenWidth)  * 2.0f;  // from -1 to +1
    float height = (texHeight / screenHeight) * 2.0f;

    float marginX = (marginPx / screenWidth)  * 2.0f;
    float marginY = (marginPx / screenHeight) * 2.0f;

    float centerX = (-1.0f + marginX + width * 0.5f);
    float centerY =  (1.0f - marginY - height * 0.5f) * 1.1f;

    vec2 pos = { centerX, centerY };
    vec2 scale = { width, height };

    drawTexturedRect(studentInfoTexture, pos, scale, 0.0f, false, 0.7f);
}

void drawQuote(int planetIndex) {
    const float texWidth = 310.0f;
    const float texHeight = 110.0f;

    const float marginPx = 10.0f;

    float baseWidth = (texWidth / screenWidth)  * 2.0f;
    float baseHeight = (texHeight / screenHeight) * 2.0f;

    float width = baseWidth * QUOTE_SCALE_X[planetIndex];
    float height = baseHeight * QUOTE_SCALE_Y[planetIndex];

    float marginX = (marginPx / screenWidth)  * 2.0f;
    float marginY = (marginPx / screenHeight) * 2.0f;

    // top-right corner [1, 1]
    float centerX = (1.0f - marginX - width * 0.5f) * 0.95f;
    float centerY = (1.0f - marginY - height * 0.5f) * 0.85f;

    vec2 pos = { centerX, centerY };
    vec2 scale = { width, height };

    drawTexturedRect(quoteTextures[planetIndex], pos, scale, 0.0f, false);
}

void drawFoxQuoteBottomRight() {
    const float texWidth  = 310.0f;
    const float texHeight = 110.0f;
    const float marginPx  = 10.0f;

    float width = (texWidth / screenWidth)  * 2.0f;
    float height = (texHeight / screenHeight) * 2.0f;

    float marginX = (marginPx / screenWidth)  * 2.0f;
    float marginY = (marginPx / screenHeight) * 2.0f;

    // bottom-right corner [+1, -1]
    float centerX = (1.0f - marginX - width * 0.5f) * 0.9f;
    float centerY = (-1.0f + marginY + height * 0.5f) * 0.86f;

    vec2 pos = { centerX, centerY };
    vec2 scale = { width * 1.05f, height * 0.7f };

    drawTexturedRect(foxQuoteTexture, pos, scale, 0.0f, false, 0.8f);
}

// -------- Setup --------

/**
 * Initialize GLFW and set version 3 with a programmable pipeline
 */
void initializeGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

/**
 * Form the window with screen dimensions and title
 */
GLFWwindow* setupWindow() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    screenWidth = mode->width;
    screenHeight = mode->height;
    aspectRatio = (float)screenWidth / (float)screenHeight;

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "TheLittlePrince", nullptr, nullptr);
    if (window == nullptr) return nullptr;

    glfwMakeContextCurrent(window);
    return window;
}

/**
 * Link all callback functions to the window after it's initialized
 */
void linkCallbackFunctions(GLFWwindow* window) {
    glfwSetKeyCallback(window, squish_callback);
    glfwSetMouseButtonCallback(window, center_callback);
}

/**
 * Load and link the custom cursor with the initialized window
 */
void linkCursor(GLFWwindow* window) {
    cursor = loadImageToCursor("../res/cursor/rose-1.png");
    cursorPressed = loadImageToCursor("../res/cursor/rose-2.png");
    glfwSetCursor(window, cursor);
}

void loadTextures() {
    // all planets
    string planetPath = "../res/planets/";
    for (int i = 0; i < 8; i++) {
        string path = planetPath + to_string(i + 1) + ".png";
        planetTextures[i] = loadImageToTexture(path.c_str());
    }

    // background sky
    skyTexture = loadImageToTexture("../res/misc/sky.png");

    // prince
    princeStandRight = loadImageToTexture("../res/prince/standing-right.png");
    princeStandLeft = loadImageToTexture("../res/prince/standing-left.png");
    princeWalkRight[0] = loadImageToTexture("../res/prince/walking-right-1.png");
    princeWalkRight[1] = loadImageToTexture("../res/prince/walking-right-2.png");
    princeWalkLeft[0] = loadImageToTexture("../res/prince/walking-left-1.png");
    princeWalkLeft[1] = loadImageToTexture("../res/prince/walking-left-2.png");

    // fox
    foxStandRight = loadImageToTexture("../res/fox/standing-right.png");
    foxStandLeft = loadImageToTexture("../res/fox/standing-left.png");
    foxWalkRight[0] = loadImageToTexture("../res/fox/running-right-1.png");
    foxWalkRight[1] = loadImageToTexture("../res/fox/running-right-2.png");
    foxWalkLeft[0] = loadImageToTexture("../res/fox/running-left-1.png");
    foxWalkLeft[1] = loadImageToTexture("../res/fox/running-left-2.png");

    // prince + fox holding
    princeHoldingLeftTexture  = loadImageToTexture("../res/misc/holding-left.png");
    princeHoldingRightTexture = loadImageToTexture("../res/misc/holding-right.png");

    // NPCs
    string npcPath = "../res/npc/";
    for (int i = 0; i < 8; ++i) {
        string path = npcPath + to_string(i + 1) + ".png";
        npcTextures[i] = loadImageToTexture(path.c_str());
    }

    // quotes
    string quotePath = "../res/quotes/";
    for (int i = 0; i < 8; ++i) {
        string path = quotePath + to_string(i + 1) + ".png";
        quoteTextures[i] = loadImageToTexture(path.c_str());
    }

    // fox quote
    foxQuoteTexture = loadImageToTexture("../res/quotes/fox.png");

    // student info
    studentInfoTexture = loadImageToTexture("../res/text/student-info-350-467.png");
}

void formVAOs() {
    float verticesRect[] = {
        -0.5f, 0.5f, 0.0f, 1.0f, // Top Left
        -0.5f, -0.5f, 0.0f, 0.0f, // Bottom Left
         0.5f, -0.5f, 1.0f, 0.0f, // Bottom Right
         0.5f, 0.5f, 1.0f, 1.0f  // Top Right
    };

    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);
    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRect), verticesRect, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float verticesStar[12 * 6]; // 12 points, x,y + r,g,b,a
    verticesStar[0] = 0.0f;
    verticesStar[1] = 0.0f;
    verticesStar[2] = 1.0f;
    verticesStar[3] = 1.0f;
    verticesStar[4] = 0.8f;
    verticesStar[5] = 0.0f;

    for (int i = 1; i < 12; ++i) {
        float angle = i * 2 * M_PI / 10 + M_PI / 2;
        float r = 0.05f;
        if (i % 2 == 1) r = 0.02f;
        verticesStar[i * 6] = cos(angle) * r;
        verticesStar[i * 6 + 1] = sin(angle) * r;
        // color
        verticesStar[i * 6 + 2] = 1.0f;
        verticesStar[i * 6 + 3] = 1.0f;
        verticesStar[i * 6 + 4] = 0.8f;
        verticesStar[i * 6 + 5] = 0.0f;
    }
    glGenVertexArrays(1, &VAOstar);
    glGenBuffers(1, &VBOstar);
    glBindVertexArray(VAOstar);

    glBindBuffer(GL_ARRAY_BUFFER, VBOstar);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesStar), verticesStar, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float verticesNight[] = {
    //    x,    y,       r,     g,     b,   aBase
        -1.0f,  1.0f,  0.06f, 0.05f, 0.18f, 0.0f,
        -1.0f, -1.0f,  0.06f, 0.05f, 0.18f, 0.0f,
         1.0f, -1.0f,  0.06f, 0.05f, 0.18f, 0.0f,
         1.0f,  1.0f,  0.06f, 0.05f, 0.18f, 0.0f,
    };
    glGenVertexArrays(1, &VAOnight);
    glGenBuffers(1, &VBOnight);
    glBindVertexArray(VAOnight);

    glBindBuffer(GL_ARRAY_BUFFER, VBOnight);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesNight), verticesNight, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// -------- Main --------

int main() {
    initializeGLFW();

    GLFWwindow* window = setupWindow();
    if (window == nullptr) return endProgram("Prozor nije uspeo da se inicijalizuje.");

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");  // initialize GLEW

    // -- vertices, shaders, textures --

    // Potrebno naglasiti da program koristi alfa kanal za providnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    rectShader = createShader("../shader/rect.vert", "../shader/rect.frag");
    colorShader = createShader("../shader/color.vert", "../shader/color.frag");

    formVAOs();
    loadTextures();

    linkCallbackFunctions(window);
    linkCursor(window);

    glClearColor(0.8f, 0.45f, 0.3f, 1.0); // Postavljanje boje pozadine

    while (!glfwWindowShouldClose(window)) {
        // Tajmer na početku frejma
        double startTime = glfwGetTime();

        // 1. Input Handling
        // Svejedno je da li izlaz na Escape definišeno sa ili bez callback-a jer se program isključuje čim detektuje pritisak tastera
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        // switch planet
        for (int i = 0; i < 8; i++) {
            if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS) currentPlanet = i;
        }

        // movement
        bool princeMoving = false;
        bool foxMoving = false;

        Direction prevPrinceDir = princeMoveDir;
        Direction princeDir = DIR_NONE;
        Direction foxDir = DIR_NONE;

        // prince: A + D
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            princeDir = DIR_LEFT;
            princeMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            princeDir = DIR_RIGHT;
            princeMoving = true;
        }

        // fox: LEFT + RIGHT
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            foxDir = DIR_LEFT;
            foxMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            foxDir = DIR_RIGHT;
            foxMoving = true;
        }

        if (princeDir == DIR_NONE) {
            if (prevPrinceDir == DIR_LEFT) princeDir = DIR_LEFT;
            else princeDir = DIR_RIGHT; // default
        }
        princeMoveDir = princeDir;

        if (foxDir == DIR_NONE) {
            foxDir = foxMoveDir;
        }
        foxMoveDir = foxDir;

        // planet rotation (like clock)
        planetRotation += PLANET_ROTATION_SPEED;

        // move prince
        if (princeMoving) {
            if (princeDir == DIR_RIGHT) {  // with planet
                princeAngle -= SPEED_PRINCE_RIGHT;
            } else {  // opposite of planet
                princeAngle += SPEED_PRINCE_LEFT;
            }
        }

        // move fox
        if (foxMoving) {
            if (foxDir == DIR_RIGHT) {  // with planet
                foxAngle -= SPEED_FOX_RIGHT;
            } else {  // opposite of planet
                foxAngle += SPEED_FOX_LEFT;
            }
        }

        if (princeAngle > 2*M_PI) princeAngle -= 2*M_PI;
        if (princeAngle < 0) princeAngle += 2*M_PI;
        if (foxAngle > 2*M_PI) foxAngle -= 2*M_PI;
        if (foxAngle < 0) foxAngle += 2*M_PI;

        // prince + fox: holding (overlap)
        bool overlap = false;
        float angleDiff = abs(princeAngle - foxAngle);
        if (angleDiff > M_PI) angleDiff = 2*M_PI - angleDiff;
        if (angleDiff < 0.1f) overlap = true;

        //  ------------- SUNSET -------------
        float t = static_cast<float>(glfwGetTime());

        // durations in seconds
        const float T_BLUE1   = 5.0f;  // blue day
        const float T_PINK    = 3.0f;  // blue -> pink
        const float T_PURPLE  = 3.0f;  // pink -> purple
        const float T_NIGHT   = 5.0f;  // purple -> night
        const float T_BLUE2   = 7.0f;  // night -> blue morning
        const float CYCLE_LEN = T_BLUE1 + T_PINK + T_PURPLE + T_NIGHT + T_BLUE2; // 13.0

        float tc = fmod(t, CYCLE_LEN); // time within one cycle

        auto lerp = [](float a, float b, float f) { return a + (b - a) * f; };

        // Smooth easing 0..1 -> 0..1 (optional but nicer than linear)
        auto smooth01 = [](float x) {
            if (x < 0.0f) x = 0.0f;
            if (x > 1.0f) x = 1.0f;
            return x * x * (3.0f - 2.0f * x);
        };

        const float rBlue   = 0.20f, gBlue   = 0.35f, bBlue   = 0.65f;
        const float rPink   = 0.65f, gPink   = 0.30f, bPink   = 0.50f;
        const float rPurple = 0.30f, gPurple = 0.17f, bPurple = 0.42f;
        const float rNight  = 0.06f, gNight  = 0.07f, bNight  = 0.18f;

        float skyR = rBlue, skyG = gBlue, skyB = bBlue;
        float nightFactor = 0.0f; // 0..1, used by drawNight
        float starFactor  = 0.0f; // 0..1, used by drawStars

        float t0 = 0.0f;
        float t1 = t0 + T_BLUE1;          // blue
        float t2 = t1 + T_PINK;           // blue->pink
        float t3 = t2 + T_PURPLE;         // pink->purple
        float t4 = t3 + T_NIGHT;          // purple->night
        float t5 = t4 + T_BLUE2;          // night->blue (== CYCLE_LEN)

        // --- Sky color (blue -> pink -> purple -> night -> blue) ---
        if (tc < t1) {
            // Pure blue day
            skyR = rBlue; skyG = gBlue; skyB = bBlue;
        } else if (tc < t2) {
            // Blue -> Pink
            float f = (tc - t1) / T_PINK;       // 0..1
            skyR = lerp(rBlue,   rPink,   f);
            skyG = lerp(gBlue,   gPink,   f);
            skyB = lerp(bBlue,   bPink,   f);
        } else if (tc < t3) {
            // Pink -> Purple
            float f = (tc - t2) / T_PURPLE;     // 0..1
            skyR = lerp(rPink,   rPurple, f);
            skyG = lerp(gPink,   gPurple, f);
            skyB = lerp(bPink,   bPurple, f);
        } else if (tc < t4) {
            // Purple -> Night
            float f = (tc - t3) / T_NIGHT;      // 0..1
            skyR = lerp(rPurple, rNight,  f);
            skyG = lerp(gPurple, gNight,  f);
            skyB = lerp(bPurple, bNight,  f);

            nightFactor = f; // night overlay strength 0..1 over this segment
        } else {
            // tc < t5
            // Night -> Blue morning (fade out night)
            float fLin = (tc - t4) / T_BLUE2;  // 0..1
            float f    = smooth01(fLin);

            // Sky color: night -> blue
            skyR = lerp(rNight,  rBlue,   f);
            skyG = lerp(gNight,  gBlue,   f);
            skyB = lerp(bNight,  bBlue,   f);

            // Night overlay fades out: 1 -> 0
            nightFactor = smooth01(1.0f - fLin);
        }

        // --- Stars -----

        // 1) From start of PURPLE (t2) through NIGHT end (t4):
        if (tc >= t2 && tc < t4) {
            float f = (tc - t2) / (t4 - t2); // 0..1 over (PURPLE + NIGHT) = 6.5s
            starFactor = f;
        }
        // 2) Early BLUE2 (morning): fade out over first 1.5s of BLUE2
        else if (tc >= t4 && tc < t5) {
            float tBlue2Local = tc - t4;  // 0..T_BLUE2
            float fadeOutDur  = 4.0f;     // fade out over first half of BLUE2
            if (tBlue2Local < fadeOutDur) {
                float f = 1.0f - (tBlue2Local / fadeOutDur); // 1..0
                starFactor = f;
            } else {
                starFactor = 0.0f;
            }
        }
        // 0) Otherwise (BLUE1 and PINK): no stars
        else {
            starFactor = 0.0f;
        }

        // Clamp just in case
        if (starFactor < 0.0f) starFactor = 0.0f;
        if (starFactor > 1.0f) starFactor = 1.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine, potrebno kako pomerajući objekti ne bi ostavljali otisak

        // Redosled iscrtavanja objekata je bitno da bi se oni iscrtali u prikladnom ispred-iza poretku
        drawSky();
        drawDayNightTint(skyR, skyG, skyB, nightFactor);
        drawStars(starPositions, starFactor);

        // --- draw planet --
        float planetRadius = PLANET_ORBIT_RADIUS * PLANET_SCALE[currentPlanet];
        vec2 planetScale = { planetRadius, planetRadius };
        drawTexturedRect(
            planetTextures[currentPlanet],
            {0.0f, 0.0f},
            planetScale,
            planetRotation,
            false
        );

        float npcAngle = planetRotation;
        float npcRadius = NPC_ORBIT_RADIUS + NPC_Y_OFFSET[currentPlanet];

        float npcX = cos(npcAngle) * npcRadius;
        float npcY = sin(npcAngle) * npcRadius * aspectRatio;
        float npcRot = npcAngle - M_PI / 2.0f;
        vec2 npcScale = {
            NPC_SCALE_X[currentPlanet],
            NPC_SCALE_Y[currentPlanet]
        };
        drawTexturedRect(
            npcTextures[currentPlanet],
            {npcX, npcY},
            npcScale,
            npcRot,
            false
        );

        // prince and fox textures
        unsigned int princeTex = princeStandRight;
        unsigned int foxTex = foxStandRight;

        int animFrame = (int)(glfwGetTime() * 8.0) % 2; // which walking/running: 1 or 2

        // prince walking animation
        if (princeMoving) {
            if (princeDir == DIR_RIGHT) {
                princeTex = princeWalkRight[animFrame];
            } else {
                princeTex = princeWalkLeft[animFrame];
            }
        } else {
            if (princeMoveDir == DIR_LEFT) {
                princeTex = princeStandLeft;
            } else {
                princeTex = princeStandRight;
            }
        }

        // fox running animation
        if (foxMoving) {
            if (foxDir == DIR_RIGHT) {
                foxTex = foxWalkRight[animFrame];
            } else {
                foxTex = foxWalkLeft[animFrame];
            }
        } else {
            if (foxMoveDir == DIR_LEFT) {
                foxTex = foxStandLeft;
            } else {
                foxTex = foxStandRight;
            }
        }

        float visualPrinceAngle = princeAngle + planetRotation;
        // prince
        float pX = cos(visualPrinceAngle) * PRINCE_ORBIT_RADIUS;
        float pY = sin(visualPrinceAngle) * PRINCE_ORBIT_RADIUS * aspectRatio;
        float pRot = visualPrinceAngle - M_PI / 2.0f;

        float visualFoxAngle    = foxAngle    + planetRotation;
        // fox
        float fX = cos(visualFoxAngle) * FOX_ORBIT_RADIUS;
        float fY = sin(visualFoxAngle) * FOX_ORBIT_RADIUS * aspectRatio;
        float fRot = visualFoxAngle - M_PI / 2.0f;
        foxPosX = fX;
        foxPosY = fY;

        if (overlap) {
            unsigned int holdingTex = princeMoveDir == DIR_LEFT ? princeHoldingLeftTexture : princeHoldingRightTexture;
            float hx = cos(visualPrinceAngle);
            float hy = sin(visualPrinceAngle) * aspectRatio;

            float hLen = sqrt(hx * hx + hy * hy);
            if (hLen > 0.0f) {
                hx /= hLen;
                hy /= hLen;
            }
            float holdOffset = 0.08f;
            float holdX = pX + hx * holdOffset;
            float holdY = pY + hy * holdOffset;

            vec2 holdingScale = { 0.32f, 0.43f };
            drawTexturedRect(holdingTex, {holdX, holdY}, holdingScale, pRot);

        } else {
            drawTexturedRect(foxTex, {fX, fY}, {0.3f, 0.3f}, fRot);
            drawTexturedRect(princeTex, {pX, pY}, {0.3f, 0.3f}, pRot);
        }

        drawStudentInfo();

        // prince close to the quote
        float dx = pX - npcX;
        float dy = pY - npcY;
        float dist2 = dx * dx + dy * dy;
        const float QUOTE_DIST = 0.25f;
        const float QUOTE_DIST2 = QUOTE_DIST * QUOTE_DIST;

        if (dist2 < QUOTE_DIST2) {
            drawQuote(currentPlanet);
        }

        if (showFoxQuote) {
            drawFoxQuoteBottomRight();
        }

        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja

        // Frame limiter: kada se frejm iscrta, čeka se da prođe 1 / 60 sekunde, zatim se prelazi na sledeći frejm frejma
        while (glfwGetTime() - startTime < 1 / 75.0) {}
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}