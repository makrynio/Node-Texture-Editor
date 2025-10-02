#include <learnopengl/texture_manager.h>
#include <stb_image.h>
#include <iostream>
#include <ctime>
#include <string>
#include <fstream>

TextureManager::TextureManager() : running(false) {
    realtimeDir = "resources/textures/realtime";
    updateFile = realtimeDir + "/UPDATE";
    displacementIntensity = 10.0f; // Default displacement intensity
}

TextureManager::~TextureManager() {
    stopWatching();
}

void TextureManager::setRealtimeDirectory(const std::string& dir) {
    realtimeDir = dir;
    updateFile = dir + "/UPDATE";
    
    // Register realtime textures
    registerTexture("realtime_albedo", realtimeDir + "/albedo.png");
    registerTexture("realtime_normal", realtimeDir + "/normal.png");
    registerTexture("realtime_metallic", realtimeDir + "/metallic.png");
    registerTexture("realtime_roughness", realtimeDir + "/roughness.png");
    registerTexture("realtime_ao", realtimeDir + "/ao.png");
    registerTexture("realtime_height", realtimeDir + "/height.png");
}

bool TextureManager::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::chrono::system_clock::time_point TextureManager::getFileModTime(const std::string& path) {
    if (!fileExists(path)) {
        return std::chrono::system_clock::time_point::min();
    }
    
    try {
        auto ftime = std::filesystem::last_write_time(path);
        
        // Convert file_time_type to system_clock::time_point safely
        // This is a more robust cross-platform conversion
        auto file_time_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            ftime.time_since_epoch()
        ).count();
        
        auto system_time = std::chrono::system_clock::from_time_t(
            static_cast<std::time_t>(file_time_seconds)
        );
        
        return system_time;
    } catch (const std::exception& e) {
        std::cout << "Warning: Failed to get file modification time for " << path 
                  << ": " << e.what() << std::endl;
        return std::chrono::system_clock::now();
    }
}

unsigned int TextureManager::loadTextureFromFile(const std::string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            std::cout << "Unsupported texture format: " << nrComponents << " components in " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ")" << std::endl;
    } else {
        std::cout << "Failed to load texture: " << path << " - " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return 0;
    }

    return textureID;
}

unsigned int TextureManager::registerTexture(const std::string& name, const std::string& path) {
    std::lock_guard<std::mutex> lock(texturesMutex);
    
    // If texture already exists, update path
    if (textures.find(name) != textures.end()) {
        textures[name].path = path;
        textures[name].needsReload = true;
        return textures[name].id;
    }
    
    // Create new texture
    TextureInfo info;
    info.path = path;
    info.lastModified = std::chrono::system_clock::time_point::min();
    info.needsReload = true;
    info.id = 0; // Will be loaded in updateTextures()
    
    textures[name] = info;
    return 0; // Texture will be loaded on first update
}

unsigned int TextureManager::getTexture(const std::string& name) {
    std::lock_guard<std::mutex> lock(texturesMutex);
    
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second.id;
    }
    
    return 0; // Texture not found
}

void TextureManager::updateTextures() {
    std::lock_guard<std::mutex> lock(texturesMutex);
    
    for (auto& pair : textures) {
        TextureInfo& info = pair.second;
        
        if (!fileExists(info.path)) {
            continue;
        }
        
        auto currentModTime = getFileModTime(info.path);
        
        // Check if texture needs to be loaded or reloaded
        if (info.needsReload || currentModTime > info.lastModified) {
            // Delete old texture if it exists
            if (info.id != 0) {
                glDeleteTextures(1, &info.id);
            }
            
            // Load new texture
            info.id = loadTextureFromFile(info.path);
            info.lastModified = currentModTime;
            info.needsReload = false;
            
            std::cout << "🔄 Hot-reloaded texture: " << pair.first << " from " << info.path << std::endl;
        }
    }
}

void TextureManager::watchFiles() {
    std::chrono::system_clock::time_point lastUpdateCheck = std::chrono::system_clock::now();
    
    while (running) {
        try {
            // Check for UPDATE file
            if (fileExists(updateFile)) {
                auto updateTime = getFileModTime(updateFile);
                
                if (updateTime > lastUpdateCheck) {
                    std::cout << "⚡ Detected Python editor update signal - refreshing textures..." << std::endl;
                    
                    // Mark realtime textures for reload
                    std::lock_guard<std::mutex> lock(texturesMutex);
                    for (auto& pair : textures) {
                        if (pair.first.find("realtime_") == 0) {
                            pair.second.needsReload = true;
                        }
                    }
                    
                    lastUpdateCheck = updateTime;
                    
                    // Remove UPDATE file
                    try {
                        std::filesystem::remove(updateFile);
                    } catch (...) {
                        // Ignore errors when removing update file
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: Exception in file watching: " << e.what() << std::endl;
        }
        
        // Sleep for a short time to avoid high CPU usage (faster polling for responsiveness)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void TextureManager::startWatching() {
    if (!running) {
        running = true;
        watchThread = std::thread(&TextureManager::watchFiles, this);
        std::cout << "Started texture file watching..." << std::endl;
    }
}

void TextureManager::stopWatching() {
    if (running) {
        running = false;
        if (watchThread.joinable()) {
            watchThread.join();
        }
        std::cout << "Stopped texture file watching." << std::endl;
    }
}

bool TextureManager::isRealtimeMaterialAvailable() {
    return fileExists(realtimeDir + "/albedo.png") &&
           fileExists(realtimeDir + "/normal.png") &&
           fileExists(realtimeDir + "/metallic.png") &&
           fileExists(realtimeDir + "/roughness.png") &&
           fileExists(realtimeDir + "/ao.png");
}

float TextureManager::getDisplacementIntensity() {
    try {
        std::string paramsFile = realtimeDir + "/material_params.json";
        if (!fileExists(paramsFile)) {
            return displacementIntensity; // Return cached value if file doesn't exist
        }
        
        // Simple JSON parsing for displacement_intensity
        std::ifstream file(paramsFile);
        if (!file.is_open()) {
            return displacementIntensity;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Look for displacement_intensity value
            size_t pos = line.find("\"displacement_intensity\":");
            if (pos != std::string::npos) {
                // Extract the value after the colon
                size_t colonPos = line.find(':', pos);
                if (colonPos != std::string::npos) {
                    std::string valueStr = line.substr(colonPos + 1);
                    // Remove whitespace and trailing comma
                    size_t start = valueStr.find_first_not_of(" \t");
                    size_t end = valueStr.find_first_of(",}");
                    if (start != std::string::npos) {
                        valueStr = valueStr.substr(start, end - start);
                        try {
                            float newValue = std::stof(valueStr);
                            displacementIntensity = newValue;
                            return displacementIntensity / 100.0f; // Convert 0-100 to 0.0-1.0 for shader
                        } catch (const std::exception& e) {
                            std::cout << "Warning: Failed to parse displacement intensity: " << e.what() << std::endl;
                        }
                    }
                }
            }
        }
        file.close();
    } catch (const std::exception& e) {
        std::cout << "Warning: Failed to read displacement intensity: " << e.what() << std::endl;
    }
    
    return displacementIntensity / 100.0f; // Convert 0-100 to 0.0-1.0 for shader
}
