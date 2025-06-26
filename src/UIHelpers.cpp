#include "UIHelpers.hpp"

#include <unordered_map>

#include "ResourceManager.hpp"
#include "imgui.h"

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

static std::unordered_map<std::string, bool> highlighted_before;
bool DrawHyperlinkButton(const char* label)
{
    if(highlighted_before.size() > 16)
        highlighted_before.clear();
    bool is_hovered = highlighted_before[label];
    if(is_hovered)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(62, 130, 210, 255));
    bool pressed = ImGui::SmallButton(label);

    highlighted_before[label] = ImGui::IsItemHovered();

    // Fake underlined
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