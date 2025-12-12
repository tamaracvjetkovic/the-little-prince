#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;
out vec2 chTex;

uniform vec2 uPos;
uniform vec2 uScale;
uniform float uRotation;
uniform bool flipped;

void main() {
    // scaling
    vec2 scaledPos = inPos * uScale;

    // rotation
    float s = sin(uRotation);
    float c = cos(uRotation);
    vec2 rotatedPos = vec2(
        scaledPos.x * c - scaledPos.y * s,
        scaledPos.x * s + scaledPos.y * c
    );

    // correction + translate
    vec2 correctedPos = vec2(rotatedPos.x, rotatedPos.y * 1.47f);
    gl_Position = vec4(correctedPos + uPos, 0.0, 1.0);

    if (flipped) {
        chTex = vec2(1.0 - inTex.s, inTex.t);
    } else {
        chTex = inTex;
    }
}