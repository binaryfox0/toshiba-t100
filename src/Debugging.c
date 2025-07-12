#include "Debugging.h"

#include <SDL2/SDL.h>


void LogSDLRendererInfo(SDL_Renderer* renderer) {
#ifdef BUILD_DEBUG
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    
    const char* supported_renderer_flags_str[] = {
        "SDL_RENDERER_SOFTWARE",
        "SDL_RENDERER_ACCELERATED",
        "SDL_RENDERER_PRESENTVSYNC",
        "SDL_RENDERER_TARGETTEXTURE"
    };
    
    info("SDL_Renderer info:\n"
            " - name: %s",
            info.name ? info.name : "unknown");

    printf(" - supported flags: \n");
    for (int i = 0; i < sizeof(supported_renderer_flags_str) / sizeof(supported_renderer_flags_str[0]); i++) {
        if (info.flags & (1 << i)) {
            printf("  - %s\n", supported_renderer_flags_str[i]);
        }
    }
    
    printf(" - texture formats: %d format%s\n", info.num_texture_formats, info.num_texture_formats > 1 ? "s" : "");
    for(int i = 0; i < info.num_texture_formats; i++) {
        printf("  - %s\n", SDL_GetPixelFormatName(info.texture_formats[i]));
    }

    printf(" - max texture size: %dx%dpx\n", info.max_texture_width, info.max_texture_height);
#endif
}