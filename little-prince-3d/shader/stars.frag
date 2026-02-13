#version 330 core
out vec4 FragColor;
in float vBrightness;

void main() {
    if (vBrightness < 0.01) discard;
    FragColor = vec4(1.0, 1.0, 1.0, vBrightness);
}
