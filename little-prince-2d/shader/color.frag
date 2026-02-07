#version 330 core

in vec4 chCol;
out vec4 outCol;

uniform float uA;  // extra aplha (transparency)
uniform vec3 uTintColor;  // RGB tint
uniform float uTintAmount;  // how much tint to apply

void main() {
    vec3 baseColor = vec3(chCol.r, chCol.g, chCol.b);

    // mix: baseColor + tintColor
    vec3 finalColor = mix(baseColor, uTintColor, uTintAmount);

    float finalAlpha = chCol.a + uA;
    outCol = vec4(finalColor, finalAlpha);
}