#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>

struct TextureData {
    unsigned int textureID;
    int width;
    int height;
    int channels;
    std::vector<unsigned char> data;
    
    TextureData(int w, int h, int c) : width(w), height(h), channels(c) {
        data.resize(w * h * c);
    }
};

class TextureGenerator {
public:
    static std::shared_ptr<TextureData> GenerateCheckerboard(int width, int height, int checkerSize = 32);
    static unsigned int CreateOpenGLTexture(const TextureData& textureData);
    static void DeleteTexture(unsigned int textureID);
};
