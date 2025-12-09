#include "ModelLoader.h"
#include <algorithm>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/scene.h>
//not
std::map<std::string, std::shared_ptr<ppgso::Texture>> ModelLoader::textureCache;

std::string getFileExtension(const std::string& path) {
    auto pos = path.rfind('.');
    return (pos == std::string::npos) ? "" : path.substr(pos);
}

std::string getFileName(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

std::string getFileNameWithoutExtension(const std::string& path) {
    std::string filename = getFileName(path);
    auto pos = filename.rfind('.');
    return (pos == std::string::npos) ? filename : filename.substr(0, pos);
}

std::string combinePaths(const std::string& base, const std::string& relative) {
    if (base.empty()) return relative;
    if (relative.empty()) return base;

    char lastChar = base[base.length() - 1];

    if (lastChar == '/' || lastChar == '\\') return base + relative;
    return base + "/" + relative;
}

ppgso::Image ModelLoader::loadImageFile(const std::string& filename) {
    int width, height, channels;

    // Load image using stb_image
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (!data) throw std::runtime_error("Failed to load image: " + filename);

    ppgso::Image image(width, height);

    // Copy pixel data
    auto& framebuffer = image.getFramebuffer();
    for (int i = 0; i < width * height; i++) {
        framebuffer[i].r = data[i * 4 + 0];
        framebuffer[i].g = data[i * 4 + 1];
        framebuffer[i].b = data[i * 4 + 2];
        framebuffer[i].a = data[i * 4 + 3];
    }

    stbi_image_free(data);
    return image;
}

std::shared_ptr<ppgso::Texture> ModelLoader::loadTextureFile(const std::string& path) {
    std::string ext = getFileExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto it = textureCache.find(path);

    if (it != textureCache.end()) {
        return it->second;
    }

    try {
        std::shared_ptr<ppgso::Texture> texture;

        if (ext == ".png") {
            texture = std::make_unique<ppgso::Texture>(loadImageFile(path));
        }

        if (texture) {
            textureCache[path] = texture;
        }

        return texture;

    } catch (const std::exception& e) {
        std::cerr << "Failed to load texture " << path << ": " << e.what() << std::endl;
        return nullptr;
    }
}

//Placeholder texture
std::unique_ptr<ppgso::Texture> ModelLoader::createWhiteTexture() {
    // Create a texture as placeholder
    ppgso::Image image(1, 1);
    image.setPixel(0, 0, {255, 255, 255});
    return std::make_unique<ppgso::Texture>(std::move(image));
}

std::unique_ptr<ppgso::Texture> ModelLoader::createDefaultNormalMap() {
    ppgso::Image image(1, 1);
    image.setPixel(0, 0, {128, 128, 255, 255});
    return std::make_unique<ppgso::Texture>(std::move(image));
}

std::string ModelLoader::findTextureFile(const std::string& texturePath, const std::string& baseDir) {
    std::string fullPath = combinePaths(baseDir, texturePath);
    std::string filename = getFileName(texturePath);

    std::string texturesPath = combinePaths(baseDir, "textures");
    std::string textureFullPath = combinePaths(texturesPath, filename);

    return textureFullPath;
}

std::string ModelLoader::findNormalMapByName(const std::string& diffusePath, const std::string& baseDir) {
    // Get the base name without extension
    std::string baseName = getFileNameWithoutExtension(diffusePath);
    std::string ext = getFileExtension(diffusePath);

    std::vector<std::string> patterns = {
        baseName + "_normal" + ext,      // texture_normal.png
    };

    // Also try .png if original was .jpg and vice versa
    std::string altExt = (ext == ".png") ? ".jpg" : ".png";
    patterns.push_back(baseName + "_normal" + altExt);
    patterns.push_back(baseName + "_Normal" + altExt);

    std::string diffuseDir = diffusePath.substr(0, diffusePath.find_last_of("/\\") + 1);

    for (const auto& pattern : patterns) {
        std::string fullPath = diffuseDir + pattern;

        // Check if file exists
        std::ifstream file(fullPath);
        if (file.good()) {
            std::cout << "Found normal map: " << fullPath << std::endl;
            return fullPath;
        }

        std::string texturesPath = combinePaths(baseDir, "textures");
        std::string altPath = combinePaths(texturesPath, pattern);
        std::ifstream altFile(altPath);
        if (altFile.good()) {
            std::cout << "Found normal map: " << altPath << std::endl;
            return altPath;
        }
    }

    return "";
}

std::map<unsigned int, ModelLoader::MaterialTextures> ModelLoader::loadAllTextures(
    const ppgso::Mesh_Assimp* mesh) {

    std::map<unsigned int, MaterialTextures> materialTextures;

    const aiScene* scene = mesh->getScene();
    if (!scene) {
        std::cerr << "No scene available!" << std::endl;
        return materialTextures;
    }

    std::string baseDir = mesh->getBasePath();
    std::cout << "\n=== Loading textures from: " << baseDir << " ===" << std::endl;

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        MaterialTextures matTex;

        aiString materialName;
        material->Get(AI_MATKEY_NAME, materialName);
        std::cout << "\nMaterial " << i << ": " << materialName.C_Str() << std::endl;

        std::string diffuseFullPath;

        // Load diffuse texture
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString texPath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
            diffuseFullPath = findTextureFile(texPath.C_Str(), baseDir);

            if (!diffuseFullPath.empty()) {
                std::cout << "Diffuse: " << diffuseFullPath << std::endl;
                matTex.diffuse = loadTextureFile(diffuseFullPath);
            }
        }

        //Normal map
        if (!diffuseFullPath.empty()) {
            std::cout << "Looking for normal map" << std::endl;
            std::string normalPath = findNormalMapByName(diffuseFullPath, baseDir);
            if (!normalPath.empty()) {
                matTex.normal = loadTextureFile(normalPath);
                if (matTex.normal) {
                    std::cout << "Normal map loaded successfully!" << std::endl;
                }
            }
        }

        // Load specular map
        if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            aiString texPath;
            material->GetTexture(aiTextureType_SPECULAR, 0, &texPath);
            std::string fullPath = findTextureFile(texPath.C_Str(), baseDir);

            if (!fullPath.empty()) {
                std::cout << "  Specular: " << fullPath << std::endl;
                matTex.specular = loadTextureFile(fullPath);
            }
        }

        // Create defaults
        if (!matTex.diffuse) {
            std::cout << "  Using white placeholder for diffuse" << std::endl;
            matTex.diffuse = createWhiteTexture();
        }
        if (!matTex.normal) {
            std::cout << "  Using default flat normal map" << std::endl;
            matTex.normal = createDefaultNormalMap();
        }

        materialTextures[i] = matTex;
    }

    std::cout << "\n=== Total materials: " << materialTextures.size() << " ===" << std::endl;
    return materialTextures;
}

// Backwards compatibility - just return diffuse textures
std::map<unsigned int, std::shared_ptr<ppgso::Texture>> ModelLoader::loadTextures(
    const ppgso::Mesh_Assimp* mesh) {

    auto allTextures = loadAllTextures(mesh);
    std::map<unsigned int, std::shared_ptr<ppgso::Texture>> diffuseOnly;

    for (const auto& pair : allTextures) {
        diffuseOnly[pair.first] = pair.second.diffuse;
    }

    return diffuseOnly;
}
