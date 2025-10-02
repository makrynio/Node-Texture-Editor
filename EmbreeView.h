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
#include <embree4/rtcore.h>
#include <embree4/rtcore_ray.h>
#include <embree4/rtcore_scene.h>
#include <embree4/rtcore_geometry.h>

#include "learnopengl/shader.h"
#include "learnopengl/camera.h"
#include "learnopengl/texture_manager.h"
#include "learnopengl/filesystem.h"
#include "ModelLoader.h"
#include "NodeSystem.h"
#include "TextureGenerator.h"

class EmbreeView
{
public:
    EmbreeView();
    ~EmbreeView();

    // Initialize the Embree view with a specific size
    bool Initialize(int width, int height);
    
    // Render the ray-traced content into an ImGui window
    void Render(const char* windowTitle, bool* p_open = nullptr);
    
    // Set the render callback function
    void SetRenderCallback(std::function<void()> callback);
    
    // Get the current size
    ImVec2 GetSize() const { return m_Size; }
    
    // Check if the view is valid
    bool IsValid() const { return m_Valid; }
    
    // Load mesh from OBJ file
    bool LoadMeshFromOBJ(const std::string& filename);
    
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
    
    // Ray tracing parameters (public for ImGui access)
    int m_SamplesPerPixel;
    int m_MaxDepth;
    bool m_UsePathTracing;
    
    // Performance optimization
    float m_LastRenderTime;
    float m_RenderInterval;
    bool m_NeedsRender;
    
    // Mark that rendering is needed (call when parameters change)
    void MarkNeedsRender() { m_NeedsRender = true; }
    
    // Material system
    void UpdateTexturesFromNodes();
    glm::vec3 SampleTexture(unsigned int textureID, const glm::vec2& uv) const;
    glm::vec3 GetMaterialAlbedo(const glm::vec2& uv) const;
    float GetMaterialRoughness(const glm::vec2& uv) const;
    float GetMaterialMetallic(const glm::vec2& uv) const;
    glm::vec3 GetMaterialNormal(const glm::vec2& uv, const glm::vec3& worldNormal, const glm::vec3& tangent, const glm::vec3& bitangent) const;
    float GetMaterialHeight(const glm::vec2& uv) const;
    float GetMaterialAO(const glm::vec2& uv) const;
    glm::vec2 CalculateUV(const glm::vec3& hitPoint, const glm::vec3& normal) const;
    glm::vec3 CalculateTangentSpace(const glm::vec3& normal, const glm::vec3& hitPoint) const;

private:
    void CreateFramebuffer();
    void DestroyFramebuffer();
    void UpdateSize(int width, int height);
    bool SetupEmbree();
    void CleanupEmbree();
    void BuildScene();
    void RenderScene();
    void RayTrace();
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
    
    // Embree components
    RTCDevice m_EmbreeDevice;
    RTCScene m_EmbreeScene;
    RTCGeometry m_EmbreeGeometry;
    
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
    
    // Material textures from node system
    unsigned int m_AlbedoTexture;
    unsigned int m_RoughnessTexture;
    unsigned int m_MetallicTexture;
    unsigned int m_NormalTexture;
    unsigned int m_HeightTexture;
    unsigned int m_AOTexture;
    unsigned int m_DefaultGreyTexture;
    
    // Texture data for CPU sampling
    std::map<unsigned int, std::shared_ptr<TextureData>> m_TextureDataCache;
    
    
    // Methods
    void UpdateOrbitCamera();
    void SetOrbitTarget(const glm::vec3& target);
    glm::vec3 TraceRay(const RTCRay& ray, int depth);
    glm::vec3 SampleEnvironment(const glm::vec3& direction);
    bool IntersectScene(const RTCRay& ray, RTCIntersectArguments& args);
};
