#ifndef UI_HELPERS_HPP
#define UI_HELPERS_HPP

#include <stdint.h>
#include <stdbool.h>

#include "imgui.h"

bool DrawInactiveableButton(
    const char* active_label, const char* inactive_label,
    const bool image_button, const bool active
);

bool DrawHyperlinkButton(const char* label);

void UIHelpersInit();

void CalculateSplitterPanel(ImVec2* panel_size, float *ratio, const float size, const bool vertical, const ImVec2 range = ImVec2(0, 1));
void DrawSplitter(
    const int id, ImVec2* panel_size,
    float* ratio, const float size, const bool vertical, 
    void (*draw_panel1)(const float, uint8_t*), uint8_t* panel1_data,
    void (*draw_panel2)(const float, uint8_t*), uint8_t* panel2_data,
    const ImVec2 range = ImVec2(0, 1));

void DisplayCenteredText(const char* text);
#endif