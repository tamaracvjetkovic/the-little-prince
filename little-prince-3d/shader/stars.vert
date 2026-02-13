#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aBrightness;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float starAlpha;

out float vBrightness;

void main() {
    vBrightness = aBrightness * starAlpha;
    
    // Simple twinkling effect
    vBrightness *= (0.8 + 0.2 * sin(time * 3.0 + aPos.x + aPos.y));

    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aBrightness * 3.0;
}
