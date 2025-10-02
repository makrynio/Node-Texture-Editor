#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "learnopengl/shader.h"
#include "learnopengl/camera.h"
#include "learnopengl/texture_manager.h"
#include "learnopengl/filesystem.h"
#include "ModelLoader.h"
#include "RemeshManager.h"
#include "QuadriflowWrapper.h"

class PBRView
{
public:
    PBRView();
    ~PBRView();

    // Initialize the PBR view with a specific size
    bool Initialize(int width, int height);
    
    // Render the PBR content into an ImGui window
    void Render(const char* windowTitle, bool* p_open = nullptr);
    
    // Set the render callback function
    void SetRenderCallback(std::function<void()> callback);
    
    // Get the current size
    ImVec2 GetSize() const { return m_Size; }
    
    // Check if the view is valid
    bool IsValid() const { return m_Valid; }
    
    // Set material textures
    void SetMaterialTextures(unsigned int albedo, unsigned int normal, 
                           unsigned int metallic, unsigned int roughness, 
                           unsigned int ao, unsigned int height = 0);
    
    // Set displacement intensity
    void SetDisplacementIntensity(float intensity);
    
    // Load mesh from OBJ file
    bool LoadMeshFromOBJ(const std::string& filename);
    
    // Reload sphere geometry (call after remeshing)
    void ReloadSphereGeometry();
    
    // Revert to original procedural sphere
    void RevertToOriginalMesh();
    
    // Save current model state for revert functionality
    void SaveCurrentModelState();
    
    // Revert to the original model (before remeshing)
    void RevertToOriginalModel();
    
    // Load 3D model from file
    bool LoadModelFromFile(const std::string& filepath);
    
    // Export current model to OBJ file
    bool ExportCurrentModelToOBJ(const std::string& filepath);
    
    // Export custom mesh data to OBJ file
    bool ExportCustomMeshToOBJ(const std::string& filepath);
    
    // Check if using custom model
    bool IsUsingCustomModel() const { return m_UseCustomModel; }
    
    // Get current model info
    std::string GetModelInfo() const;
    
    // Handle mouse input for zoom and rotation
    void ProcessMouseInput(float deltaTime);
    
    // Check if mouse is over the PBR view
    bool IsMouseOverView() const;
    
    // Focus and frame the current model
    void FocusOnModel();
    
    // Reset camera view to default position
    void ResetCameraView();
    
    // Export procedural sphere to OBJ file
    bool ExportProceduralSphereToOBJ(const std::string& filepath);
    
    // Remesh current model
    bool RemeshCurrentModel(const RemeshSettings& settings);
    
    // Quadriflow quadrangulation using external executable
    bool QuadriflowCurrentModel();
    
    // Get remesh manager
    RemeshManager& GetRemeshManager() { return m_RemeshManager; }
    
    // Get Quadriflow wrapper
    QuadriflowWrapper& GetQuadriflowWrapper() { return m_QuadriflowWrapper; }
    
    // Get ModelLoader for accessing mesh data
    ModelLoader& GetModelLoader() { return m_ModelLoader; }
    
    // Check if remesh is available
    bool IsRemeshAvailable() const { return true; }
    
    // Public getters for synchronization with other views
    const Camera* GetCamera() const { return m_Camera.get(); }
    const ModelLoader& GetModelLoader() const { return m_ModelLoader; }
    glm::vec3 GetOrbitTarget() const { return m_OrbitTarget; }
    float GetOrbitDistance() const { return m_OrbitDistance; }
    float GetOrbitYaw() const { return m_OrbitYaw; }
    float GetOrbitPitch() const { return m_OrbitPitch; }

private:
    void CreateFramebuffer();
    void DestroyFramebuffer();
    void UpdateSize(int width, int height);
    void SetupShaders();
    void SetupIBL();
    void RenderSphere();
    void RenderCube();
    void RenderQuad();
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
    
    // PBR rendering components
    std::unique_ptr<Shader> m_PBRShader;
    std::unique_ptr<Shader> m_BackgroundShader;
    std::unique_ptr<Shader> m_SimpleShader;
    std::unique_ptr<Shader> m_EquirectangularToCubemapShader;
    std::unique_ptr<Shader> m_IrradianceShader;
    std::unique_ptr<Shader> m_PrefilterShader;
    std::unique_ptr<Shader> m_BRDFShader;
    
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<TextureManager> m_TextureManager;
    
    // IBL textures
    unsigned int m_EnvCubemap;
    unsigned int m_IrradianceMap;
    unsigned int m_PrefilterMap;
    unsigned int m_BRDFLUTTexture;
    unsigned int m_HDRTexture;
    
    // Geometry
    unsigned int m_SphereVAO;
    unsigned int m_SphereVBO;
    unsigned int m_SphereEBO;
    unsigned int m_SphereUVBO;
    unsigned int m_SphereNormalBO;
    unsigned int m_CubeVAO;
    unsigned int m_QuadVAO;
    GLsizei m_SphereIndexCount;
    
    // Material textures
    unsigned int m_AlbedoTexture;
    unsigned int m_NormalTexture;
    unsigned int m_MetallicTexture;
    unsigned int m_RoughnessTexture;
    unsigned int m_AOTexture;
    unsigned int m_HeightTexture;
    
    // Lighting
    glm::vec3 m_LightPositions[4];
    glm::vec3 m_LightColors[4];
    
    // Matrices
    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_Model;
    
    // Displacement
    float m_DisplacementIntensity;
    
    // IBL setup flag
    bool m_IBLSetup;
    
    // Default textures for blank channels
    unsigned int m_DefaultGreyTexture;
    
    // Mesh data
    std::vector<glm::vec3> m_MeshPositions;
    std::vector<glm::vec2> m_MeshUVs;
    std::vector<glm::vec3> m_MeshNormals;
    std::vector<unsigned int> m_MeshIndices;
    bool m_UseCustomMesh;
    
    // Model loading
    ModelLoader m_ModelLoader;
    bool m_UseCustomModel;
    
    // Remesh functionality
    RemeshManager m_RemeshManager;
    
    // Quadriflow wrapper
    QuadriflowWrapper m_QuadriflowWrapper;
    
    // Original model state for revert functionality
    std::string m_OriginalModelPath;
    bool m_HasOriginalModel;
    bool m_UseOriginalCustomMesh;
    std::vector<glm::vec3> m_OriginalMeshPositions;
    std::vector<glm::vec2> m_OriginalMeshUVs;
    std::vector<glm::vec3> m_OriginalMeshNormals;
    std::vector<unsigned int> m_OriginalMeshIndices;
    
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
    unsigned int CreateDefaultGreyTexture();
    void UpdateTexturesFromNodes();
    void DrawMeshInCurrentContext(const Mesh& mesh);
    void UpdateOrbitCamera();
    void SetOrbitTarget(const glm::vec3& target);
};
