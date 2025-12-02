#pragma once
#include <memory>
#include <map>
#include <string>
#include <ppgso/ppgso.h>
#include "Mesh_Assimp.h"

class ModelLoader {
public:
    // Load textures for a mesh based on its material information
    static std::map<unsigned int, std::unique_ptr<ppgso::Texture>> loadTextures(
        const ppgso::Mesh_Assimp* mesh);
    
    // Load a single texture file
    static std::unique_ptr<ppgso::Texture> loadTextureFile(const std::string& path);
    static std::shared_ptr<ppgso::Texture> loadTextureFileShared(const std::string& path);
    // Create a default placeholder texture
    static std::unique_ptr<ppgso::Texture> createWhiteTexture();

private:
    // Load image using stb_image
    static ppgso::Image loadImageFile(const std::string& filename);
    
    // Find texture file with various path strategies
    static std::string findTextureFile(const std::string& texturePath, const std::string& baseDir);
    static std::map<std::string, std::shared_ptr<ppgso::Texture>> textureCache;
};


class TransparentTexture : public ppgso::Texture {
public:
    TransparentTexture(const std::string& path);
private:
    void load(const std::string& path);
};