#include "EmbreeViewSimple.h"
#include "PBRView.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "learnopengl/filesystem.h"

EmbreeViewSimple::EmbreeViewSimple()
    : m_Framebuffer(0)
    , m_ColorTexture(0)
    , m_DepthStencilBuffer(0)
    , m_Size(800, 600)
    , m_Valid(false)
    , m_SizeChanged(false)
    , m_UseCustomModel(false)
    , m_IsMouseOverView(false)
    , m_IsRightMousePressed(false)
    , m_IsLeftMousePressed(false)
    , m_IsOptionPressed(false)
    , m_LastMousePos(0.0f, 0.0f)
    , m_LastFrameTime(0.0f)
    , m_OrbitTarget(0.0f, 0.0f, 0.0f)
    , m_OrbitDistance(4.0f)
    , m_OrbitYaw(-90.0f)
    , m_OrbitPitch(0.0f)
{
}

EmbreeViewSimple::~EmbreeViewSimple()
{
    DestroyFramebuffer();
}

bool EmbreeViewSimple::Initialize(int width, int height)
{
    m_Size = ImVec2(static_cast<float>(width), static_cast<float>(height));
    
    // Initialize camera
    m_Camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 4.0f));
    
    // Initialize orbit camera
    m_OrbitTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 4.0f;
    m_OrbitYaw = -90.0f;
    m_OrbitPitch = 0.0f;
    UpdateOrbitCamera();
    
    // Setup simple shader
    m_SimpleShader = std::make_unique<Shader>("shaders/simple.vs", "shaders/simple.fs");
    if (m_SimpleShader->ID == 0) {
        std::cerr << "Failed to load simple shader" << std::endl;
        return false;
    }
    
    CreateFramebuffer();
    return m_Valid;
}

void EmbreeViewSimple::CreateFramebuffer()
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
        std::cerr << "EmbreeViewSimple Framebuffer not complete!" << std::endl;
        m_Valid = false;
        return;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Valid = true;
}

void EmbreeViewSimple::DestroyFramebuffer()
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

void EmbreeViewSimple::UpdateSize(int width, int height)
{
    if (width != static_cast<int>(m_Size.x) || height != static_cast<int>(m_Size.y))
    {
        m_Size = ImVec2(static_cast<float>(width), static_cast<float>(height));
        m_SizeChanged = true;
    }
}

void EmbreeViewSimple::SetRenderCallback(std::function<void()> callback)
{
    m_RenderCallback = callback;
}

void EmbreeViewSimple::Render(const char* windowTitle, bool* p_open)
{
    if (!m_Valid)
    {
        std::cout << "EmbreeViewSimple not valid!" << std::endl;
        return;
    }
    
    // Get the current ImGui window size
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    if (windowSize.x < 50.0f) windowSize.x = 50.0f;
    if (windowSize.y < 50.0f) windowSize.y = 50.0f;
    
    // Update size if needed
    UpdateSize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    
    // Process mouse input for zoom and rotation
    float currentTime = ImGui::GetTime();
    float deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;
    ProcessMouseInput(deltaTime);
    
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
    
    // Set up OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Clear the framebuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup matrices
    m_Projection = glm::perspective(glm::radians(m_Camera->Zoom), m_Size.x / m_Size.y, 0.1f, 100.0f);
    m_View = m_Camera->GetViewMatrix();
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Render the scene
    RenderScene();
    
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

void EmbreeViewSimple::RenderScene()
{
    if (m_SimpleShader && m_SimpleShader->ID != 0)
    {
        m_SimpleShader->use();
        m_SimpleShader->setMat4("projection", m_Projection);
        m_SimpleShader->setMat4("view", m_View);
        m_SimpleShader->setMat4("model", m_Model);
        m_SimpleShader->setVec3("color", glm::vec3(0.7f, 0.3f, 0.3f)); // Red-ish color
        
        if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
            RenderModel();
        } else {
            RenderSphere();
        }
    }
}

