#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1; // metallic map
uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform float globalAlpha;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLight;
uniform bool usePointLight;

void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // transparency fix
    if(texColor.a < 0.01) discard;

    vec3 color = texColor.rgb;
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // directional light
    vec3 lightDir = normalize(-dirLight.direction);
    // Ambient
    vec3 ambient = dirLight.ambient * color;
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * color;
    // specular (blinn-phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float roughness = texture(texture_roughness1, TexCoords).r;
    float shininess = mix(64.0, 2.0, roughness); 
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float specularStrength = texture(texture_specular1, TexCoords).r;
    vec3 specular = dirLight.specular * spec * specularStrength;
    
    vec3 result = ambient + diffuse + specular;

    // point light
    if (usePointLight) {
        vec3 pLightDir = normalize(pointLight.position - FragPos);
        // diffuse shading
        float pDiff = max(dot(normal, pLightDir), 0.0);
        // specular shading
        vec3 pHalfwayDir = normalize(pLightDir + viewDir);
        float pSpec = pow(max(dot(normal, pHalfwayDir), 0.0), shininess);
        // attenuation
        float distance = length(pointLight.position - FragPos);
        float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + 
      			     pointLight.quadratic * (distance * distance));    
        // combine results
        vec3 pAmbient = pointLight.ambient * color;
        vec3 pDiffuse = pointLight.diffuse * pDiff * color;
        vec3 pSpecular = pointLight.specular * pSpec * specularStrength;
        
        result += (pAmbient + pDiffuse + pSpecular) * attenuation;
    }
    
    FragColor = vec4(result, texColor.a * globalAlpha);
}
