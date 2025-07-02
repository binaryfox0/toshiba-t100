#include "UIHelpers.hpp"

#include <cassert>
#include <cmath>
#include <unordered_map>

#include "Internal.h"
#include "ResourceManager.hpp"
#include "imgui.h"
#include "imgui_internal.h"

bool DrawInactiveableButton(
    const char* active_label, const char* inactive_label,
    const bool image_button, const bool active
)
{
    if(!active) {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));
    }
    ImGui::PushID(active_label);

    bool pressed = false;
    if(image_button) {
        ImageResource &image = ResourceManager::GetImage(active ? active_label : inactive_label);
        pressed = ImGui::ImageButton("##inactivable_button", (ImTextureID)(intptr_t)image.texture, image.size) && active;
    } else {
        pressed = ImGui::Button(active ? active_label : inactive_label);
    }

    ImGui::PopID();

    if(!active) {
        ImGui::PopStyleColor(2);
    }
    return pressed;
}

// I don't use ImGui::TextLink here because it don't have thing changed color from white (unhovered) to blue (hovered)
static std::unordered_map<std::string, bool> highlighted_before;
bool DrawHyperlinkButton(const char* label)
{
    if(highlighted_before.size() > 16)
        highlighted_before.clear();
    bool is_hovered = false;
    bool is_existed = highlighted_before.count(label);
    if(is_existed)
        is_hovered = highlighted_before[label];
    if(is_hovered)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(62, 130, 210, 255));
    bool pressed = ImGui::SmallButton(label);

    bool is_hovered_imgui = ImGui::IsItemHovered();
    if(is_existed || is_hovered_imgui) {
        highlighted_before[label] = is_hovered_imgui;
        if(is_hovered_imgui)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);        
    }

    ImVec2 text_size = ImGui::CalcTextSize(label, 0, true);

    // Get item rect
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    float item_width = max.x - min.x;

    // Y position for underline (a bit below text)
    float underline_y = min.y + (max.y - min.y) * 0.5f + text_size.y * 0.5f + 1.0f;

    // Center the underline to match the label
    float text_start_x = min.x + (item_width - text_size.x) * 0.5f;

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(text_start_x, underline_y),
        ImVec2(text_start_x + text_size.x, underline_y),
        ImGui::GetColorU32(ImGuiCol_Text)
    );
    if(is_hovered)
        ImGui::PopStyleColor();

    return pressed;
}

static const float splitter_width = 4.0f;
static ImGuiStyle* style = 0;
void UIHelpersInit() {
    style = &ImGui::GetStyle();
}

void CalculateSplitterPanel(ImVec2* panel_size, float* ratio, const float size, const bool vertical, const ImVec2 range) {
    float unusable = (vertical ? 
        (splitter_width + style->WindowPadding.x * 2) :
        (splitter_width + style->WindowPadding.y * 2)
    );
    float real_space = size - unusable;
    if(*ratio > range.y)
        *ratio = range.y;
    if(*ratio < range.x)
        *ratio = range.x;
    panel_size->x = (real_space * *ratio);
    panel_size->y = (real_space - panel_size->x);
}

void DrawSplitter(
    const int id, ImVec2* panel_size,
    float* ratio, const float size, const bool vertical, 
    void (*draw_panel1)(const float, uint8_t*), uint8_t* panel1_data,
    void (*draw_panel2)(const float, uint8_t*), uint8_t* panel2_data,
    const ImVec2 range)
{
    // auto get_pos = vertical ? ImGui::GetCursorPosX : ImGui::GetCursorPosY;
    // auto set_pos = vertical ? ImGui::SetCursorPosX : ImGui::SetCursorPosY;

    // float panel1_pos = get_pos();
    // set_pos(panel1_pos + (vertical ? panel_size->y : panel_size->x));

    ImVec2 panel1_pos = ImGui::GetCursorPos();
    ImVec2 splitter_pos = ImVec2(
        panel1_pos.x + (vertical ? panel_size->x : 0),
        panel1_pos.y + (!vertical ? panel_size->x : 0)
    );
    ImGui::SetCursorPos(splitter_pos);

    ImGui::PushID(id);
    ImGui::Button("##splitter", vertical ? ImVec2(splitter_width, -1) : ImVec2(-1, splitter_width));
    ImGui::PopID();

    ImGuiMouseCursor type = vertical ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
    if(ImGui::IsItemHovered())
        ImGui::SetMouseCursor(type);
    if(ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(type);
        float new_ratio = *ratio + (vertical ? ImGui::GetIO().MouseDelta.x : ImGui::GetIO().MouseDelta.y) / size;
        if(new_ratio > range.y)
            new_ratio = range.y;
        if(new_ratio < range.x)
            new_ratio = range.x;
        *ratio = new_ratio;
        CalculateSplitterPanel(panel_size, ratio, size, vertical, range);
    }

    ImGui::SetCursorPos(panel1_pos);
    draw_panel1(panel_size->x, panel1_data);
    ImGui::SetCursorPos(ImVec2(
        panel1_pos.x + (vertical  ? panel_size->x + 4 : 0),
        panel1_pos.y + (!vertical ? panel_size->x + 4 : 0)
    ));
    draw_panel2(panel_size->y, panel2_data);
}

void DisplayCenteredText(const char* text)
{
    ImVec2 avail = ImGui::GetWindowSize();
    ImVec2 text_size = ImGui::CalcTextSize(text);

    ImGui::SetCursorPos(ImVec2(
        (avail.x - text_size.x) * 0.5f,
        (avail.y - text_size.y) * 0.5f
    ));
    ImGui::TextUnformatted(text);
}