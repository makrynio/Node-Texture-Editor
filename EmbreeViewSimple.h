#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <functional>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "learnopengl/shader.h"
#include "learnopengl/camera.h"
#include "learnopengl/texture_manager.h"
#include "learnopengl/filesystem.h"
#include "ModelLoader.h"

// Forward declaration
class PBRView;

class EmbreeViewSimple
{
public:
    EmbreeViewSimple();
    ~EmbreeViewSimple();

    // Initialize the view with a specific size
    bool Initialize(int width, int height);
    
    // Render the content into an ImGui window
    void Render(const char* windowTitle, bool* p_open = nullptr);
    
    // Set the render callback function
    void SetRenderCallback(std::function<void()> callback);
    
    // Get the current size
    ImVec2 GetSize() const { return m_Size; }
    
    // Check if the view is valid
    bool IsValid() const { return m_Valid; }
    
    // Load 3D model from file
    bool LoadModelFromFile(const std::string& filepath);
    
    // Get current model info
    std::string GetModelInfo() const;
    
    // Handle mouse input for zoom and rotation
    void ProcessMouseInput(float deltaTime);
    
    // Check if mouse is over the view
    bool IsMouseOverView() const;
    
    // Focus and frame the current model
    void FocusOnModel();
    
    // Reset camera view to default position
    void ResetCameraView();
    
    // Sync with PBR view data
    void SyncWithPBRView(const class PBRView* pbrView);
    
    // Get ModelLoader for accessing mesh data
    ModelLoader& GetModelLoader() { return m_ModelLoader; }

private:
    void CreateFramebuffer();
    void DestroyFramebuffer();
    void UpdateSize(int width, int height);
    void RenderScene();
    unsigned int LoadTexture(const char *path);
    
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
    
    // Simple shader for basic rendering
    std::unique_ptr<Shader> m_SimpleShader;
    
    // Camera
    std::unique_ptr<Camera> m_Camera;
    
    // Matrices
    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_Model;
    
    // Model loading
    ModelLoader m_ModelLoader;
    bool m_UseCustomModel;
    
    // Mouse input state
    bool m_IsMouseOverView;
    bool m_IsRightMousePressed;
    bool m_IsLeftMousePressed;
    bool m_IsOptionPressed;
    glm::vec2 m_LastMousePos;
    float m_LastFrameTime;
    
    // Orbit camera state
    glm::vec3 m_OrbitTarget;
    float m_OrbitDistance;
    float m_OrbitYaw;
    float m_OrbitPitch;
    
    // Methods
    void UpdateOrbitCamera();
    void SetOrbitTarget(const glm::vec3& target);
    void RenderSphere();
    void RenderModel();
};
