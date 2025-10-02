#include "PBRView.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include "learnopengl/filesystem.h"
#include "NodeSystem.h"
#include "RemeshManager.h"

PBRView::PBRView()
    : m_Framebuffer(0)
    , m_ColorTexture(0)
    , m_DepthStencilBuffer(0)
    , m_Size(800, 600)
    , m_Valid(false)
    , m_SizeChanged(false)
    , m_EnvCubemap(0)
    , m_IrradianceMap(0)
    , m_PrefilterMap(0)
    , m_BRDFLUTTexture(0)
    , m_HDRTexture(0)
    , m_SphereVAO(0)
    , m_SphereVBO(0)
    , m_SphereEBO(0)
    , m_SphereUVBO(0)
    , m_SphereNormalBO(0)
    , m_CubeVAO(0)
    , m_QuadVAO(0)
    , m_SphereIndexCount(0)
    , m_AlbedoTexture(0)
    , m_NormalTexture(0)
    , m_MetallicTexture(0)
    , m_RoughnessTexture(0)
    , m_AOTexture(0)
    , m_HeightTexture(0)
    , m_DisplacementIntensity(0.1f)
    , m_IBLSetup(false)
    , m_DefaultGreyTexture(0)
    , m_UseCustomMesh(false)
    , m_UseCustomModel(false)
    , m_HasOriginalModel(false)
    , m_UseOriginalCustomMesh(false)
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
    // Initialize light positions and colors
    m_LightPositions[0] = glm::vec3(-10.0f,  10.0f, 10.0f);
    m_LightPositions[1] = glm::vec3( 10.0f,  10.0f, 10.0f);
    m_LightPositions[2] = glm::vec3(-10.0f, -10.0f, 10.0f);
    m_LightPositions[3] = glm::vec3( 10.0f, -10.0f, 10.0f);
    
    m_LightColors[0] = glm::vec3(300.0f, 300.0f, 300.0f);
    m_LightColors[1] = glm::vec3(300.0f, 300.0f, 300.0f);
    m_LightColors[2] = glm::vec3(300.0f, 300.0f, 300.0f);
    m_LightColors[3] = glm::vec3(300.0f, 300.0f, 300.0f);
}

PBRView::~PBRView()
{
    DestroyFramebuffer();
    
    if (m_SphereVAO) glDeleteVertexArrays(1, &m_SphereVAO);
    if (m_SphereVBO) glDeleteBuffers(1, &m_SphereVBO);
    if (m_SphereEBO) glDeleteBuffers(1, &m_SphereEBO);
    if (m_SphereUVBO) glDeleteBuffers(1, &m_SphereUVBO);
    if (m_SphereNormalBO) glDeleteBuffers(1, &m_SphereNormalBO);
    if (m_CubeVAO) glDeleteVertexArrays(1, &m_CubeVAO);
    if (m_QuadVAO) glDeleteVertexArrays(1, &m_QuadVAO);
    
    if (m_EnvCubemap) glDeleteTextures(1, &m_EnvCubemap);
    if (m_IrradianceMap) glDeleteTextures(1, &m_IrradianceMap);
    if (m_PrefilterMap) glDeleteTextures(1, &m_PrefilterMap);
    if (m_BRDFLUTTexture) glDeleteTextures(1, &m_BRDFLUTTexture);
    if (m_HDRTexture) glDeleteTextures(1, &m_HDRTexture);
}

bool PBRView::Initialize(int width, int height)
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
    
    // Initialize texture manager
    m_TextureManager = std::make_unique<TextureManager>();
    m_TextureManager->setRealtimeDirectory(FileSystem::getPath("resources/textures/realtime"));
    m_TextureManager->startWatching();
    
    // Setup shaders
    SetupShaders();
    
    // IBL will be set up on first render
    
    // Load default material textures
    m_AlbedoTexture = LoadTexture(FileSystem::getPath("resources/textures/pbr/gold/albedo.png").c_str());
    m_NormalTexture = LoadTexture(FileSystem::getPath("resources/textures/pbr/gold/normal.png").c_str());
    m_MetallicTexture = LoadTexture(FileSystem::getPath("resources/textures/pbr/gold/metallic.png").c_str());
    m_RoughnessTexture = LoadTexture(FileSystem::getPath("resources/textures/pbr/gold/roughness.png").c_str());
    m_AOTexture = LoadTexture(FileSystem::getPath("resources/textures/pbr/gold/ao.png").c_str());
    
    CreateFramebuffer();
    return m_Valid;
}

void PBRView::CreateFramebuffer()
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
        std::cerr << "PBRView Framebuffer not complete!" << std::endl;
        m_Valid = false;
        return;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Valid = true;
}

void PBRView::DestroyFramebuffer()
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

void PBRView::UpdateSize(int width, int height)
{
    if (width != static_cast<int>(m_Size.x) || height != static_cast<int>(m_Size.y))
    {
        m_Size = ImVec2(static_cast<float>(width), static_cast<float>(height));
        m_SizeChanged = true;
    }
}

void PBRView::SetRenderCallback(std::function<void()> callback)
{
    m_RenderCallback = callback;
}

void PBRView::SetMaterialTextures(unsigned int albedo, unsigned int normal, 
                                 unsigned int metallic, unsigned int roughness, 
                                 unsigned int ao, unsigned int height)
{
    m_AlbedoTexture = albedo;
    m_NormalTexture = normal;
    m_MetallicTexture = metallic;
    m_RoughnessTexture = roughness;
    m_AOTexture = ao;
    m_HeightTexture = height;
}

void PBRView::SetDisplacementIntensity(float intensity)
{
    m_DisplacementIntensity = intensity;
}

