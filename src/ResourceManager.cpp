#include "ResourceManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>

#include "Internal.h"
#include "Logging.h"

SDL_Renderer* ResourceManager::s_Renderer = nullptr;
std::unordered_map<std::string, ImageResource> ResourceManager::s_Images = {};

void ResourceManager::Init(SDL_Renderer *renderer) {
    s_Renderer = renderer;
}

void ResourceManager::LoadImage(const std::string& path, const std::string &name)
{
    int width, height, channels;
    stbi_uc* image_data = stbi_load(path.c_str(), &width, &height, &channels, 4); // force RGBA
    if (!image_data) {
        warn("ToshibaT100: error: failed to load image '%s': %s\n", name.c_str(), stbi_failure_reason());
        return;
    }

    // Create surface assuming RGBA (RGBA32)
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        image_data,
        width, height,
        32, width * 4,
        0x000000ff, // R
        0x0000ff00, // G
        0x00ff0000, // B
        0xff000000  // A
    );

    if (!surface) {
        warn("SDL_CreateRGBSurfaceFrom failed: %s\n", SDL_GetError());
        stbi_image_free(image_data);
        return;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(s_Renderer, surface);
    if (!tex) {
        warn("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        stbi_image_free(image_data);
        return;
    }

    s_Images[name] = ImageResource(tex, ImVec2((float)width, (float)height));

    SDL_FreeSurface(surface);
    stbi_image_free(image_data);
    info("image resources: \"%s\" was loaded successfully", path.c_str());
}

static ImageResource empty_image_resource = ImageResource();
ImageResource& ResourceManager::GetImage(const std::string &name) {
    if(s_Images.count(name) > 0)
        return s_Images[name];
    warn("trying to load non-existent image resource: %s", name.c_str());
    return empty_image_resource;
}
