#include "PBRView.h"
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "learnopengl/filesystem.h"

void PBRView::SetupShaders()
{
    try {
        std::cout << "Setting up PBR shaders..." << std::endl;
        
        // Check if shader files exist
        std::string simpleVS = FileSystem::getPath("shaders/simple_model.vs");
        std::string simpleFS = FileSystem::getPath("shaders/simple_model.fs");
        std::cout << "Simple shader paths: " << simpleVS << " and " << simpleFS << std::endl;
        
        m_PBRShader = std::make_unique<Shader>(FileSystem::getPath("shaders/2.2.2.pbr.vs").c_str(), FileSystem::getPath("shaders/2.2.2.pbr.fs").c_str());
        std::cout << "PBR shader loaded successfully" << std::endl;
        m_BackgroundShader = std::make_unique<Shader>(FileSystem::getPath("shaders/2.2.2.background.vs").c_str(), FileSystem::getPath("shaders/2.2.2.background.fs").c_str());
        std::cout << "Background shader loaded successfully" << std::endl;
        m_SimpleShader = std::make_unique<Shader>(simpleVS.c_str(), simpleFS.c_str());
        std::cout << "Simple shader loaded successfully" << std::endl;
        
        // Verify simple shader is working
        if (m_SimpleShader) {
            GLint linkStatus;
            glGetProgramiv(m_SimpleShader->ID, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                std::cerr << "Simple shader failed to link!" << std::endl;
                GLchar infoLog[1024];
                glGetProgramInfoLog(m_SimpleShader->ID, 1024, NULL, infoLog);
                std::cerr << "Link error: " << infoLog << std::endl;
            } else {
                std::cout << "Simple shader linked successfully" << std::endl;
            }
        }
        
        m_EquirectangularToCubemapShader = std::make_unique<Shader>(FileSystem::getPath("shaders/2.2.2.cubemap.vs").c_str(), FileSystem::getPath("shaders/2.2.2.equirectangular_to_cubemap.fs").c_str());
        m_IrradianceShader = std::make_unique<Shader>(FileSystem::getPath("shaders/2.2.2.cubemap.vs").c_str(), FileSystem::getPath("shaders/2.2.2.irradiance_convolution.fs").c_str());
        m_PrefilterShader = std::make_unique<Shader>(FileSystem::getPath("shaders/2.2.2.cubemap.vs").c_str(), FileSystem::getPath("shaders/2.2.2.prefilter.fs").c_str());
        m_BRDFShader = std::make_unique<Shader>(FileSystem::getPath("shaders/2.2.2.brdf.vs").c_str(), FileSystem::getPath("shaders/2.2.2.brdf.fs").c_str());
        
        // Set texture units
        m_PBRShader->use();
        m_PBRShader->setInt("irradianceMap", 0);
        m_PBRShader->setInt("prefilterMap", 1);
        m_PBRShader->setInt("brdfLUT", 2);
        m_PBRShader->setInt("albedoMap", 3);
        m_PBRShader->setInt("normalMap", 4);
        m_PBRShader->setInt("metallicMap", 5);
        m_PBRShader->setInt("roughnessMap", 6);
        m_PBRShader->setInt("aoMap", 7);
        m_PBRShader->setInt("heightMap", 8);
        
        m_BackgroundShader->use();
        m_BackgroundShader->setInt("environmentMap", 0);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to setup shaders: " << e.what() << std::endl;
    }
}