void PBRView::Render(const char* windowTitle, bool* p_open)
{
    if (!m_Valid)
    {
        std::cout << "PBRView not valid!" << std::endl;
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
    
    // Set up OpenGL state (same as original program)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    // Disable culling to see both sides of the model
    glDisable(GL_CULL_FACE);
    
    // Clear the framebuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup IBL on first render
    if (!m_IBLSetup)
    {
        SetupIBL();
        m_IBLSetup = true;
    }
    
    // Update textures
    if (m_TextureManager)
    {
        m_TextureManager->updateTextures();
    }
    
    // Update PBR textures from node system
    UpdateTexturesFromNodes();
    
    // Setup matrices
    m_Projection = glm::perspective(glm::radians(m_Camera->Zoom), m_Size.x / m_Size.y, 0.1f, 100.0f);
    m_View = m_Camera->GetViewMatrix();
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Apply uniform scaling to all models for consistent sizing
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        // Use uniform scaling to maintain proper proportions
        m_Model = glm::scale(m_Model, glm::vec3(1.0f, 1.0f, 1.0f)); // Uniform 1x scaling
    }
    // Note: Model scaling is now handled in ModelLoader for consistent sizing
    
    // Debug output for model rendering
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        static int debugCounter = 0;
        if (debugCounter % 60 == 0) { // Print every 60 frames (about once per second)
            std::cout << "=== MODEL RENDERING DEBUG ===" << std::endl;
            std::cout << "Camera position: (" << m_Camera->Position.x << ", " << m_Camera->Position.y << ", " << m_Camera->Position.z << ")" << std::endl;
            std::cout << "Camera zoom: " << m_Camera->Zoom << std::endl;
            std::cout << "Model matrix scale: (" << m_Model[0][0] << ", " << m_Model[1][1] << ", " << m_Model[2][2] << ")" << std::endl;
            std::cout << "Model matrix translation: (" << m_Model[3][0] << ", " << m_Model[3][1] << ", " << m_Model[3][2] << ")" << std::endl;
            std::cout << "Model info: " << m_ModelLoader.GetModelInfo() << std::endl;
            std::cout << "Number of meshes: " << m_ModelLoader.GetMeshes().size() << std::endl;
            std::cout << "=============================" << std::endl;
        }
        debugCounter++;
    }
    
    // Render PBR sphere
    if (m_PBRShader)
    {
        m_PBRShader->use();
        m_PBRShader->setMat4("projection", m_Projection);
        m_PBRShader->setMat4("view", m_View);
        m_PBRShader->setMat4("model", m_Model);
        m_PBRShader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m_Model))));
        m_PBRShader->setVec3("camPos", m_Camera->Position);
        
        // Set up lights
        for (unsigned int i = 0; i < 4; ++i)
        {
            m_PBRShader->setVec3("lightPositions[" + std::to_string(i) + "]", m_LightPositions[i]);
            m_PBRShader->setVec3("lightColors[" + std::to_string(i) + "]", m_LightColors[i]);
        }
        
        // Bind IBL textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTexture);
        
        // Bind material textures
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_AlbedoTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, m_MetallicTexture);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, m_RoughnessTexture);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, m_AOTexture);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, m_HeightTexture);
        
        m_PBRShader->setBool("useDisplacement", m_HeightTexture != 0);
        m_PBRShader->setFloat("displacementScale", m_DisplacementIntensity);
        
        RenderSphere();
    }
    
    // Render background/skybox
    if (m_BackgroundShader)
    {
        m_BackgroundShader->use();
        m_BackgroundShader->setMat4("projection", m_Projection);
        m_BackgroundShader->setMat4("view", m_View);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
        
        RenderCube();
    }
    
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

unsigned int PBRView::CreateDefaultGreyTexture()
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Create a 1x1 mid-grey texture
    unsigned char greyData[3] = {128, 128, 128}; // Mid-grey RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, greyData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return textureID;
}

void PBRView::UpdateTexturesFromNodes()
{
    // Get textures from output nodes
    unsigned int albedoTexture = NodeSystem::GetOutputTexture(ChannelType::Color);
    unsigned int roughnessTexture = NodeSystem::GetOutputTexture(ChannelType::Roughness);
    unsigned int metallicTexture = NodeSystem::GetOutputTexture(ChannelType::Metallic);
    unsigned int normalTexture = NodeSystem::GetOutputTexture(ChannelType::Normal);
    unsigned int heightTexture = NodeSystem::GetOutputTexture(ChannelType::Height);
    
    // Use default grey texture for blank channels
    if (m_DefaultGreyTexture == 0) {
        m_DefaultGreyTexture = CreateDefaultGreyTexture();
    }
    
    // Update material textures
    m_AlbedoTexture = (albedoTexture != 0) ? albedoTexture : m_DefaultGreyTexture;
    m_RoughnessTexture = (roughnessTexture != 0) ? roughnessTexture : m_DefaultGreyTexture;
    m_MetallicTexture = (metallicTexture != 0) ? metallicTexture : m_DefaultGreyTexture;
    m_NormalTexture = (normalTexture != 0) ? normalTexture : m_DefaultGreyTexture;
    m_HeightTexture = (heightTexture != 0) ? heightTexture : 0; // Height can be 0 (no displacement)
    
    // For AO, we'll use the default grey for now (no AO output node in current system)
    m_AOTexture = m_DefaultGreyTexture;
}

