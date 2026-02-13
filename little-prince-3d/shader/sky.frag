#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float time;
uniform vec3 topColor;
uniform vec3 bottomColor;
uniform float cloudAlpha;

// Simple hash function for noise
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// 2D Noise
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// Fractional Brownian Motion for clouds
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 m = mat2(1.6,  1.2, -1.2,  1.6);
    for (int i = 0; i < 5; i++) {
        v += a * noise(p);
        p = m * p;
        a *= 0.5;
    }
    return v;
}

void main() {
    // Gradient background
    vec3 sky = mix(bottomColor, topColor, TexCoords.y);
    FragColor = vec4(sky, 1.0);
}
