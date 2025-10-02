#include "EmbreeView.h"
#include "PBRView.h" // Include PBRView for full access to its members
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

EmbreeView::EmbreeView()
    : m_Framebuffer(0)
    , m_ColorTexture(0)
    , m_DepthStencilBuffer(0)
    , m_Size(800, 600)
    , m_Valid(false)
    , m_SizeChanged(false)
    , m_EmbreeDevice(nullptr)
    , m_EmbreeScene(nullptr)
    , m_EmbreeGeometry(nullptr)
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
    , m_SamplesPerPixel(1)
    , m_MaxDepth(3)
    , m_UsePathTracing(false)
    , m_LastRenderTime(0.0f)
    , m_RenderInterval(1.0f / 30.0f) // Limit to 30 FPS
    , m_NeedsRender(true)
    , m_AlbedoTexture(0)
    , m_RoughnessTexture(0)
    , m_MetallicTexture(0)
    , m_NormalTexture(0)
    , m_HeightTexture(0)
    , m_AOTexture(0)
    , m_DefaultGreyTexture(0)
{
}

EmbreeView::~EmbreeView()
{
    CleanupEmbree();
    DestroyFramebuffer();
}

bool EmbreeView::Initialize(int width, int height)
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
    
    // Setup Embree
    if (!SetupEmbree()) {
        std::cerr << "Failed to setup Embree" << std::endl;
        return false;
    }
    
    CreateFramebuffer();
    return m_Valid;
}

void EmbreeView::CreateFramebuffer()
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
        std::cerr << "EmbreeView Framebuffer not complete!" << std::endl;
        m_Valid = false;
        return;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Valid = true;
}

void EmbreeView::DestroyFramebuffer()
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

void EmbreeView::UpdateSize(int width, int height)
{
    if (width != static_cast<int>(m_Size.x) || height != static_cast<int>(m_Size.y))
    {
        m_Size = ImVec2(static_cast<float>(width), static_cast<float>(height));
        m_SizeChanged = true;
    }
}

void EmbreeView::SetRenderCallback(std::function<void()> callback)
{
    m_RenderCallback = callback;
}

bool EmbreeView::SetupEmbree()
{
    // Create Embree device
    m_EmbreeDevice = rtcNewDevice(nullptr);
    if (!m_EmbreeDevice) {
        std::cerr << "Failed to create Embree device" << std::endl;
        return false;
    }
    
    // Create Embree scene
    m_EmbreeScene = rtcNewScene(m_EmbreeDevice);
    if (!m_EmbreeScene) {
        std::cerr << "Failed to create Embree scene" << std::endl;
        return false;
    }
    
    // Set scene build quality
    rtcSetSceneBuildQuality(m_EmbreeScene, RTC_BUILD_QUALITY_HIGH);
    
    return true;
}

void EmbreeView::CleanupEmbree()
{
    if (m_EmbreeScene) {
        rtcReleaseScene(m_EmbreeScene);
        m_EmbreeScene = nullptr;
    }
    
    if (m_EmbreeDevice) {
        rtcReleaseDevice(m_EmbreeDevice);
        m_EmbreeDevice = nullptr;
    }
}

