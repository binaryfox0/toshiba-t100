#ifndef UI_HELPERS_HPP
#define UI_HELPERS_HPP

#include <stdint.h>
#include <stdbool.h>

bool DrawInactiveableButton(
    const char* active_label, const char* inactive_label,
    const bool image_button, const bool active
);

bool DrawHyperlinkButton(const char* label);

void DisplayCenteredText(const char* text);
#endif