void EmbreeViewSimple::RenderSphere()
{
    // Simple sphere rendering - this would be implemented with proper geometry
    // For now, just render a placeholder
    static bool sphereGenerated = false;
    static unsigned int sphereVAO = 0;
    static unsigned int sphereVBO = 0;
    static unsigned int sphereEBO = 0;
    static GLsizei sphereIndexCount = 0;
    
    if (!sphereGenerated) {
        // Generate sphere geometry
        std::vector<glm::vec3> positions;
        std::vector<unsigned int> indices;
        
        const unsigned int X_SEGMENTS = 32;
        const unsigned int Y_SEGMENTS = 32;
        const float PI = 3.14159265359f;
        
        // Generate vertices
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                
                positions.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }
        
        // Generate indices
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
            for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
                unsigned int v0 = y * (X_SEGMENTS + 1) + x;
                unsigned int v1 = v0 + 1;
                unsigned int v2 = (y + 1) * (X_SEGMENTS + 1) + x;
                unsigned int v3 = v2 + 1;
                
                // First triangle
                indices.push_back(v0);
                indices.push_back(v2);
                indices.push_back(v1);
                
                // Second triangle
                indices.push_back(v1);
                indices.push_back(v2);
                indices.push_back(v3);
            }
        }
        
        // Create VAO, VBO, EBO
        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);
        
        glBindVertexArray(sphereVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        
        glBindVertexArray(0);
        
        sphereIndexCount = indices.size();
        sphereGenerated = true;
    }
    
    // Render sphere
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void EmbreeViewSimple::RenderModel()
{
    // Render loaded model using ModelLoader
    const auto& meshes = m_ModelLoader.GetMeshes();
    for (const auto& mesh : meshes) {
        // This would render the mesh - simplified for now
        // In a full implementation, you'd set up VAOs and render the mesh
        std::cout << "Rendering model with " << mesh.vertices.size() << " vertices" << std::endl;
    }
}

bool EmbreeViewSimple::LoadModelFromFile(const std::string& filepath)
{
    std::cout << "Loading model for simple view: " << filepath << std::endl;
    
    if (m_ModelLoader.LoadModel(filepath)) {
        m_UseCustomModel = true;
        std::cout << "Successfully loaded model for simple view" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to load model for simple view" << std::endl;
        return false;
    }
}

std::string EmbreeViewSimple::GetModelInfo() const
{
    if (m_UseCustomModel) {
        return m_ModelLoader.GetModelInfo();
    } else {
        return "Procedural sphere (simple rendering)";
    }
}

void EmbreeViewSimple::ProcessMouseInput(float deltaTime)
{
    if (!m_Valid || !m_Camera) return;
    
    // Get current mouse position
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    
    // Check if mouse is over the view
    m_IsMouseOverView = (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
                        mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y);
    
    if (!m_IsMouseOverView) {
        m_IsRightMousePressed = false;
        m_IsLeftMousePressed = false;
        return;
    }
    
    // Check for Option key (Alt key on macOS)
    m_IsOptionPressed = ImGui::GetIO().KeyMods & ImGuiMod_Alt;
    
    // Handle right mouse button for orbit rotation
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        if (!m_IsRightMousePressed) {
            m_IsRightMousePressed = true;
            m_LastMousePos = glm::vec2(mousePos.x, mousePos.y);
        } else {
            // Calculate mouse delta
            glm::vec2 currentMousePos(mousePos.x, mousePos.y);
            glm::vec2 mouseDelta = currentMousePos - m_LastMousePos;
            
            // Update orbit angles
            m_OrbitYaw -= mouseDelta.x * 0.5f;
            m_OrbitPitch -= mouseDelta.y * 0.5f;
            
            // Constrain pitch
            if (m_OrbitPitch > 89.0f) m_OrbitPitch = 89.0f;
            if (m_OrbitPitch < -89.0f) m_OrbitPitch = -89.0f;
            
            // Update camera position
            UpdateOrbitCamera();
            m_LastMousePos = currentMousePos;
        }
    } else {
        m_IsRightMousePressed = false;
    }
    
    // Handle scroll wheel for zoom
    float scrollY = ImGui::GetIO().MouseWheel;
    if (scrollY != 0.0f) {
        m_OrbitDistance -= scrollY * 0.5f;
        if (m_OrbitDistance < 0.1f) m_OrbitDistance = 0.1f;
        if (m_OrbitDistance > 50.0f) m_OrbitDistance = 50.0f;
        UpdateOrbitCamera();
    }
}

bool EmbreeViewSimple::IsMouseOverView() const
{
    return m_IsMouseOverView;
}

