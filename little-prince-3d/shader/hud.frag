#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hudTexture;
uniform float alpha;
uniform vec3 tintColor;
uniform bool useTint;

void main() {
    vec4 texColor = texture(hudTexture, TexCoords);
    
    vec3 color = texColor.rgb;
    if (useTint) {
        color *= tintColor;
    }
    
    FragColor = vec4(color, texColor.a * alpha);
}