bool PBRView::LoadMeshFromOBJ(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filename << std::endl;
        return false;
    }
    
    // Clear existing mesh data
    m_MeshPositions.clear();
    m_MeshUVs.clear();
    m_MeshNormals.clear();
    m_MeshIndices.clear();
    
    std::vector<glm::vec3> tempPositions;
    std::vector<glm::vec2> tempUVs;
    std::vector<glm::vec3> tempNormals;
    std::vector<std::string> faceIndices;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            tempPositions.push_back(position);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            tempUVs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (prefix == "f") {
            // Read all vertices in the face (support both triangles and quads)
            std::string vertex;
            std::vector<std::string> faceVertices;
            
            while (iss >> vertex) {
                faceVertices.push_back(vertex);
            }
            
            if (faceVertices.size() >= 3) {
                if (faceVertices.size() == 3) {
                    // Triangle face
                    faceIndices.push_back(faceVertices[0]);
                    faceIndices.push_back(faceVertices[1]);
                    faceIndices.push_back(faceVertices[2]);
                } else if (faceVertices.size() == 4) {
                    // Quad face - split into two triangles
                    // Triangle 1: v0, v1, v2
                    faceIndices.push_back(faceVertices[0]);
                    faceIndices.push_back(faceVertices[1]);
                    faceIndices.push_back(faceVertices[2]);
                    
                    // Triangle 2: v0, v2, v3
                    faceIndices.push_back(faceVertices[0]);
                    faceIndices.push_back(faceVertices[2]);
                    faceIndices.push_back(faceVertices[3]);
                } else {
                    // Polygon with more than 4 vertices - triangulate using fan method
                    for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
                        faceIndices.push_back(faceVertices[0]);
                        faceIndices.push_back(faceVertices[i]);
                        faceIndices.push_back(faceVertices[i + 1]);
                    }
                }
            }
        }
    }
    
    // Process face indices - FIXED: Proper OBJ face parsing
    for (const auto& face : faceIndices) {
        std::istringstream faceStream(face);
        std::string vertexStr;
        std::vector<std::string> vertexStrings;
        
        // Split face into individual vertex references
        while (std::getline(faceStream, vertexStr, ' ')) {
            if (!vertexStr.empty()) {
                vertexStrings.push_back(vertexStr);
            }
        }
        
        // Process each vertex reference in the face
        std::vector<int> faceIndices;
        for (const auto& vertexRef : vertexStrings) {
            std::istringstream vertexStream(vertexRef);
            std::string indexStr;
            std::vector<int> indices;
            
            // Split vertex reference (e.g., "1/2/3" or "1//3" or "1")
            while (std::getline(vertexStream, indexStr, '/')) {
                if (!indexStr.empty()) {
                    indices.push_back(std::stoi(indexStr) - 1); // OBJ indices are 1-based
                } else {
                    indices.push_back(-1); // Missing component
                }
            }
            
            if (indices.size() >= 1 && indices[0] >= 0 && indices[0] < tempPositions.size()) {
                // Find or create vertex with these attributes
                int vertexIndex = -1;
                
                // Check if this exact vertex combination already exists
                for (size_t i = 0; i < m_MeshPositions.size(); i++) {
                    bool matches = true;
                    
                    // Check position
                    if (m_MeshPositions[i] != tempPositions[indices[0]]) {
                        matches = false;
                    }
                    
                    // Check UV if available
                    if (matches && indices.size() >= 2 && indices[1] >= 0) {
                        glm::vec2 expectedUV = (indices[1] < tempUVs.size()) ? tempUVs[indices[1]] : glm::vec2(0.0f, 0.0f);
                        if (m_MeshUVs[i] != expectedUV) {
                            matches = false;
                        }
                    }
                    
                    // Check normal if available
                    if (matches && indices.size() >= 3 && indices[2] >= 0) {
                        glm::vec3 expectedNormal;
                        if (indices[2] < tempNormals.size()) {
                            expectedNormal = tempNormals[indices[2]];
                        } else {
                            expectedNormal = glm::normalize(tempPositions[indices[0]]);
                        }
                        if (m_MeshNormals[i] != expectedNormal) {
                            matches = false;
                        }
                    }
                    
                    if (matches) {
                        vertexIndex = i;
                        break;
                    }
                }
                
                // If vertex doesn't exist, create it
                if (vertexIndex == -1) {
                    m_MeshPositions.push_back(tempPositions[indices[0]]);
                    
                    // Add UV
                    if (indices.size() >= 2 && indices[1] >= 0 && indices[1] < tempUVs.size()) {
                        m_MeshUVs.push_back(tempUVs[indices[1]]);
                    } else {
                        m_MeshUVs.push_back(glm::vec2(0.0f, 0.0f));
                    }
                    
                    // Add normal
                    if (indices.size() >= 3 && indices[2] >= 0 && indices[2] < tempNormals.size()) {
                        m_MeshNormals.push_back(tempNormals[indices[2]]);
                    } else {
                        m_MeshNormals.push_back(glm::normalize(tempPositions[indices[0]]));
                    }
                    
                    vertexIndex = m_MeshPositions.size() - 1;
                }
                
                faceIndices.push_back(vertexIndex);
            }
        }
        
        // Add triangular faces (triangulate if necessary)
        if (faceIndices.size() >= 3) {
            if (faceIndices.size() == 3) {
                // Triangle - add as is
                m_MeshIndices.push_back(faceIndices[0]);
                m_MeshIndices.push_back(faceIndices[1]);
                m_MeshIndices.push_back(faceIndices[2]);
            } else if (faceIndices.size() == 4) {
                // Quad - triangulate into 2 triangles
                // First triangle: 0, 1, 2
                m_MeshIndices.push_back(faceIndices[0]);
                m_MeshIndices.push_back(faceIndices[1]);
                m_MeshIndices.push_back(faceIndices[2]);
                // Second triangle: 0, 2, 3
                m_MeshIndices.push_back(faceIndices[0]);
                m_MeshIndices.push_back(faceIndices[2]);
                m_MeshIndices.push_back(faceIndices[3]);
            } else {
                // Polygon with more than 4 vertices - fan triangulation
                for (size_t i = 1; i < faceIndices.size() - 1; i++) {
                    m_MeshIndices.push_back(faceIndices[0]);
                    m_MeshIndices.push_back(faceIndices[i]);
                    m_MeshIndices.push_back(faceIndices[i + 1]);
                }
            }
        }
    }
    
    // Normalize mesh to unit sphere (same as original procedural sphere)
    if (!m_MeshPositions.empty()) {
        // Find bounding box
        glm::vec3 minPos = m_MeshPositions[0];
        glm::vec3 maxPos = m_MeshPositions[0];
        
        for (const auto& pos : m_MeshPositions) {
            minPos = glm::min(minPos, pos);
            maxPos = glm::max(maxPos, pos);
        }
        
        // Calculate center and scale
        glm::vec3 center = (minPos + maxPos) * 0.5f;
        glm::vec3 size = maxPos - minPos;
        float maxSize = glm::max(glm::max(size.x, size.y), size.z);
        float scale = 2.0f / maxSize; // Normalize to unit sphere (radius = 1)
        
        // Apply normalization
        for (auto& pos : m_MeshPositions) {
            pos = (pos - center) * scale;
        }
        
        // Recalculate normals for the normalized positions
        for (size_t i = 0; i < m_MeshPositions.size(); ++i) {
            m_MeshNormals[i] = glm::normalize(m_MeshPositions[i]);
        }
        
        std::cout << "Normalized mesh: center=" << center.x << "," << center.y << "," << center.z 
                  << " scale=" << scale << std::endl;
    }
    
    std::cout << "Loaded mesh from " << filename << ": " 
              << m_MeshPositions.size() << " vertices, " 
              << m_MeshIndices.size() / 3 << " triangles" << std::endl;
    std::cout << "Face indices processed: " << faceIndices.size() << " total face vertices" << std::endl;
    
    m_UseCustomMesh = true;
    return true;
}