void EmbreeViewSimple::FocusOnModel()
{
    if (!m_Camera) return;
    
    std::cout << "Focusing on model in simple view..." << std::endl;
    
    // Calculate model bounds (similar to PBRView)
    glm::vec3 modelCenter(0.0f, 0.0f, 0.0f);
    float modelRadius = 1.0f;
    
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        const auto& meshes = m_ModelLoader.GetMeshes();
        if (!meshes.empty()) {
            glm::vec3 minBounds(FLT_MAX);
            glm::vec3 maxBounds(-FLT_MAX);
            
            for (const auto& mesh : meshes) {
                for (const auto& vertex : mesh.vertices) {
                    minBounds = glm::min(minBounds, vertex.Position);
                    maxBounds = glm::max(maxBounds, vertex.Position);
                }
            }
            
            modelCenter = (minBounds + maxBounds) * 0.5f;
            glm::vec3 size = maxBounds - minBounds;
            modelRadius = glm::max(glm::max(size.x, size.y), size.z) * 0.5f;
        }
    }
    
    // Calculate optimal camera distance
    float fov = glm::radians(m_Camera->Zoom);
    float optimalDistance = modelRadius / std::sin(fov * 0.5f) * 1.2f;
    
    // Set camera position
    m_Camera->Position = modelCenter + glm::vec3(0.0f, 0.0f, optimalDistance);
    m_Camera->Front = glm::normalize(modelCenter - m_Camera->Position);
    m_Camera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Reset camera zoom
    m_Camera->Zoom = 45.0f;
}

void EmbreeViewSimple::ResetCameraView()
{
    if (!m_Camera) return;
    
    // Reset orbit camera to default position
    m_OrbitTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 4.0f;
    m_OrbitYaw = -90.0f;
    m_OrbitPitch = 0.0f;
    UpdateOrbitCamera();
    
    std::cout << "Camera view reset to default in simple view" << std::endl;
}

void EmbreeViewSimple::SyncWithPBRView(const PBRView* pbrView)
{
    if (!pbrView) return;
    
    // Sync camera position and orientation
    const Camera* pbrCamera = pbrView->GetCamera();
    if (m_Camera && pbrCamera) {
        m_Camera->Position = pbrCamera->Position;
        m_Camera->Front = pbrCamera->Front;
        m_Camera->Up = pbrCamera->Up;
        m_Camera->Zoom = pbrCamera->Zoom;
    }
    
    // Sync model data - reload the model if needed
    if (pbrView->IsUsingCustomModel() && pbrView->GetModelLoader().HasLoadedModel()) {
        // Get the model path and reload it
        std::string modelPath = pbrView->GetModelLoader().GetModelPath();
        if (!modelPath.empty()) {
            LoadModelFromFile(modelPath);
        }
    }
    
    // Sync orbit camera state
    m_OrbitTarget = pbrView->GetOrbitTarget();
    m_OrbitDistance = pbrView->GetOrbitDistance();
    m_OrbitYaw = pbrView->GetOrbitYaw();
    m_OrbitPitch = pbrView->GetOrbitPitch();
}

void EmbreeViewSimple::UpdateOrbitCamera()
{
    if (!m_Camera) return;
    
    // Calculate camera position based on orbit angles and distance
    float x = m_OrbitDistance * cos(glm::radians(m_OrbitPitch)) * cos(glm::radians(m_OrbitYaw));
    float y = m_OrbitDistance * sin(glm::radians(m_OrbitPitch));
    float z = m_OrbitDistance * cos(glm::radians(m_OrbitPitch)) * sin(glm::radians(m_OrbitYaw));
    
    m_Camera->Position = m_OrbitTarget + glm::vec3(x, y, z);
    
    // Update camera vectors to look at the target
    m_Camera->Front = glm::normalize(m_OrbitTarget - m_Camera->Position);
    m_Camera->Right = glm::normalize(glm::cross(m_Camera->Front, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_Camera->Up = glm::normalize(glm::cross(m_Camera->Right, m_Camera->Front));
}

void EmbreeViewSimple::SetOrbitTarget(const glm::vec3& target)
{
    m_OrbitTarget = target;
    UpdateOrbitCamera();
}

unsigned int EmbreeViewSimple::LoadTexture(const char *path)
{
    // This would be implemented if needed for texture mapping
    return 0;
}
