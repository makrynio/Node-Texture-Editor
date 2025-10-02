#include "OpenGLView.h"
#include <iostream>

OpenGLView::OpenGLView()
    : m_Framebuffer(0)
    , m_ColorTexture(0)
    , m_DepthStencilBuffer(0)
    , m_Size(800, 600)
    , m_Valid(false)
    , m_SizeChanged(false)
{
}

OpenGLView::~OpenGLView()
{
    DestroyFramebuffer();
}

bool OpenGLView::Initialize(int width, int height)
{
    m_Size = ImVec2(static_cast<float>(width), static_cast<float>(height));
    CreateFramebuffer();
    return m_Valid;
}

void OpenGLView::CreateFramebuffer()
{
    // Clean up existing framebuffer
    DestroyFramebuffer();
    
    // Create framebuffer
    glGenFramebuffers(1, &m_Framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    
    // Create color texture
    glGenTextures(1, &m_ColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);
    
    // Create depth-stencil buffer
    glGenRenderbuffers(1, &m_DepthStencilBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilBuffer);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete!" << std::endl;
        m_Valid = false;
        return;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Valid = true;
}

void OpenGLView::DestroyFramebuffer()
{
    if (m_ColorTexture)
    {
        glDeleteTextures(1, &m_ColorTexture);
        m_ColorTexture = 0;
    }
    
    if (m_DepthStencilBuffer)
    {
        glDeleteRenderbuffers(1, &m_DepthStencilBuffer);
        m_DepthStencilBuffer = 0;
    }
    
    if (m_Framebuffer)
    {
        glDeleteFramebuffers(1, &m_Framebuffer);
        m_Framebuffer = 0;
    }
    
    m_Valid = false;
}

void OpenGLView::UpdateSize(int width, int height)
{
    if (width != static_cast<int>(m_Size.x) || height != static_cast<int>(m_Size.y))
    {
        m_Size = ImVec2(static_cast<float>(width), static_cast<float>(height));
        m_SizeChanged = true;
    }
}

void OpenGLView::SetRenderCallback(std::function<void()> callback)
{
    m_RenderCallback = callback;
}

void OpenGLView::Render(const char* windowTitle, bool* p_open)
{
    if (!m_Valid)
        return;
    
    // Get the current ImGui window size
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    if (windowSize.x < 50.0f) windowSize.x = 50.0f;
    if (windowSize.y < 50.0f) windowSize.y = 50.0f;
    
    // Update size if needed
    UpdateSize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    
    // Recreate framebuffer if size changed
    if (m_SizeChanged)
    {
        CreateFramebuffer();
        m_SizeChanged = false;
    }
    
    // Save current OpenGL state
    glGetIntegerv(GL_VIEWPORT, m_PrevViewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_PrevFramebuffer);
    
    // Render to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    glViewport(0, 0, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
    
    // Clear the framebuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Call the render callback
    if (m_RenderCallback)
    {
        m_RenderCallback();
    }
    
    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, m_PrevFramebuffer);
    glViewport(m_PrevViewport[0], m_PrevViewport[1], m_PrevViewport[2], m_PrevViewport[3]);
    
    // Display the texture in ImGui
    ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(m_ColorTexture)), 
                 ImVec2(m_Size.x, m_Size.y), 
                 ImVec2(0, 1), ImVec2(1, 0)); // Flip Y coordinate
}