void PBRView::ReloadSphereGeometry()
{
    // QuadriFlow functionality removed - reset to default sphere geometry
    std::cout << "Reloading sphere geometry..." << std::endl;
    
    // Clear any custom mesh data and reset to default sphere
    m_MeshPositions.clear();
    m_MeshUVs.clear();
    m_MeshNormals.clear();
    m_MeshIndices.clear();
    m_UseCustomMesh = false;
    
    // Recreate the sphere VAO with default geometry
    if (m_SphereVAO != 0) {
        glDeleteVertexArrays(1, &m_SphereVAO);
        m_SphereVAO = 0;
    }
    
    std::cout << "Reset to default sphere geometry" << std::endl;
}

void PBRView::ProcessMouseInput(float deltaTime)
{
    if (!m_Valid || !m_Camera) return;
    
    // Get current mouse position
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    
    // Check if mouse is over the PBR view
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
            
            // Update orbit angles (inverted for natural left/right movement)
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
    
    // Handle left mouse button for orbit rotation (alternative to right mouse)
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (!m_IsLeftMousePressed) {
            m_IsLeftMousePressed = true;
            m_LastMousePos = glm::vec2(mousePos.x, mousePos.y);
        } else {
            // Calculate mouse delta
            glm::vec2 currentMousePos(mousePos.x, mousePos.y);
            glm::vec2 mouseDelta = currentMousePos - m_LastMousePos;
            
            // Update orbit angles (inverted for natural left/right movement)
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
        m_IsLeftMousePressed = false;
    }
    
    // Handle scroll wheel for zoom (orbit distance)
    float scrollY = ImGui::GetIO().MouseWheel;
    if (scrollY != 0.0f) {
        m_OrbitDistance -= scrollY * 0.5f;
        if (m_OrbitDistance < 0.1f) m_OrbitDistance = 0.1f;
        if (m_OrbitDistance > 50.0f) m_OrbitDistance = 50.0f;
        UpdateOrbitCamera();
    }
    
    // Handle Option + Right Mouse for zoom (alternative to scroll)
    if (m_IsOptionPressed && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        if (!m_IsRightMousePressed) {
            m_IsRightMousePressed = true;
            m_LastMousePos = glm::vec2(mousePos.x, mousePos.y);
        } else {
            // Calculate mouse delta for zoom
            glm::vec2 currentMousePos(mousePos.x, mousePos.y);
            glm::vec2 mouseDelta = currentMousePos - m_LastMousePos;
            
            // Use vertical mouse movement for zoom
            float zoomDelta = -mouseDelta.y * 0.1f; // Scale factor for zoom sensitivity
            m_Camera->ProcessMouseScroll(zoomDelta);
            m_LastMousePos = currentMousePos;
        }
    }
    
    // Handle F key to focus and frame the current model
    if (ImGui::IsKeyPressed(ImGuiKey_F) && m_IsMouseOverView) {
        FocusOnModel();
    }
    
    // Handle R key to reset camera view
    if (ImGui::IsKeyPressed(ImGuiKey_R) && m_IsMouseOverView) {
        ResetCameraView();
    }
    
    // Handle E key to export procedural sphere to OBJ
    if (ImGui::IsKeyPressed(ImGuiKey_E) && m_IsMouseOverView) {
        std::string desktopPath = "/Users/thomasmakryniotis/Desktop/procedural_sphere.obj";
        if (ExportProceduralSphereToOBJ(desktopPath)) {
            std::cout << "Procedural sphere exported to Desktop!" << std::endl;
        } else {
            std::cerr << "Failed to export procedural sphere!" << std::endl;
        }
    }
}

bool PBRView::IsMouseOverView() const
{
    return m_IsMouseOverView;
}

void PBRView::RevertToOriginalMesh()
{
    std::cout << "Reverting to original procedural sphere..." << std::endl;
    
    // Clear custom mesh data
    m_MeshPositions.clear();
    m_MeshUVs.clear();
    m_MeshNormals.clear();
    m_MeshIndices.clear();
    m_UseCustomMesh = false;
    
    // Clear loaded model
    m_ModelLoader.ClearModel();
    m_UseCustomModel = false;
    
    // Force regeneration of ALL geometry by clearing VAOs
    if (m_SphereVAO != 0) {
        glDeleteVertexArrays(1, &m_SphereVAO);
        m_SphereVAO = 0;
    }
    if (m_CubeVAO != 0) {
        glDeleteVertexArrays(1, &m_CubeVAO);
        m_CubeVAO = 0;
    }
    if (m_QuadVAO != 0) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        m_QuadVAO = 0;
    }
    
    std::cout << "Reverted to original procedural sphere" << std::endl;
}

