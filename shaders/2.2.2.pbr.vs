#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

// Displacement uniforms
uniform sampler2D heightMap;
uniform bool useDisplacement;
uniform float displacementScale;

void main()
{
    TexCoords = aTexCoords;
    
    vec3 pos = aPos;
    vec3 normal = aNormal;
    
    // Apply vertex displacement if enabled
    if (useDisplacement) {
        // Sample height from texture (use red channel of checkerboard)
        float height = texture(heightMap, aTexCoords).r;
        
        // Scale the displacement
        float displacement = (height - 0.5) * displacementScale;
        
        // Displace vertex along normal
        pos += normal * displacement;
    }
    
    WorldPos = vec3(model * vec4(pos, 1.0));
    Normal = normalMatrix * normal;   

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
