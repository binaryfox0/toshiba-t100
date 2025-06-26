#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <stdbool.h>

#include "imgui.h"

bool DrawInactiveableButton(
    const char* active_label, const char* inactive_label,
    const bool image_button, const bool active
);

bool DrawHyperlinkButton(const char* label);

#endif