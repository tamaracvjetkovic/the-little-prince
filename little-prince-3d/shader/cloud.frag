#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float alpha;
uniform float time;

// Simple hash for noise
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

// FBM for wispy texture
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 4; i++) {
        v += a * noise(p);
        p *= 2.02;
        a *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = TexCoords;
    
    // Create a mask to fade out at edges of the billboard quad
    float edgeMask = smoothstep(0.0, 0.1, uv.x) * smoothstep(1.0, 0.9, uv.x) *
                     smoothstep(0.0, 0.1, uv.y) * smoothstep(1.0, 0.9, uv.y);
    
    // Softer coordinate transformation for wispy cirrus
    // Reduced the vertical multiplier (from 12.0 down to 4.0-6.0) to avoid "lines"
    // and increased horizontal complexity.
    vec2 noiseUV = vec2(uv.x * 1.2 - uv.y * 0.2, uv.y * 5.0); 
    
    // Distort noiseUV with multiple noise layers for a "feathered", organic feel
    float d1 = fbm(uv * 3.0 + time * 0.01);
    float d2 = fbm(uv * 8.0 - time * 0.02);
    noiseUV += (d1 - d2) * 0.2;

    // Use FBM for the main wisp shape with some scale variation
    float n = fbm(noiseUV + vec2(time * 0.02, 0.0));
    
    // Smooth transitions for a "foggy/wispy" look instead of sharp streaks
    float cirrus = smoothstep(0.3, 0.7, n);
    
    // Add fine, low-contrast fibrous detail
    float detail = fbm(noiseUV * 3.0 + vec2(time * 0.04, 0.1));
    float finalShape = mix(cirrus, detail, 0.4) * edgeMask;
    
    // Softer alpha for a less "solid" look
    float finalAlpha = finalShape * alpha * 0.4;
    
    if (finalAlpha < 0.01) discard;
    
    FragColor = vec4(1.0, 1.0, 1.0, finalAlpha);
}
