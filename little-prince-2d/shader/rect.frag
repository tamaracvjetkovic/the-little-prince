#version 330 core

in vec2 chTex;
out vec4 outCol;

uniform sampler2D uTex0;
uniform float uAlpha;

void main() {
    vec4 tex = texture(uTex0, chTex);
    tex.a *= uAlpha;
    outCol = tex;
}