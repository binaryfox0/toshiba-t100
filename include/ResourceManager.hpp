#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <stdint.h> // for uint64_t aka ImTextureID

#include "SDL2/SDL.h"
#include "imgui.h"

class ImageResource {
public:
    ImageResource() = default;

    ImageResource(SDL_Texture* texture, ImVec2 size)
        : texture(texture), size(size) {}

    // Delete copy operations (avoid double free)
    ImageResource(const ImageResource&) = delete;
    ImageResource& operator=(const ImageResource&) = delete;

    // Move constructor
    ImageResource(ImageResource&& other) noexcept
        : texture(other.texture), size(other.size) {
        other.texture = nullptr;
    }

    // Move assignment
    ImageResource& operator=(ImageResource&& other) noexcept {
        if (this != &other) {
            if (texture)
                SDL_DestroyTexture(texture);
            texture = other.texture;
            size = other.size;
            other.texture = nullptr;
        }
        return *this;
    }

    ~ImageResource() {
        if (texture)
            SDL_DestroyTexture(texture);
    }

    SDL_Texture* texture = nullptr;
    ImVec2 size = ImVec2(0, 0);
};


class ResourceManager {
public:
    static void Init(SDL_Renderer* renderer);
    static void LoadImage(const std::string& path, const std::string& name);
    static ImageResource& GetImage(const std::string& name);

private:
    static SDL_Renderer* s_Renderer;
    static std::unordered_map<std::string, ImageResource> s_Images;
};

#endif