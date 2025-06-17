#include "ResourceManager.hpp"
#include <SDL_render.h>
#include <SDL_surface.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>

SDL_Renderer* ResourceManager::s_Renderer = nullptr;
std::unordered_map<std::string, ImageResource> ResourceManager::s_Images = {};

void ResourceManager::Init(SDL_Renderer *renderer) {
    s_Renderer = renderer;
}

void ResourceManager::LoadImage(const std::string &name)
{
    int width, height, channels;
    std::string constructed = "/home/binaryfox0/proj/toshiba-t100-pc/resources/images/" + name; // relative path for portability
    stbi_uc* image_data = stbi_load(constructed.c_str(), &width, &height, &channels, 4); // force RGBA
    if (!image_data) {
        fprintf(stderr, "ToshibaT100: error: failed to load image '%s': %s\n", name.c_str(), stbi_failure_reason());
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
        fprintf(stderr, "SDL_CreateRGBSurfaceFrom failed: %s\n", SDL_GetError());
        stbi_image_free(image_data);
        return;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(s_Renderer, surface);
    if (!tex) {
        fprintf(stderr, "SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        stbi_image_free(image_data);
        return;
    }

    s_Images[name] = ImageResource(tex, ImVec2((float)width, (float)height));

    SDL_FreeSurface(surface);
    stbi_image_free(image_data);
}

ImageResource& ResourceManager::GetImage(const std::string &name) {
    return s_Images.at(name);
}
