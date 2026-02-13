#ifndef SHADER_H
#define SHADER_H

// load and compile shaders
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace glm;

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath) {
        string vertexCode;
        string fragmentCode;
        ifstream vShaderFile;
        ifstream fShaderFile;
        vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (ifstream::failure& e) {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << vertexPath << " or " << fragmentPath << endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    void use() const { glUseProgram(ID); }
    void setBool(const string& name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
    void setInt(const string& name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
    void setFloat(const string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
    void setVec2(const string& name, const vec2& value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
    void setVec3(const string& name, const vec3& value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
    void setVec4(const string& name, const vec4& value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
    void setMat4(const string& name, const mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
    void setMat4Array(const string& name, const vector<mat4>& mats) const {
        if (mats.empty()) return;
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), mats.size(), GL_FALSE, value_ptr(mats[0]));
    }

private:
    void checkCompileErrors(GLuint shader, string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << endl;
            }
        }
    }
};
#endif
