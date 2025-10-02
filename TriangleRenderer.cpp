#include "TriangleRenderer.h"
#include <iostream>

const char* TriangleRenderer::vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char* TriangleRenderer::fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

TriangleRenderer::TriangleRenderer()
    : m_VAO(0)
    , m_VBO(0)
    , m_ShaderProgram(0)
    , m_VertexShader(0)
    , m_FragmentShader(0)
    , m_Initialized(false)
{
}

TriangleRenderer::~TriangleRenderer()
{
    Cleanup();
}

bool TriangleRenderer::Initialize()
{
    if (m_Initialized)
        return true;
    
    // Create shader program
    if (!CreateShaderProgram())
    {
        std::cerr << "Failed to create shader program" << std::endl;
        return false;
    }
    
    // Set up vertex data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };
    
    // Create VAO and VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    // Bind VAO
    glBindVertexArray(m_VAO);
    
    // Bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    m_Initialized = true;
    return true;
}

void TriangleRenderer::Render()
{
    if (!m_Initialized)
        return;
    
    // Use shader program
    glUseProgram(m_ShaderProgram);
    
    // Draw triangle
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void TriangleRenderer::Cleanup()
{
    if (m_VAO)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    
    if (m_VBO)
    {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    
    if (m_ShaderProgram)
    {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }
    
    if (m_VertexShader)
    {
        glDeleteShader(m_VertexShader);
        m_VertexShader = 0;
    }
    
    if (m_FragmentShader)
    {
        glDeleteShader(m_FragmentShader);
        m_FragmentShader = 0;
    }
    
    m_Initialized = false;
}

GLuint TriangleRenderer::CompileShader(const char* source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool TriangleRenderer::CreateShaderProgram()
{
    // Compile shaders
    m_VertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
    if (!m_VertexShader)
        return false;
    
    m_FragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    if (!m_FragmentShader)
    {
        glDeleteShader(m_VertexShader);
        return false;
    }
    
    // Create shader program
    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, m_VertexShader);
    glAttachShader(m_ShaderProgram, m_FragmentShader);
    glLinkProgram(m_ShaderProgram);
    
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteShader(m_VertexShader);
        glDeleteShader(m_FragmentShader);
        glDeleteProgram(m_ShaderProgram);
        return false;
    }
    
    // Clean up shaders (they're now linked into the program)
    glDeleteShader(m_VertexShader);
    glDeleteShader(m_FragmentShader);
    m_VertexShader = 0;
    m_FragmentShader = 0;
    
    return true;
}
