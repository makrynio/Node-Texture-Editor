#pragma once

#include <glad/glad.h>
#include <memory>

class TriangleRenderer
{
public:
    TriangleRenderer();
    ~TriangleRenderer();
    
    // Initialize the triangle renderer
    bool Initialize();
    
    // Render the triangle
    void Render();
    
    // Cleanup
    void Cleanup();

private:
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_ShaderProgram;
    GLuint m_VertexShader;
    GLuint m_FragmentShader;
    
    bool m_Initialized;
    
    // Shader source code
    static const char* vertexShaderSource;
    static const char* fragmentShaderSource;
    
    // Compile shader
    GLuint CompileShader(const char* source, GLenum type);
    
    // Create shader program
    bool CreateShaderProgram();
};