void EmbreeView::BuildScene()
{
    if (!m_EmbreeScene) return;
    
    // Clear existing geometry
    if (m_EmbreeGeometry) {
        rtcDetachGeometry(m_EmbreeScene, 0);
        rtcReleaseGeometry(m_EmbreeGeometry);
        m_EmbreeGeometry = nullptr;
    }
    
    // Create geometry based on current model
    if (m_UseCustomModel && m_ModelLoader.HasLoadedModel()) {
        const auto& meshes = m_ModelLoader.GetMeshes();
        if (!meshes.empty()) {
            const auto& mesh = meshes[0];
            
            // Create triangle mesh
            m_EmbreeGeometry = rtcNewGeometry(m_EmbreeDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
            
            // Set vertex buffer
            float* vertices = (float*)rtcSetNewGeometryBuffer(m_EmbreeGeometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), mesh.vertices.size());
            for (size_t i = 0; i < mesh.vertices.size(); ++i) {
                vertices[i * 3 + 0] = mesh.vertices[i].Position.x;
                vertices[i * 3 + 1] = mesh.vertices[i].Position.y;
                vertices[i * 3 + 2] = mesh.vertices[i].Position.z;
            }
            
            // Set index buffer
            unsigned int* indices = (unsigned int*)rtcSetNewGeometryBuffer(m_EmbreeGeometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), mesh.indices.size() / 3);
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                indices[i / 3 * 3 + 0] = mesh.indices[i];
                indices[i / 3 * 3 + 1] = mesh.indices[i + 1];
                indices[i / 3 * 3 + 2] = mesh.indices[i + 2];
            }
            
            rtcCommitGeometry(m_EmbreeGeometry);
            rtcAttachGeometry(m_EmbreeScene, m_EmbreeGeometry);
        }
    } else {
        // Create a default sphere geometry
        m_EmbreeGeometry = rtcNewGeometry(m_EmbreeDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
        
        // Generate sphere vertices
        const unsigned int X_SEGMENTS = 32;
        const unsigned int Y_SEGMENTS = 32;
        const float PI = 3.14159265359f;
        
        std::vector<glm::vec3> positions;
        std::vector<unsigned int> indices;
        
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
        
        // Set vertex buffer
        float* vertices = (float*)rtcSetNewGeometryBuffer(m_EmbreeGeometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), positions.size());
        for (size_t i = 0; i < positions.size(); ++i) {
            vertices[i * 3 + 0] = positions[i].x;
            vertices[i * 3 + 1] = positions[i].y;
            vertices[i * 3 + 2] = positions[i].z;
        }
        
        // Set index buffer
        unsigned int* indices_ptr = (unsigned int*)rtcSetNewGeometryBuffer(m_EmbreeGeometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), indices.size() / 3);
        for (size_t i = 0; i < indices.size(); i += 3) {
            indices_ptr[i / 3 * 3 + 0] = indices[i];
            indices_ptr[i / 3 * 3 + 1] = indices[i + 1];
            indices_ptr[i / 3 * 3 + 2] = indices[i + 2];
        }
        
        rtcCommitGeometry(m_EmbreeGeometry);
        rtcAttachGeometry(m_EmbreeScene, m_EmbreeGeometry);
        
        std::cout << "EmbreeView: Created sphere with " << positions.size() << " vertices and " << indices.size()/3 << " triangles" << std::endl;
        std::cout << "EmbreeView: First few sphere vertices: (" << positions[0].x << ", " << positions[0].y << ", " << positions[0].z << ")" << std::endl;
        std::cout << "EmbreeView: Sphere center should be at origin (0,0,0)" << std::endl;
    }
    
    // Commit scene
    rtcCommitScene(m_EmbreeScene);
    std::cout << "EmbreeView: Scene committed successfully" << std::endl;
    
    // Test ray: from camera position straight towards origin
    RTCRay testRay;
    testRay.org_x = m_Camera->Position.x;
    testRay.org_y = m_Camera->Position.y;
    testRay.org_z = m_Camera->Position.z;
    testRay.dir_x = 0.0f;
    testRay.dir_y = 0.0f;
    testRay.dir_z = 1.0f; // Towards origin
    testRay.tnear = 0.0f;
    testRay.tfar = FLT_MAX;
    testRay.mask = 0xFFFFFFFF;
    testRay.flags = 0;
    
    RTCRayHit testRayHit;
    testRayHit.ray = testRay;
    testRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    
    RTCIntersectArguments testArgs;
    rtcInitIntersectArguments(&testArgs);
    rtcIntersect1(m_EmbreeScene, &testRayHit, &testArgs);
    
    if (testRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        std::cout << "EmbreeView: Test ray HIT! Distance: " << testRayHit.ray.tfar << std::endl;
    } else {
        std::cout << "EmbreeView: Test ray MISSED!" << std::endl;
    }
}

