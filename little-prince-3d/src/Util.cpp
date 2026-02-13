#include "Util.h"

#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// helpers for program termination, shader loading, textures and cursors

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

unsigned int compileShader(GLenum type, const char* source) {
    // load shader source from file and compile it
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;

    if (file.is_open()) {
        ss << file.rdbuf();
        file.close();
        std::cout << "successfully read file: " << source << std::endl;

    } else {
        ss << "";
        std::cout << "error reading file: " << source << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" shader error! error: \n");
        printf("%s\n", infoLog);
    }
    return shader;
}

unsigned int createShader(const char* vsSource, const char* fsSource) {
    // create combined shader program
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "shader program error! error: \n";
        std::cout << infoLog << std::endl;
    }

    // cleanup individual shaders
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);

    if (ImageData != NULL) {
        // vertical flip for opengl
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // check color format
        GLint InternalFormat = -1;
        switch (TextureChannels) {
            case 1: InternalFormat = GL_RED; break;
            case 2: InternalFormat = GL_RG; break;
            case 3: InternalFormat = GL_RGB; break;
            case 4: InternalFormat = GL_RGBA; break;
            default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat,
                     TextureWidth, TextureHeight, 0,
                     InternalFormat, GL_UNSIGNED_BYTE, ImageData);

        // mipmaps and filtering
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(ImageData);
        return Texture;

    } else {
        std::cout << "texture failed to load! path: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

GLFWcursor* loadImageToCursor(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;

    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);

    if (ImageData != NULL) {
        GLFWimage image;
        image.width = TextureWidth;
        image.height = TextureHeight;
        image.pixels = ImageData;

        // cursor hitbox hotspot
        int hotspotX = 3;
        int hotspotY = 3;

        GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);
        stbi_image_free(ImageData);
        return cursor;

    } else {
        std::cout << "cursor failed to load! path: " << filePath << std::endl;
        stbi_image_free(ImageData);
    }

    return nullptr;
}
