#include "ModelLoader.h"
#include <algorithm>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/scene.h>

std::string getFileExtension(const std::string& path) {
    auto pos = path.rfind('.');
    return (pos == std::string::npos) ? "" : path.substr(pos);
}

std::string getFileName(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
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
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);

    if (!data) throw std::runtime_error("Failed to load image: " + filename);

    ppgso::Image image(width, height);

    // Copy pixel data
    auto& framebuffer = image.getFramebuffer();
    for (int i = 0; i < width * height; i++) {
        framebuffer[i].r = data[i * 3 + 0];
        framebuffer[i].g = data[i * 3 + 1];
        framebuffer[i].b = data[i * 3 + 2];
    }

    stbi_image_free(data);
    return image;
}

std::unique_ptr<ppgso::Texture> ModelLoader::loadTextureFile(const std::string& path) {
    std::string ext = getFileExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    try {
        if (ext == ".bmp") {
            return std::make_unique<ppgso::Texture>(ppgso::image::loadBMP(path));
        }
        if (ext == ".png") {
           return std::make_unique<TransparentTexture>(path);
        }
        else {
            return std::make_unique<ppgso::Texture>(loadImageFile(path));
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load texture " << path << ": " << e.what() << std::endl;
        return nullptr;
    }
}

//Placeholder texture
std::unique_ptr<ppgso::Texture> ModelLoader::createWhiteTexture() {
    return std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("asteroid.bmp")); //TODO: Replace asteroid texture with something else
}

std::string ModelLoader::findTextureFile(const std::string& texturePath, const std::string& baseDir) {
    std::string fullPath = combinePaths(baseDir, texturePath);
    std::string filename = getFileName(texturePath);

    std::string texturesPath = combinePaths(baseDir, "textures");
    std::string textureFullPath = combinePaths(texturesPath, filename);

    return textureFullPath;
}

std::map<unsigned int, std::unique_ptr<ppgso::Texture>> ModelLoader::loadTextures(const ppgso::Mesh_Assimp* mesh) {
    std::map<unsigned int, std::unique_ptr<ppgso::Texture>> materialTextures;

    const aiScene* scene = mesh->getScene();
    if (!scene) {
        std::cerr << "No scene available!" << std::endl;
        return materialTextures;
    }

    std::string baseDir = mesh->getBasePath();
    std::cout << "Loading textures from: " << baseDir << std::endl;
    std::cout << "Number of materials: " << scene->mNumMaterials << std::endl;

    // Load texture for each material
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];

        // Get material name
        aiString materialName;
        material->Get(AI_MATKEY_NAME, materialName);
        std::cout << "\nMaterial " << i << ": " << materialName.C_Str() << std::endl;

        // Try to load diffuse texture
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString texPath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);

            std::string texPathStr = texPath.C_Str();
            std::cout << "  Texture path from FBX: " << texPathStr << std::endl;

            // Find the actual file
            std::string fullPath = findTextureFile(texPathStr, baseDir);

            if (!fullPath.empty()) {
                std::cout << "  Loading: " << fullPath << std::endl;
                auto texture = loadTextureFile(fullPath);
                if (texture) {
                    materialTextures[i] = std::move(texture);
                    std::cout << "  Successfully loaded!" << std::endl;
                }
            } else {
                std::cerr << "  Texture file not found!" << std::endl;
            }
        } else {
            std::cout << "  No diffuse texture specified" << std::endl;
        }
    }

    // Create placeholder texture for materials without textures
    std::cout << "\nCreating fallback textures..." << std::endl;
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        if (materialTextures.find(i) == materialTextures.end()) {
            std::cout << "Material " << i << " using placeholder texture" << std::endl;
            materialTextures[i] = createWhiteTexture();
        }
    }

    std::cout << "\nTotal textures loaded: " << materialTextures.size() << std::endl;

    return materialTextures;
}


TransparentTexture::TransparentTexture(const std::string& path) : ppgso::Texture(1, 1) {
    load(path);
}

void TransparentTexture::load(const std::string& path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::cerr << "Failed to load transparent PNG: " << path << std::endl;
        return;
    }
    else {
        std::cout << "Loaded transparent PNG: " << path << std::endl;
    }

    // Get the current texture ID
    GLuint oldTexture = getTexture();
    glDeleteTextures(1, &oldTexture);

    // Create new RGBA texture
    GLuint newTexture;
    glGenTextures(1, &newTexture);
    glBindTexture(GL_TEXTURE_2D, newTexture);

    // Upload as RGBA8
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    stbi_image_free(data);

    // Replace internal texture ID (access protected member)
    texture = newTexture;
}