bool PBRView::LoadModelFromFile(const std::string& filepath)
{
    std::cout << "=== LOADING 3D MODEL ===" << std::endl;
    std::cout << "File path: " << filepath << std::endl;
    
    // Clear any existing model first
    m_ModelLoader.ClearModel();
    
    if (m_ModelLoader.LoadModel(filepath)) {
        m_UseCustomModel = true;
        m_UseCustomMesh = false; // Clear any previous custom mesh data
        
        // Clear ALL cached geometry to force complete regeneration
        if (m_SphereVAO != 0) {
            glDeleteVertexArrays(1, &m_SphereVAO);
            m_SphereVAO = 0;
        }
        if (m_CubeVAO != 0) {
            glDeleteVertexArrays(1, &m_CubeVAO);
            m_CubeVAO = 0;
        }
        if (m_QuadVAO != 0) {
            glDeleteVertexArrays(1, &m_QuadVAO);
            m_QuadVAO = 0;
        }
        
        // Clear custom mesh data
        m_MeshPositions.clear();
        m_MeshUVs.clear();
        m_MeshNormals.clear();
        m_MeshIndices.clear();
        
        std::cout << "Successfully loaded model: " << m_ModelLoader.GetModelInfo() << std::endl;
        std::cout << "Model will be rendered in next frame" << std::endl;
        std::cout << "=========================" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to load model: " << filepath << std::endl;
        std::cerr << "=========================" << std::endl;
        return false;
    }
}

std::string PBRView::GetModelInfo() const
{
    if (m_UseCustomModel) {
        return m_ModelLoader.GetModelInfo();
    } else if (m_UseCustomMesh) {
        return "Custom mesh (" + std::to_string(m_MeshPositions.size()) + " vertices)";
    } else {
        return "Procedural sphere";
    }
}

void PBRView::FocusOnModel()
{
    if (!m_Camera) return;
    
    std::cout << "Focusing on model..." << std::endl;
    
    // Calculate model bounds
    glm::vec3 modelCenter(0.0f, 0.0f, 0.0f);
    float modelRadius = 1.0f; // Default sphere radius
    
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        // Get model bounds from the loaded model
        const auto& meshes = m_ModelLoader.GetMeshes();
        if (!meshes.empty()) {
            // Calculate bounding box of all meshes
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
            
            // Apply the same scaling as the model matrix
            modelRadius *= 1.0f; // Account for uniform scaling
            
            std::cout << "Model bounds: center=(" << modelCenter.x << "," << modelCenter.y << "," << modelCenter.z 
                      << ") radius=" << modelRadius << std::endl;
        }
    } else if (m_UseCustomMesh && !m_MeshPositions.empty()) {
        // Calculate bounds from custom mesh data
        glm::vec3 minBounds = m_MeshPositions[0];
        glm::vec3 maxBounds = m_MeshPositions[0];
        
        for (const auto& pos : m_MeshPositions) {
            minBounds = glm::min(minBounds, pos);
            maxBounds = glm::max(maxBounds, pos);
        }
        
        modelCenter = (minBounds + maxBounds) * 0.5f;
        glm::vec3 size = maxBounds - minBounds;
        modelRadius = glm::max(glm::max(size.x, size.y), size.z) * 0.5f;
    }
    
    // Calculate optimal camera distance to frame the model
    // We want the model to fill about 60% of the viewport
    float fov = glm::radians(m_Camera->Zoom);
    float optimalDistance = modelRadius / std::sin(fov * 0.5f) * 1.2f; // 1.2x for some padding
    
    // Set camera position to look at the model center
    m_Camera->Position = modelCenter + glm::vec3(0.0f, 0.0f, optimalDistance);
    m_Camera->Front = glm::normalize(modelCenter - m_Camera->Position);
    m_Camera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Reset camera zoom to default
    m_Camera->Zoom = 45.0f;
    
    std::cout << "Camera repositioned to: (" << m_Camera->Position.x << "," << m_Camera->Position.y << "," << m_Camera->Position.z << ")" << std::endl;
    std::cout << "Model should now be visible and framed!" << std::endl;
}

void PBRView::ResetCameraView()
{
    if (!m_Camera) return;
    
    // Reset orbit camera to default position
    m_OrbitTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 4.0f;
    m_OrbitYaw = -90.0f;
    m_OrbitPitch = 0.0f;
    UpdateOrbitCamera();
    
    std::cout << "Camera view reset to default" << std::endl;
}