void EmbreeView::Render(const char* windowTitle, bool* p_open)
{
    static int renderCounter = 0;
    if (renderCounter < 5) {
        std::cout << "EmbreeView::Render called " << renderCounter << " times" << std::endl;
        renderCounter++;
    }
    
    if (!m_Valid)
    {
        std::cout << "EmbreeView not valid!" << std::endl;
        return;
    }
    
    // Performance optimization: frame rate limiting
    float currentTime = ImGui::GetTime();
    if (!m_NeedsRender && (currentTime - m_LastRenderTime) < m_RenderInterval) {
        // Just display the existing texture without re-rendering
        ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(m_ColorTexture)), 
                     ImGui::GetContentRegionAvail(), 
                     ImVec2(0, 0), ImVec2(1, 1));
        return;
    }
    
    // Get the current ImGui window size
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    if (windowSize.x < 50.0f) windowSize.x = 50.0f;
    if (windowSize.y < 50.0f) windowSize.y = 50.0f;
    
    // Update size if needed
    UpdateSize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    
    // Process mouse input for zoom and rotation
    float deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;
    ProcessMouseInput(deltaTime);
    
    // Mark as needing render if camera moved
    if (deltaTime > 0.0f) {
        m_NeedsRender = true;
    }
    
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
    
    // Build scene if needed
    BuildScene();
    
    // Update orbit camera
    UpdateOrbitCamera();
    
    // Debug: Print camera info
    static int debugCounter = 0;
    if (debugCounter < 5) {
        std::cout << "EmbreeView Camera - Position: (" << m_Camera->Position.x << ", " << m_Camera->Position.y << ", " << m_Camera->Position.z << ")" << std::endl;
        std::cout << "EmbreeView Camera - Front: (" << m_Camera->Front.x << ", " << m_Camera->Front.y << ", " << m_Camera->Front.z << ")" << std::endl;
        std::cout << "EmbreeView Camera - Up: (" << m_Camera->Up.x << ", " << m_Camera->Up.y << ", " << m_Camera->Up.z << ")" << std::endl;
        std::cout << "EmbreeView Camera - Right: (" << m_Camera->Right.x << ", " << m_Camera->Right.y << ", " << m_Camera->Right.z << ")" << std::endl;
        std::cout << "EmbreeView Camera - Zoom: " << m_Camera->Zoom << std::endl;
        debugCounter++;
    }
    
    // Setup matrices
    m_Projection = glm::perspective(glm::radians(m_Camera->Zoom), m_Size.x / m_Size.y, 0.1f, 100.0f);
    m_View = m_Camera->GetViewMatrix();
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Ray trace the scene
    RayTrace();
    
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
                 ImVec2(0, 0), ImVec2(1, 1)); // No Y flip needed
    
    // Update render time and mark as not needing render
    m_LastRenderTime = currentTime;
    m_NeedsRender = false;
}

