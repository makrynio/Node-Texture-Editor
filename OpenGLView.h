#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <functional>

class OpenGLView
{
public:
    OpenGLView();
    ~OpenGLView();

    // Initialize the OpenGL view with a specific size
    bool Initialize(int width, int height);
    
    // Render the OpenGL content into an ImGui window
    void Render(const char* windowTitle, bool* p_open = nullptr);
    
    // Set the render callback function
    void SetRenderCallback(std::function<void()> callback);
    
    // Get the current size
    ImVec2 GetSize() const { return m_Size; }
    
    // Check if the view is valid
    bool IsValid() const { return m_Valid; }

private:
    void CreateFramebuffer();
    void DestroyFramebuffer();
    void UpdateSize(int width, int height);
    
    GLuint m_Framebuffer;
    GLuint m_ColorTexture;
    GLuint m_DepthStencilBuffer;
    
    ImVec2 m_Size;
    bool m_Valid;
    bool m_SizeChanged;
    
    std::function<void()> m_RenderCallback;
    
    // OpenGL state backup
    GLint m_PrevViewport[4];
    GLint m_PrevFramebuffer;
};