bool PBRView::ExportProceduralSphereToOBJ(const std::string& filepath)
{
    std::cout << "Exporting procedural sphere to: " << filepath << std::endl;
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    // Generate procedural sphere geometry (same as in RenderSphere)
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    
    // Generate vertices
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            
            positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }
    
    // Generate indices - create proper triangular faces instead of triangle strips
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x)
        {
            // Calculate vertex indices for the quad
            unsigned int v0 = y * (X_SEGMENTS + 1) + x;           // Bottom-left
            unsigned int v1 = v0 + 1;                             // Bottom-right
            unsigned int v2 = (y + 1) * (X_SEGMENTS + 1) + x;     // Top-left
            unsigned int v3 = v2 + 1;                             // Top-right
            
            // First triangle (v0, v2, v1)
            indices.push_back(v0);
            indices.push_back(v2);
            indices.push_back(v1);
            
            // Second triangle (v1, v2, v3)
            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);
        }
    }
    
    // Write OBJ file
    file << "# Procedural Sphere Export" << std::endl;
    file << "# Generated by Quadriflow" << std::endl;
    file << "# Vertices: " << positions.size() << std::endl;
    file << "# Faces: " << indices.size() / 3 << std::endl;
    file << std::endl;
    
    // Write vertices
    for (const auto& pos : positions) {
        file << "v " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    }
    
    // Write texture coordinates
    for (const auto& texCoord : uv) {
        file << "vt " << texCoord.x << " " << texCoord.y << std::endl;
    }
    
    // Write normals
    for (const auto& normal : normals) {
        file << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
    }
    
    // Write faces
    file << std::endl;
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 < indices.size()) {
            unsigned int v1 = indices[i] + 1;     // OBJ indices are 1-based
            unsigned int v2 = indices[i + 1] + 1;
            unsigned int v3 = indices[i + 2] + 1;
            unsigned int vt1 = indices[i] + 1;
            unsigned int vt2 = indices[i + 1] + 1;
            unsigned int vt3 = indices[i + 2] + 1;
            unsigned int vn1 = indices[i] + 1;
            unsigned int vn2 = indices[i + 1] + 1;
            unsigned int vn3 = indices[i + 2] + 1;
            
            file << "f " << v1 << "/" << vt1 << "/" << vn1 
                 << " " << v2 << "/" << vt2 << "/" << vn2 
                 << " " << v3 << "/" << vt3 << "/" << vn3 << std::endl;
        }
    }
    
    file.close();
    
    std::cout << "Successfully exported procedural sphere to: " << filepath << std::endl;
    std::cout << "Vertices: " << positions.size() << ", Faces: " << indices.size() / 3 << std::endl;
    
    return true;
}

bool PBRView::ExportCustomMeshToOBJ(const std::string& filepath)
{
    if (m_MeshPositions.empty() || m_MeshIndices.empty()) {
        std::cerr << "No custom mesh data to export" << std::endl;
        return false;
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "Exporting custom mesh to: " << filepath << std::endl;
    
    // Write OBJ header
    file << "# Custom Mesh Export (Remeshed Model)" << std::endl;
    file << "# Generated by Quadriflow" << std::endl;
    file << "# Vertices: " << m_MeshPositions.size() << std::endl;
    file << "# Faces: " << m_MeshIndices.size() / 3 << std::endl;
    file << std::endl;
    
    // Write vertices
    for (const auto& pos : m_MeshPositions) {
        file << "v " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    }
    
    file << std::endl;
    
    // Write texture coordinates if available
    if (!m_MeshUVs.empty() && m_MeshUVs.size() == m_MeshPositions.size()) {
        for (const auto& uv : m_MeshUVs) {
            file << "vt " << uv.x << " " << uv.y << std::endl;
        }
        file << std::endl;
    }
    
    // Write normals if available
    if (!m_MeshNormals.empty() && m_MeshNormals.size() == m_MeshPositions.size()) {
        for (const auto& normal : m_MeshNormals) {
            file << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
        }
        file << std::endl;
    }
    
    // Write faces (OBJ uses 1-based indexing)
    bool hasUVs = (!m_MeshUVs.empty() && m_MeshUVs.size() == m_MeshPositions.size());
    bool hasNormals = (!m_MeshNormals.empty() && m_MeshNormals.size() == m_MeshPositions.size());
    
    for (size_t i = 0; i < m_MeshIndices.size(); i += 3) {
        if (i + 2 < m_MeshIndices.size()) {
            unsigned int v1 = m_MeshIndices[i] + 1;     // Convert to 1-based
            unsigned int v2 = m_MeshIndices[i + 1] + 1;
            unsigned int v3 = m_MeshIndices[i + 2] + 1;
            
            if (hasUVs && hasNormals) {
                file << "f " << v1 << "/" << v1 << "/" << v1 
                     << " " << v2 << "/" << v2 << "/" << v2 
                     << " " << v3 << "/" << v3 << "/" << v3 << std::endl;
            } else if (hasUVs) {
                file << "f " << v1 << "/" << v1 
                     << " " << v2 << "/" << v2 
                     << " " << v3 << "/" << v3 << std::endl;
            } else if (hasNormals) {
                file << "f " << v1 << "//" << v1 
                     << " " << v2 << "//" << v2 
                     << " " << v3 << "//" << v3 << std::endl;
            } else {
                file << "f " << v1 << " " << v2 << " " << v3 << std::endl;
            }
        }
    }
    
    file.close();
    
    std::cout << "Successfully exported custom mesh!" << std::endl;
    std::cout << "Vertices: " << m_MeshPositions.size() << ", Faces: " << m_MeshIndices.size() / 3 << std::endl;
    
    return true;
}

bool PBRView::ExportCurrentModelToOBJ(const std::string& filepath)
{
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        // Export the loaded 3D model with TRIANGLES for Quadriflow (not quads!)
        std::cout << "Exporting loaded model with triangles for Quadriflow..." << std::endl;
        return m_ModelLoader.ExportToOBJ(filepath);
    } else if (m_UseCustomMesh && !m_MeshPositions.empty()) {
        // Export the custom mesh data (e.g., remeshed model)
        std::cout << "Exporting custom mesh data (remeshed model)..." << std::endl;
        return ExportCustomMeshToOBJ(filepath);
    } else {
        // Export the procedural sphere
        std::cout << "Exporting procedural sphere..." << std::endl;
        return ExportProceduralSphereToOBJ(filepath);
    }
}

void PBRView::UpdateOrbitCamera()
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

void PBRView::SetOrbitTarget(const glm::vec3& target)
{
    m_OrbitTarget = target;
    UpdateOrbitCamera();
}