void EmbreeView::RayTrace()
{
    if (!m_EmbreeScene) return;
    
    // Update textures from node system
    UpdateTexturesFromNodes();
    
    // Get framebuffer data
    std::vector<unsigned char> pixels(static_cast<int>(m_Size.x) * static_cast<int>(m_Size.y) * 4);
    
    // Performance optimization: reduce resolution for real-time rendering
    int renderWidth = static_cast<int>(m_Size.x);
    int renderHeight = static_cast<int>(m_Size.y);
    
    // Scale down for performance (more aggressive scaling for better performance)
    if (renderWidth > 400) {
        renderWidth = 400;
        renderHeight = static_cast<int>(400.0f * m_Size.y / m_Size.x);
    }
    
    // Further scale down if samples are high
    if (m_SamplesPerPixel > 4) {
        renderWidth = renderWidth / 2;
        renderHeight = renderHeight / 2;
    }
    
    // Debug: Check if scene has geometry
    static bool debugScene = false;
    if (!debugScene) {
        std::cout << "EmbreeView: Scene built successfully" << std::endl;
        debugScene = true;
    }
    
    // Ray trace each pixel with anti-aliasing
    for (int y = 0; y < renderHeight; ++y) {
        for (int x = 0; x < renderWidth; ++x) {
            glm::vec3 totalColor(0.0f);
            
            // Anti-aliasing: sample multiple rays per pixel
            int samples = m_SamplesPerPixel;
            for (int s = 0; s < samples; ++s) {
                // Generate primary ray with jittering for anti-aliasing
                RTCRay ray;
                ray.org_x = m_Camera->Position.x;
                ray.org_y = m_Camera->Position.y;
                ray.org_z = m_Camera->Position.z;
                
                // Calculate ray direction with jittering
                float jitterX = (samples > 1) ? ((float)rand() / RAND_MAX - 0.5f) / renderWidth : 0.0f;
                float jitterY = (samples > 1) ? ((float)rand() / RAND_MAX - 0.5f) / renderHeight : 0.0f;
                
                // Convert pixel coordinates to NDC (-1 to 1)
                float u = (2.0f * (x + jitterX) / renderWidth) - 1.0f;
                float v = (2.0f * (y + jitterY) / renderHeight) - 1.0f;
                
                // Debug: Print UV coordinates for center pixels
                if (x == renderWidth/2 && y == renderHeight/2 && s == 0) {
                    std::cout << "Center pixel UV: (" << u << ", " << v << ")" << std::endl;
                }
                
                // Apply aspect ratio correction
                float aspectRatio = static_cast<float>(renderWidth) / static_cast<float>(renderHeight);
                u *= aspectRatio;
                
                // Apply FOV scaling
                float fov = glm::radians(m_Camera->Zoom);
                float tanHalfFov = tan(fov * 0.5f);
                u *= tanHalfFov;
                v *= tanHalfFov;
                
                // Calculate ray direction in camera space
                glm::vec3 rayDir = glm::normalize(m_Camera->Front + u * m_Camera->Right + v * m_Camera->Up);
                ray.dir_x = rayDir.x;
                ray.dir_y = rayDir.y;
                ray.dir_z = rayDir.z;
                
                // Debug: Print first few rays
                static int rayCounter = 0;
                if (rayCounter < 5) {
                    std::cout << "Ray " << rayCounter << ": Origin(" << ray.org_x << ", " << ray.org_y << ", " << ray.org_z << ") Dir(" << ray.dir_x << ", " << ray.dir_y << ", " << ray.dir_z << ")" << std::endl;
                    rayCounter++;
                }
                
                // Debug: Print center ray
                if (x == renderWidth/2 && y == renderHeight/2 && s == 0) {
                    std::cout << "Center ray: Origin(" << ray.org_x << ", " << ray.org_y << ", " << ray.org_z << ") Dir(" << ray.dir_x << ", " << ray.dir_y << ", " << ray.dir_z << ")" << std::endl;
                    
                    // Test this specific ray
                    RTCRayHit testCenterRayHit;
                    testCenterRayHit.ray = ray;
                    testCenterRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
                    
                    RTCIntersectArguments testCenterArgs;
                    rtcInitIntersectArguments(&testCenterArgs);
                    rtcIntersect1(m_EmbreeScene, &testCenterRayHit, &testCenterArgs);
                    
                    if (testCenterRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
                        std::cout << "Center ray HIT! Distance: " << testCenterRayHit.ray.tfar << std::endl;
                    } else {
                        std::cout << "Center ray MISSED!" << std::endl;
                    }
                }
                
                ray.tnear = 0.0f;
                ray.tfar = FLT_MAX;
                ray.mask = 0xFFFFFFFF;
                ray.flags = 0;
                
                // Trace ray
                glm::vec3 color = TraceRay(ray, 0);
                totalColor += color;
            }
            
            // Average the samples
            totalColor /= static_cast<float>(samples);
            
            // Tone mapping and gamma correction
            totalColor = totalColor / (totalColor + glm::vec3(1.0f)); // Reinhard tone mapping
            totalColor = glm::pow(totalColor, glm::vec3(1.0f / 2.2f)); // Gamma correction
            
            // Store pixel color (upscale if needed)
            int pixelIndex = (y * static_cast<int>(m_Size.x) + x) * 4;
            pixels[pixelIndex + 0] = static_cast<unsigned char>(glm::clamp(totalColor.r * 255.0f, 0.0f, 255.0f));
            pixels[pixelIndex + 1] = static_cast<unsigned char>(glm::clamp(totalColor.g * 255.0f, 0.0f, 255.0f));
            pixels[pixelIndex + 2] = static_cast<unsigned char>(glm::clamp(totalColor.b * 255.0f, 0.0f, 255.0f));
            pixels[pixelIndex + 3] = 255; // Alpha
        }
    }
    
    // Upload to texture
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

glm::vec3 EmbreeView::TraceRay(const RTCRay& ray, int depth)
{
    if (depth >= m_MaxDepth) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    
    RTCIntersectArguments args;
    rtcInitIntersectArguments(&args);
    
    RTCRayHit rayhit;
    rayhit.ray = ray;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    
    rtcIntersect1(m_EmbreeScene, &rayhit, &args);
    
    if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        // Debug: Count hits
        static int hitCount = 0;
        if (hitCount < 10) {
            std::cout << "Ray hit! Distance: " << rayhit.ray.tfar << std::endl;
            hitCount++;
        }
        // Hit something - advanced shading
        glm::vec3 hitPoint(
            ray.org_x + ray.dir_x * rayhit.ray.tfar,
            ray.org_y + ray.dir_y * rayhit.ray.tfar,
            ray.org_z + ray.dir_z * rayhit.ray.tfar
        );
        
        // Surface normal
        glm::vec3 normal(
            rayhit.hit.Ng_x,
            rayhit.hit.Ng_y,
            rayhit.hit.Ng_z
        );
        normal = glm::normalize(normal);
        
        // Calculate UV coordinates (simple sphere mapping for now)
        glm::vec2 uv = CalculateUV(hitPoint, normal);
        
        // Sample material properties from textures
        glm::vec3 albedo = GetMaterialAlbedo(uv);
        float metallic = GetMaterialMetallic(uv);
        float roughness = GetMaterialRoughness(uv);
        float specular = 0.04f;
        float ao = GetMaterialAO(uv);
        float height = GetMaterialHeight(uv);
        
        // Calculate tangent space for normal mapping
        glm::vec3 tangent = CalculateTangentSpace(normal, hitPoint);
        glm::vec3 bitangent = glm::cross(normal, tangent);
        
        // Apply height displacement
        if (height > 0.0f) {
            // Offset hit point along normal based on height map
            float displacementScale = 0.1f; // Adjust this for more/less displacement
            hitPoint += normal * height * displacementScale;
        }
        
        // Apply normal mapping
        normal = GetMaterialNormal(uv, normal, tangent, bitangent);
        
        // Lighting setup
        glm::vec3 lightPos = glm::vec3(2.0f, 2.0f, 2.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);
        
        // View direction
        glm::vec3 viewDir = glm::normalize(glm::vec3(ray.org_x, ray.org_y, ray.org_z) - hitPoint);
        
        // PBR shading
        glm::vec3 F0 = glm::mix(glm::vec3(specular), albedo, metallic);
        glm::vec3 F = F0 + (glm::vec3(1.0f) - F0) * std::pow(1.0f - glm::max(glm::dot(normal, viewDir), 0.0f), 5.0f);
        
        // Diffuse
        float NdotL = glm::max(glm::dot(normal, lightDir), 0.0f);
        glm::vec3 diffuse = albedo / glm::vec3(3.14159f) * (1.0f - metallic);
        
        // Specular (simplified Cook-Torrance)
        glm::vec3 halfDir = glm::normalize(lightDir + viewDir);
        float NdotH = glm::max(glm::dot(normal, halfDir), 0.0f);
        float NdotV = glm::max(glm::dot(normal, viewDir), 0.0f);
        float VdotH = glm::max(glm::dot(viewDir, halfDir), 0.0f);
        
        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
        float D = alpha2 / (3.14159f * denom * denom);
        
        float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
        float G1L = NdotL / (NdotL * (1.0f - k) + k);
        float G1V = NdotV / (NdotV * (1.0f - k) + k);
        float G = G1L * G1V;
        
        glm::vec3 specularColor = F * D * G / (4.0f * NdotL * NdotV + 0.001f);
        
        // Direct lighting
        glm::vec3 directLight = (diffuse + specularColor) * lightColor * NdotL;
        
        // Ambient lighting with AO
        glm::vec3 ambient = albedo * 0.1f * ao;
        
        // Reflection (if using path tracing)
        glm::vec3 reflection = glm::vec3(0.0f);
        if (m_UsePathTracing && depth < m_MaxDepth - 1) {
            RTCRay reflectRay;
            glm::vec3 reflectDir = glm::reflect(glm::vec3(ray.dir_x, ray.dir_y, ray.dir_z), normal);
            reflectRay.org_x = hitPoint.x + normal.x * 0.001f;
            reflectRay.org_y = hitPoint.y + normal.y * 0.001f;
            reflectRay.org_z = hitPoint.z + normal.z * 0.001f;
            reflectRay.dir_x = reflectDir.x;
            reflectRay.dir_y = reflectDir.y;
            reflectRay.dir_z = reflectDir.z;
            reflectRay.tnear = 0.001f;
            reflectRay.tfar = FLT_MAX;
            reflectRay.mask = 0xFFFFFFFF;
            reflectRay.flags = 0;
            
            reflection = TraceRay(reflectRay, depth + 1) * F;
        }
        
        return directLight + ambient + reflection;
    } else {
        // Miss - return environment color
        glm::vec3 rayDir(ray.dir_x, ray.dir_y, ray.dir_z);
        return SampleEnvironment(rayDir);
    }
}