void PBRView::SetupIBL()
{
    std::cout << "Setting up IBL..." << std::endl;
    // Load HDR environment map
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *data = stbi_loadf(FileSystem::getPath("resources/textures/hdr/newport_loft.hdr").c_str(), &width, &height, &nrComponents, 0);
    
    if (data)
    {
        std::cout << "HDR image loaded successfully: " << width << "x" << height << std::endl;
        glGenTextures(1, &m_HDRTexture);
        glBindTexture(GL_TEXTURE_2D, m_HDRTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
        return;
    }
    
    // Setup capture framebuffer
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    
    // Convert HDR to cubemap
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    // Create environment cubemap
    glGenTextures(1, &m_EnvCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Convert HDR to cubemap
    m_EquirectangularToCubemapShader->use();
    m_EquirectangularToCubemapShader->setInt("equirectangularMap", 0);
    m_EquirectangularToCubemapShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_HDRTexture);
    
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        m_EquirectangularToCubemapShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_EnvCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Generate mipmaps
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    // Create irradiance map
    glGenTextures(1, &m_IrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    
    m_IrradianceShader->use();
    m_IrradianceShader->setInt("environmentMap", 0);
    m_IrradianceShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    
    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        m_IrradianceShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Create prefilter map
    glGenTextures(1, &m_PrefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    m_PrefilterShader->use();
    m_PrefilterShader->setInt("environmentMap", 0);
    m_PrefilterShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        m_PrefilterShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            m_PrefilterShader->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Create BRDF LUT
    glGenTextures(1, &m_BRDFLUTTexture);
    glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUTTexture, 0);
    
    glViewport(0, 0, 512, 512);
    m_BRDFShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Cleanup
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
    
    std::cout << "IBL setup completed successfully" << std::endl;
}

void PBRView::RenderSphere()
{
    // If we have a loaded model, render it instead of the sphere
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        static int renderCount = 0;
        if (renderCount % 60 == 0) { // Print every 60 frames
            std::cout << "Rendering custom model with " << m_ModelLoader.GetMeshes().size() << " meshes" << std::endl;
            
            // Calculate actual model bounds
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
                
                glm::vec3 center = (minBounds + maxBounds) * 0.5f;
                glm::vec3 size = maxBounds - minBounds;
                
                std::cout << "Model bounds: center=(" << center.x << "," << center.y << "," << center.z 
                          << ") size=(" << size.x << "," << size.y << "," << size.z << ")" << std::endl;
            }
        }
        renderCount++;
        
        // Clear any previous OpenGL errors
        while (glGetError() != GL_NO_ERROR);
        
        // Use PBR shader for custom models (it's more robust)
        if (m_PBRShader) {
            m_PBRShader->use();
            
            // Set basic uniforms for PBR shader
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
            
            // Bind material textures from node system (same as sphere)
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
            
            // Check for errors after setting uniforms
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cout << "OpenGL error after setting PBR uniforms: " << error << std::endl;
            }
        }
        
        const auto& meshes = m_ModelLoader.GetMeshes();
        for (const auto& mesh : meshes) {
            // Check for OpenGL errors before drawing
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cout << "OpenGL error before mesh draw: " << error << std::endl;
            }
            
            // Debug mesh info
            static int debugCount = 0;
            if (debugCount % 60 == 0) { // Print every 60 frames
                std::cout << "Drawing mesh with " << mesh.vertices.size() << " vertices and " << mesh.indices.size() << " indices" << std::endl;
                std::cout << "Mesh VAO: " << mesh.VAO << ", VBO: " << mesh.VBO << ", EBO: " << mesh.EBO << std::endl;
            }
            debugCount++;
            
            // Instead of using the mesh's VAO, recreate it in the current context
            DrawMeshInCurrentContext(mesh);
            
            // Check for OpenGL errors after drawing
            error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cout << "OpenGL error after mesh draw: " << error << std::endl;
            }
        }
        return;
    }
    
    // If we have custom mesh data, render it instead of the sphere
    if (m_UseCustomMesh && !m_MeshPositions.empty()) {
        static int renderCount = 0;
        if (renderCount % 60 == 0) { // Print every 60 frames
            std::cout << "Rendering custom mesh with " << m_MeshPositions.size() 
                      << " vertices and " << m_MeshIndices.size() / 3 << " triangles" << std::endl;
        }
        renderCount++;
        
        // Create or update VAO for custom mesh
        if (m_SphereVAO == 0) {
            glGenVertexArrays(1, &m_SphereVAO);
            glGenBuffers(1, &m_SphereVBO);
            glGenBuffers(1, &m_SphereEBO);
        }
        
        // Always regenerate buffers for custom mesh data
        glBindVertexArray(m_SphereVAO);
        
        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_SphereVBO);
        glBufferData(GL_ARRAY_BUFFER, m_MeshPositions.size() * sizeof(glm::vec3), 
                     m_MeshPositions.data(), GL_STATIC_DRAW);
        
        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_SphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MeshIndices.size() * sizeof(unsigned int), 
                     m_MeshIndices.data(), GL_STATIC_DRAW);
        
        // Set vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        
        // Upload UV data if available
        if (!m_MeshUVs.empty()) {
            glGenBuffers(1, &m_SphereUVBO);
            glBindBuffer(GL_ARRAY_BUFFER, m_SphereUVBO);
            glBufferData(GL_ARRAY_BUFFER, m_MeshUVs.size() * sizeof(glm::vec2), 
                         m_MeshUVs.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        }
        
        // Upload normal data if available
        if (!m_MeshNormals.empty()) {
            glGenBuffers(1, &m_SphereNormalBO);
            glBindBuffer(GL_ARRAY_BUFFER, m_SphereNormalBO);
            glBufferData(GL_ARRAY_BUFFER, m_MeshNormals.size() * sizeof(glm::vec3), 
                         m_MeshNormals.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        }
        
        // Draw the mesh
        glDrawElements(GL_TRIANGLES, m_MeshIndices.size(), GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
        return;
    }
    
    if (m_SphereVAO == 0)
    {
        glGenVertexArrays(1, &m_SphereVAO);
        
        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        
        // Use custom mesh data if available, otherwise generate procedural sphere
        if (m_UseCustomMesh && !m_MeshPositions.empty()) {
            positions = m_MeshPositions;
            uv = m_MeshUVs;
            normals = m_MeshNormals;
            indices = m_MeshIndices;
            m_SphereIndexCount = indices.size();
        } else {
        
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
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
        
        // Generate proper triangular faces instead of triangle strips
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
        m_SphereIndexCount = static_cast<GLsizei>(indices.size());
        }
        
        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(m_SphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }
    
    glBindVertexArray(m_SphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, m_SphereIndexCount, GL_UNSIGNED_INT, 0);
}

void PBRView::RenderCube()
{
    if (m_CubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f
        };
        glGenVertexArrays(1, &m_CubeVAO);
        unsigned int cubeVBO;
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(m_CubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(m_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void PBRView::RenderQuad()
{
    if (m_QuadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &m_QuadVAO);
        unsigned int quadVBO;
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(m_QuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int PBRView::LoadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}

void PBRView::DrawMeshInCurrentContext(const Mesh& mesh) {
    // Create temporary VAO, VBO, and EBO in the current context
    GLuint tempVAO, tempVBO, tempEBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);
    glGenBuffers(1, &tempEBO);
    
    // Bind VAO
    glBindVertexArray(tempVAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);
    
    // Set vertex attributes (same as in Mesh constructor)
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    
    // Draw the mesh
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
    
    // Clean up temporary buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &tempVAO);
    glDeleteBuffers(1, &tempVBO);
    glDeleteBuffers(1, &tempEBO);
}