void PBRView::SaveCurrentModelState()
{
    std::cout << "Saving current model state for revert functionality..." << std::endl;
    
    // Clear previous original state
    m_OriginalModelPath.clear();
    m_OriginalMeshPositions.clear();
    m_OriginalMeshUVs.clear();
    m_OriginalMeshNormals.clear();
    m_OriginalMeshIndices.clear();
    
    // Save current model state
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        // Save loaded model path (we'll reload it later)
        m_OriginalModelPath = m_ModelLoader.GetModelPath();
        m_HasOriginalModel = true;
        m_UseOriginalCustomMesh = false;
        std::cout << "Saved loaded model state: " << m_OriginalModelPath << std::endl;
    } else if (m_UseCustomMesh) {
        // Save custom mesh data
        m_OriginalMeshPositions = m_MeshPositions;
        m_OriginalMeshUVs = m_MeshUVs;
        m_OriginalMeshNormals = m_MeshNormals;
        m_OriginalMeshIndices = m_MeshIndices;
        m_HasOriginalModel = true;
        m_UseOriginalCustomMesh = true;
        std::cout << "Saved custom mesh state" << std::endl;
    } else {
        // Save procedural sphere state by generating it and saving the data
        m_HasOriginalModel = true;
        m_UseOriginalCustomMesh = true;
        
        // Generate procedural sphere data to save
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        
        m_OriginalMeshPositions.clear();
        m_OriginalMeshUVs.clear();
        m_OriginalMeshNormals.clear();
        m_OriginalMeshIndices.clear();
        
        // Generate vertices
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                
                m_OriginalMeshPositions.push_back(glm::vec3(xPos, yPos, zPos));
                m_OriginalMeshUVs.push_back(glm::vec2(xSegment, ySegment));
                m_OriginalMeshNormals.push_back(glm::vec3(xPos, yPos, zPos));
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
                m_OriginalMeshIndices.push_back(v0);
                m_OriginalMeshIndices.push_back(v2);
                m_OriginalMeshIndices.push_back(v1);
                
                // Second triangle
                m_OriginalMeshIndices.push_back(v1);
                m_OriginalMeshIndices.push_back(v2);
                m_OriginalMeshIndices.push_back(v3);
            }
        }
        
        std::cout << "Saved procedural sphere state" << std::endl;
    }
}

void PBRView::RevertToOriginalModel()
{
    if (!m_HasOriginalModel) {
        std::cout << "No original model state saved, reverting to procedural sphere" << std::endl;
        RevertToOriginalMesh();
        return;
    }
    
    std::cout << "Reverting to original model..." << std::endl;
    
    // Clear current state
    m_MeshPositions.clear();
    m_MeshUVs.clear();
    m_MeshNormals.clear();
    m_MeshIndices.clear();
    m_ModelLoader.ClearModel();
    m_UseCustomMesh = false;
    m_UseCustomModel = false;
    
    // Restore original state
    if (m_UseOriginalCustomMesh) {
        // Restore custom mesh data
        m_MeshPositions = m_OriginalMeshPositions;
        m_MeshUVs = m_OriginalMeshUVs;
        m_MeshNormals = m_OriginalMeshNormals;
        m_MeshIndices = m_OriginalMeshIndices;
        m_UseCustomMesh = true;
        std::cout << "Restored custom mesh data" << std::endl;
    } else if (!m_OriginalModelPath.empty()) {
        // Restore loaded model by reloading from the ORIGINAL raw file in mesh_ops folder
        std::string originalRawModelPath = m_ModelLoader.GetOriginalRawModelPath();
        if (!originalRawModelPath.empty() && std::filesystem::exists(originalRawModelPath)) {
            if (LoadModelFromFile(originalRawModelPath)) {
                std::cout << "Restored original model from mesh_ops: " << originalRawModelPath << std::endl;
            } else {
                std::cerr << "Failed to restore original model from mesh_ops: " << originalRawModelPath << std::endl;
                std::cout << "Reverting to procedural sphere instead" << std::endl;
                RevertToOriginalMesh();
            }
        } else {
            std::cerr << "Original raw model file not found in mesh_ops: " << originalRawModelPath << std::endl;
            std::cout << "Reverting to procedural sphere instead" << std::endl;
            RevertToOriginalMesh();
        }
    } else {
        // Restore procedural sphere (no data to restore)
        std::cout << "Restored procedural sphere" << std::endl;
    }
    
    // Force regeneration of VAO
    if (m_SphereVAO != 0) {
        glDeleteVertexArrays(1, &m_SphereVAO);
        m_SphereVAO = 0;
    }
    
    std::cout << "Successfully reverted to original model" << std::endl;
}