glm::vec3 EmbreeView::SampleEnvironment(const glm::vec3& direction)
{
    // Enhanced sky gradient with sun
    float t = 0.5f * (direction.y + 1.0f);
    glm::vec3 skyColor = (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
    
    // Add sun
    glm::vec3 sunDir = glm::normalize(glm::vec3(0.3f, 0.8f, 0.2f));
    float sunDot = glm::max(0.0f, glm::dot(direction, sunDir));
    if (sunDot > 0.99f) {
        float sunIntensity = std::pow(sunDot, 64.0f);
        skyColor += glm::vec3(1.0f, 0.9f, 0.7f) * sunIntensity * 2.0f;
    }
    
    return skyColor;
}

bool EmbreeView::IntersectScene(const RTCRay& ray, RTCIntersectArguments& args)
{
    RTCRayHit rayhit;
    rayhit.ray = ray;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    
    rtcIntersect1(m_EmbreeScene, &rayhit, &args);
    
    return rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID;
}

void EmbreeView::UpdateTexturesFromNodes()
{
    // Get textures from output nodes
    unsigned int albedoTexture = NodeSystem::GetOutputTexture(ChannelType::Color);
    unsigned int roughnessTexture = NodeSystem::GetOutputTexture(ChannelType::Roughness);
    unsigned int metallicTexture = NodeSystem::GetOutputTexture(ChannelType::Metallic);
    unsigned int normalTexture = NodeSystem::GetOutputTexture(ChannelType::Normal);
    unsigned int heightTexture = NodeSystem::GetOutputTexture(ChannelType::Height);
    
    // Create default grey texture if needed
    if (m_DefaultGreyTexture == 0) {
        auto greyData = std::make_shared<TextureData>(1, 1, 3);
        greyData->data[0] = 128; // R
        greyData->data[1] = 128; // G
        greyData->data[2] = 128; // B
        m_DefaultGreyTexture = TextureGenerator::CreateOpenGLTexture(*greyData);
        m_TextureDataCache[m_DefaultGreyTexture] = greyData;
    }
    
    // Update material textures
    m_AlbedoTexture = (albedoTexture != 0) ? albedoTexture : m_DefaultGreyTexture;
    m_RoughnessTexture = (roughnessTexture != 0) ? roughnessTexture : m_DefaultGreyTexture;
    m_MetallicTexture = (metallicTexture != 0) ? metallicTexture : m_DefaultGreyTexture;
    m_NormalTexture = (normalTexture != 0) ? normalTexture : m_DefaultGreyTexture;
    m_HeightTexture = (heightTexture != 0) ? heightTexture : 0; // Height can be 0 (no displacement)
    m_AOTexture = m_DefaultGreyTexture; // Use default for AO
    
    // Cache texture data for CPU sampling
    if (albedoTexture != 0) {
        auto node = NodeSystem::GetNodeByOutputTexture(albedoTexture);
        if (node && node->GetTextureData()) {
            m_TextureDataCache[albedoTexture] = node->GetTextureData();
        }
    }
    if (roughnessTexture != 0) {
        auto node = NodeSystem::GetNodeByOutputTexture(roughnessTexture);
        if (node && node->GetTextureData()) {
            m_TextureDataCache[roughnessTexture] = node->GetTextureData();
        }
    }
    if (metallicTexture != 0) {
        auto node = NodeSystem::GetNodeByOutputTexture(metallicTexture);
        if (node && node->GetTextureData()) {
            m_TextureDataCache[metallicTexture] = node->GetTextureData();
        }
    }
    if (normalTexture != 0) {
        auto node = NodeSystem::GetNodeByOutputTexture(normalTexture);
        if (node && node->GetTextureData()) {
            m_TextureDataCache[normalTexture] = node->GetTextureData();
        }
    }
}

glm::vec3 EmbreeView::SampleTexture(unsigned int textureID, const glm::vec2& uv) const
{
    auto it = m_TextureDataCache.find(textureID);
    if (it == m_TextureDataCache.end() || !it->second) {
        return glm::vec3(0.5f, 0.5f, 0.5f); // Default grey
    }
    
    const auto& textureData = it->second;
    
    // Clamp UV coordinates
    float u = glm::clamp(uv.x, 0.0f, 1.0f);
    float v = glm::clamp(uv.y, 0.0f, 1.0f);
    
    // Convert to texture coordinates
    float x = u * (textureData->width - 1);
    float y = v * (textureData->height - 1);
    
    // Get integer and fractional parts for bilinear filtering
    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = glm::min(x0 + 1, textureData->width - 1);
    int y1 = glm::min(y0 + 1, textureData->height - 1);
    
    float fx = x - x0;
    float fy = y - y0;
    
    // Sample four corners
    auto samplePixel = [&](int px, int py) -> glm::vec3 {
        int pixelIndex = (py * textureData->width + px) * textureData->channels;
        if (textureData->channels >= 3) {
            return glm::vec3(
                textureData->data[pixelIndex] / 255.0f,
                textureData->data[pixelIndex + 1] / 255.0f,
                textureData->data[pixelIndex + 2] / 255.0f
            );
        } else if (textureData->channels == 1) {
            float value = textureData->data[pixelIndex] / 255.0f;
            return glm::vec3(value, value, value);
        }
        return glm::vec3(0.5f, 0.5f, 0.5f);
    };
    
    glm::vec3 c00 = samplePixel(x0, y0);
    glm::vec3 c10 = samplePixel(x1, y0);
    glm::vec3 c01 = samplePixel(x0, y1);
    glm::vec3 c11 = samplePixel(x1, y1);
    
    // Bilinear interpolation
    glm::vec3 c0 = c00 * (1.0f - fx) + c10 * fx;
    glm::vec3 c1 = c01 * (1.0f - fx) + c11 * fx;
    glm::vec3 result = c0 * (1.0f - fy) + c1 * fy;
    
    return result;
}

glm::vec3 EmbreeView::GetMaterialAlbedo(const glm::vec2& uv) const
{
    return SampleTexture(m_AlbedoTexture, uv);
}

float EmbreeView::GetMaterialRoughness(const glm::vec2& uv) const
{
    glm::vec3 roughness = SampleTexture(m_RoughnessTexture, uv);
    return roughness.r; // Use red channel for roughness
}

float EmbreeView::GetMaterialMetallic(const glm::vec2& uv) const
{
    glm::vec3 metallic = SampleTexture(m_MetallicTexture, uv);
    return metallic.r; // Use red channel for metallic
}

glm::vec3 EmbreeView::GetMaterialNormal(const glm::vec2& uv, const glm::vec3& worldNormal, const glm::vec3& tangent, const glm::vec3& bitangent) const
{
    if (m_NormalTexture == 0) {
        return worldNormal;
    }
    
    glm::vec3 normalMap = SampleTexture(m_NormalTexture, uv);
    // Convert from [0,1] to [-1,1]
    normalMap = normalMap * 2.0f - 1.0f;
    
    // Transform from tangent space to world space
    glm::mat3 TBN = glm::mat3(tangent, bitangent, worldNormal);
    return glm::normalize(TBN * normalMap);
}

float EmbreeView::GetMaterialHeight(const glm::vec2& uv) const
{
    if (m_HeightTexture == 0) {
        return 0.0f;
    }
    
    glm::vec3 height = SampleTexture(m_HeightTexture, uv);
    return height.r; // Use red channel for height
}

float EmbreeView::GetMaterialAO(const glm::vec2& uv) const
{
    glm::vec3 ao = SampleTexture(m_AOTexture, uv);
    return ao.r; // Use red channel for AO
}

glm::vec2 EmbreeView::CalculateUV(const glm::vec3& hitPoint, const glm::vec3& normal) const
{
    // Simple sphere UV mapping
    // Convert 3D point to UV coordinates
    float u = 0.5f + atan2(normal.z, normal.x) / (2.0f * 3.14159f);
    float v = 0.5f - asin(normal.y) / 3.14159f;
    
    return glm::vec2(u, v);
}

glm::vec3 EmbreeView::CalculateTangentSpace(const glm::vec3& normal, const glm::vec3& hitPoint) const
{
    // Calculate tangent and bitangent for normal mapping
    // For a sphere, we can use the partial derivatives of the UV mapping
    
    // Calculate tangent using UV derivatives
    glm::vec3 tangent = glm::vec3(
        -sin(atan2(normal.z, normal.x)),
        0.0f,
        cos(atan2(normal.z, normal.x))
    );
    
    // Calculate bitangent as cross product of normal and tangent
    glm::vec3 bitangent = glm::cross(normal, tangent);
    
    // Normalize
    tangent = glm::normalize(tangent);
    bitangent = glm::normalize(bitangent);
    
    return tangent; // Return tangent, bitangent can be calculated from normal and tangent
}

bool EmbreeView::LoadMeshFromOBJ(const std::string& filename)
{
    // This would be implemented similar to PBRView
    return false;
}

bool EmbreeView::LoadModelFromFile(const std::string& filepath)
{
    std::cout << "Loading model for Embree view: " << filepath << std::endl;
    
    if (m_ModelLoader.LoadModel(filepath)) {
        m_UseCustomModel = true;
        std::cout << "Successfully loaded model for Embree view" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to load model for Embree view" << std::endl;
        return false;
    }
}

std::string EmbreeView::GetModelInfo() const
{
    if (m_UseCustomModel) {
        return m_ModelLoader.GetModelInfo();
    } else {
        return "Procedural sphere (ray-traced)";
    }
}

void EmbreeView::ProcessMouseInput(float deltaTime)
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

bool EmbreeView::IsMouseOverView() const
{
    return m_IsMouseOverView;
}

void EmbreeView::FocusOnModel()
{
    if (!m_Camera) return;
    
    std::cout << "Focusing on model in Embree view..." << std::endl;
    
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

void EmbreeView::ResetCameraView()
{
    if (!m_Camera) return;
    
    // Reset orbit camera to default position
    m_OrbitTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 4.0f;
    m_OrbitYaw = -90.0f;
    m_OrbitPitch = 0.0f;
    UpdateOrbitCamera();
    
    std::cout << "Camera view reset to default in Embree view" << std::endl;
}

void EmbreeView::SyncWithPBRView(const PBRView* pbrView)
{
    if (!pbrView) return;
    
    // Sync camera position and orientation
    const Camera* pbrCamera = pbrView->GetCamera();
    if (m_Camera && pbrCamera) {
        m_Camera->Position = pbrCamera->Position;
        m_Camera->Front = pbrCamera->Front;
        m_Camera->Up = pbrCamera->Up;
        m_Camera->Right = pbrCamera->Right;
        m_Camera->Yaw = pbrCamera->Yaw;
        m_Camera->Pitch = pbrCamera->Pitch;
        m_Camera->Zoom = pbrCamera->Zoom;
        
        // Update camera vectors to ensure consistency
        m_Camera->updateCameraVectors();
        
        // Debug: Print camera info
        static int syncCounter = 0;
        if (syncCounter < 5) {
            std::cout << "EmbreeView Camera - Position: (" << m_Camera->Position.x << ", " << m_Camera->Position.y << ", " << m_Camera->Position.z << ")" << std::endl;
            std::cout << "EmbreeView Camera - Front: (" << m_Camera->Front.x << ", " << m_Camera->Front.y << ", " << m_Camera->Front.z << ")" << std::endl;
            std::cout << "EmbreeView Camera - Up: (" << m_Camera->Up.x << ", " << m_Camera->Up.y << ", " << m_Camera->Up.z << ")" << std::endl;
            std::cout << "EmbreeView Camera - Right: (" << m_Camera->Right.x << ", " << m_Camera->Right.y << ", " << m_Camera->Right.z << ")" << std::endl;
            std::cout << "EmbreeView Camera - Zoom: " << m_Camera->Zoom << std::endl;
            syncCounter++;
        }
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

void EmbreeView::UpdateOrbitCamera()
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

void EmbreeView::SetOrbitTarget(const glm::vec3& target)
{
    m_OrbitTarget = target;
    UpdateOrbitCamera();
}

unsigned int EmbreeView::LoadTexture(const char *path)
{
    // This would be implemented if needed for texture mapping in ray tracing
    return 0;
}
