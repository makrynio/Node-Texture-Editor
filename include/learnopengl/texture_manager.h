#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <filesystem>

class TextureManager {
private:
    struct TextureInfo {
        unsigned int id;
        std::string path;
        std::chrono::system_clock::time_point lastModified;
        bool needsReload;
    };
    
    std::unordered_map<std::string, TextureInfo> textures;
    std::thread watchThread;
    std::atomic<bool> running;
    std::mutex texturesMutex;
    
    // Realtime material paths
    std::string realtimeDir;
    std::string updateFile;
    
    // Displacement intensity (0-100 range, converted to 0.0-1.0 for shader)
    float displacementIntensity;
    
    void watchFiles();
    bool fileExists(const std::string& path);
    std::chrono::system_clock::time_point getFileModTime(const std::string& path);
    unsigned int loadTextureFromFile(const std::string& path);
    
public:
    TextureManager();
    ~TextureManager();
    
    // Register a texture for hot-reloading
    unsigned int registerTexture(const std::string& name, const std::string& path);
    
    // Get texture ID (will be updated if file changed)
    unsigned int getTexture(const std::string& name);
    
    // Check and reload any changed textures (call from main thread)
    void updateTextures();
    
    // Start/stop file watching
    void startWatching();
    void stopWatching();
    
    // Realtime material management
    void setRealtimeDirectory(const std::string& dir);
    bool isRealtimeMaterialAvailable();
    
    // Get realtime material texture IDs
    unsigned int getRealtimeAlbedo() { return getTexture("realtime_albedo"); }
    unsigned int getRealtimeNormal() { return getTexture("realtime_normal"); }
    unsigned int getRealtimeMetallic() { return getTexture("realtime_metallic"); }
    unsigned int getRealtimeRoughness() { return getTexture("realtime_roughness"); }
    unsigned int getRealtimeAO() { return getTexture("realtime_ao"); }
    unsigned int getRealtimeHeight() { return getTexture("realtime_height"); }
    
    // Get displacement intensity
    float getDisplacementIntensity();
};

#endif
