#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 albedo;

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    vec3 L = normalize(lightPos - WorldPos);
    vec3 H = normalize(V + L);
    
    // Simple Phong lighting
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    // Diffuse
    vec3 diffuse = albedo * NdotL;
    
    // Specular
    float specular = pow(NdotH, 32.0);
    
    vec3 color = (diffuse + specular) * lightColor;
    
    FragColor = vec4(color, 1.0);
}

