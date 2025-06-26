#ifndef LOGGING_H
#define LOGGING_H

#include "Internal.h"

#ifdef BUILD_DEBUG
#   define logging_helper(a, b, ...) a b __VA_ARGS__
#else
#   define logging_helper(a, b, ...)
#endif

EXTERN_C_BEGIN

typedef struct SDL_Renderer SDL_Renderer;
// void LogSDLRendererInfo(const SDL_Renderer* renderer);
logging_helper(void, LogSDLRendererInfo, (SDL_Renderer* renderer));

EXTERN_C_END

#endif