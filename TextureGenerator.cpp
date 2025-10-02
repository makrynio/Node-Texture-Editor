#include "TextureGenerator.h"
#include <iostream>

std::shared_ptr<TextureData> TextureGenerator::GenerateCheckerboard(int width, int height, int checkerSize) {
    auto textureData = std::make_shared<TextureData>(width, height, 1); // Greyscale
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int checkerX = (x / checkerSize) % 2;
            int checkerY = (y / checkerSize) % 2;
            
            // Create checkerboard pattern
            unsigned char value = ((checkerX + checkerY) % 2) ? 255 : 0;
            
            int index = (y * width + x) * textureData->channels;
            textureData->data[index] = value;
        }
    }
    
    return textureData;
}

unsigned int TextureGenerator::CreateOpenGLTexture(const TextureData& textureData) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    GLenum format = textureData.channels == 1 ? GL_RED : 
                   textureData.channels == 3 ? GL_RGB : GL_RGBA;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, textureData.width, textureData.height, 
                 0, format, GL_UNSIGNED_BYTE, textureData.data.data());
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

void TextureGenerator::DeleteTexture(unsigned int textureID) {
    glDeleteTextures(1, &textureID);
}