bool PBRView::RemeshCurrentModel(const RemeshSettings& settings) {
    std::cout << "=== REMESHING CURRENT MODEL ===" << std::endl;
    
    // Save current model state for revert functionality
    SaveCurrentModelState();
    
    // Use raw model file from mesh_ops folder for remeshing
    std::string rawModelPath = m_ModelLoader.GetRawModelPath();
    if (rawModelPath.empty()) {
        std::cerr << "No raw model file available for remeshing!" << std::endl;
        return false;
    }
    
    // Create output path in mesh_ops folder
    std::filesystem::path rawPath(rawModelPath);
    std::string outputFileName = rawPath.stem().string() + "_remeshed" + rawPath.extension().string();
    std::string outputPath = "mesh_ops/" + outputFileName;
    
    std::cout << "Using raw model file: " << rawModelPath << std::endl;
    std::cout << "Output will be saved to: " << outputPath << std::endl;
    
    // Try Quadriflow remesh using RemeshManager for proper error handling
    RemeshManager remeshManager;
    RemeshSettings actualSettings = settings;
    
    bool quadriflowSuccess = remeshManager.QuadriflowRemesh(rawModelPath, outputPath, actualSettings);
    
    if (!quadriflowSuccess) {
        std::cerr << "Quadriflow remesh failed: " << remeshManager.GetLastError() << std::endl;
        
        // For very complex models, try with reduced target count
        if (actualSettings.targetQuads > 4000) {
            std::cout << "Retrying with reduced target count (4000 quads)..." << std::endl;
            actualSettings.targetQuads = 4000;
            quadriflowSuccess = remeshManager.QuadriflowRemesh(rawModelPath, outputPath, actualSettings);
            
            if (!quadriflowSuccess && actualSettings.targetQuads > 2000) {
                std::cout << "Retrying with even lower target count (2000 quads)..." << std::endl;
                actualSettings.targetQuads = 2000;
                quadriflowSuccess = remeshManager.QuadriflowRemesh(rawModelPath, outputPath, actualSettings);
            }
        }
        
        if (!quadriflowSuccess) {
            std::cerr << "All Quadriflow attempts failed. Final error: " << remeshManager.GetLastError() << std::endl;
            std::cerr << "The input mesh is likely too complex for Quadriflow to handle." << std::endl;
            std::cerr << "Try using a simpler model or pre-simplifying the mesh." << std::endl;
            
            return false;
        }
    }
    
    std::cout << "Quadriflow completed successfully, loading result..." << std::endl;
    
    // Load the remeshed result using ModelLoader with quad-to-triangle conversion
    if (!m_ModelLoader.LoadModel(outputPath)) {
        std::cerr << "Failed to load remeshed result from: " << outputPath << std::endl;
        return false;
    }
    
    // Post-process the loaded mesh to add normals and UVs
    std::cout << "Post-processing remeshed result..." << std::endl;
    m_ModelLoader.PostProcessMesh();
    
    // Verify the loaded mesh has proper triangle data
    const auto& meshes = m_ModelLoader.GetMeshes();
    if (!meshes.empty()) {
        const auto& mesh = meshes[0];
        std::cout << "Loaded Quadriflow result: " << mesh.vertices.size() 
                  << " vertices, " << mesh.indices.size() / 3 << " triangles" << std::endl;
        
        // Validate the mesh data
        bool hasValidData = true;
        for (const auto& vertex : mesh.vertices) {
            if (std::isnan(vertex.Position.x) || std::isnan(vertex.Position.y) || std::isnan(vertex.Position.z) ||
                std::isinf(vertex.Position.x) || std::isinf(vertex.Position.y) || std::isinf(vertex.Position.z)) {
                hasValidData = false;
                break;
            }
        }
        
        if (!hasValidData) {
            std::cerr << "Error: Loaded mesh contains invalid vertex data (NaN/inf values)" << std::endl;
            return false;
        }
        
        // Check if we have a reasonable number of triangles
        if (mesh.indices.size() / 3 < 100) {
            std::cout << "Warning: Low triangle count (" << mesh.indices.size() / 3 << "), but proceeding..." << std::endl;
        }
    } else {
        std::cerr << "Error: No meshes loaded from Quadriflow output" << std::endl;
        return false;
    }
    
    // Switch to using the loaded model
    m_UseCustomModel = true;
    m_UseCustomMesh = false;
    
    // Force regeneration of VAO
    if (m_SphereVAO != 0) {
        glDeleteVertexArrays(1, &m_SphereVAO);
        m_SphereVAO = 0;
    }
    
    std::cout << "Remeshing completed successfully!" << std::endl;
    std::cout << "Final target: " << actualSettings.targetQuads << " quads" << std::endl;
    std::cout << "Result saved to: " << outputPath << std::endl;
    std::cout << "Result loaded and displayed in 3D view" << std::endl;
    std::cout << "===============================" << std::endl;
    
    return true;
}


bool PBRView::QuadriflowCurrentModel() {
    std::cout << "=== QUADRIFLOW QUADRANGULATION ===" << std::endl;

    // Save current model state for revert functionality
    SaveCurrentModelState();

    // Export current model to temporary OBJ file
    std::string inputPath = "temp_input.obj";
    std::string outputPath = "temp_quadriflow_output.obj";
    
    if (!ExportCurrentModelToOBJ(inputPath)) {
        std::cerr << "Failed to export current model for Quadriflow processing" << std::endl;
        return false;
    }
    
    std::cout << "Exported current model to: " << inputPath << std::endl;

    // Run Quadriflow executable
    std::string quadriflowPath = "/Users/thomasmakryniotis/Desktop/Quadriflow_Test/QuadriFlow/build/quadriflow";
    std::string command = quadriflowPath + " -i " + inputPath + " -o " + outputPath + " -f 8000";
    
    std::cout << "Running Quadriflow: " << command << std::endl;
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Quadriflow executable failed with code: " << result << std::endl;
        return false;
    }
    
    // Check if output file was created
    std::ifstream testFile(outputPath);
    if (!testFile.good()) {
        std::cerr << "Quadriflow output file not created: " << outputPath << std::endl;
        return false;
    }
    testFile.close();
    
    std::cout << "Quadriflow completed successfully, loading result..." << std::endl;

    // Load the quadriflow result using ModelLoader
    if (!m_ModelLoader.LoadModel(outputPath)) {
        std::cerr << "Failed to load Quadriflow result" << std::endl;
        return false;
    }

    // Switch to using the loaded model
    m_UseCustomModel = true;
    m_UseCustomMesh = false;

    // Force regeneration of VAO
    if (m_SphereVAO != 0) {
        glDeleteVertexArrays(1, &m_SphereVAO);
        m_SphereVAO = 0;
    }

    // Clean up temporary files
    std::remove(inputPath.c_str());
    // Keep output file for inspection: std::remove(outputPath.c_str());

    std::cout << "Quadriflow quadrangulation completed successfully!" << std::endl;
    std::cout << "Result loaded and displayed in 3D view" << std::endl;
    std::cout << "===============================" << std::endl;

    return true;
}
