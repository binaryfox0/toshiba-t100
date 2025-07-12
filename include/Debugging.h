#ifndef DEBUGGING_H
#define DEBUGGING_H

#include "Internal.h"

EXTERN_C_BEGIN

typedef struct SDL_Renderer SDL_Renderer;
void LogSDLRendererInfo(SDL_Renderer* renderer);

EXTERN_C_END

#endif