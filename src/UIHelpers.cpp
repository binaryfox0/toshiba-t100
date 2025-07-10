#include "UIHelpers.hpp"

#include <unordered_map>

#include "ResourceManager.hpp"
#include "Internal.h"

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

// This is from imgui_internal.h but it is static inline, inaccessible from outside
INLINE const char* CalcWordWrapNextLineStartA(const char* text, const char* text_end)
{
    while (text < text_end && ImCharIsBlankA(*text))
        text++;
    return text;
}

void DisplayCenteredText(const char* text) {
    ImFont* font = ImGui::GetFont();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float font_size = ImGui::GetCurrentContext()->FontSize;
    const ImVec2 reigon = ImGui::GetContentRegionAvail();

    // First pass: Calculate text height to center window
    const char* s = text, *text_end = s + strlen(s);
    float total_height = 0.0f;
        while(s < text_end) {
        const char* line_end = (const char*)memchr(s, '\n', text_end - s);
        if(!line_end) line_end = text_end;
        const char* line_start = s;
        while(line_start < line_end) {
            const char* wrap_eol = font->CalcWordWrapPosition(font_size, line_start, line_end, reigon.x);
            if(wrap_eol == line_start)
                wrap_eol = line_start + 1;
            total_height += ImGui::GetTextLineHeightWithSpacing();
            line_start = CalcWordWrapNextLineStartA(wrap_eol, line_end);
        }
        s = line_end + (line_end < text_end);
    }

    // Second pass: Render it.
    ImVec2 draw_pos = ImGui::GetCursorScreenPos();
    draw_pos.y += (reigon.y - total_height) * 0.5f;
    s = text; text_end = s + strlen(s); // reset
    while(s < text_end) {
        const char* line_end = (const char*)memchr(s, '\n', text_end - s);
        if(!line_end) line_end = text_end;
        const char* line_start = s;
        while(line_start < line_end) {
            const char* wrap_eol = font->CalcWordWrapPosition(font_size, line_start, line_end, reigon.x);
            if(wrap_eol == line_start)
                wrap_eol = line_start + 1;
            float linesize_x = ImGui::CalcTextSize(line_start, wrap_eol).x;
            float centered_x = draw_pos.x + (reigon.x - linesize_x) * 0.5f;
            draw_list->AddText(ImVec2(centered_x, draw_pos.y), ImGui::GetColorU32(ImGuiCol_Text), line_start, wrap_eol);
            draw_pos.y += ImGui::GetTextLineHeightWithSpacing();
            line_start = CalcWordWrapNextLineStartA(wrap_eol, line_end);
        }
        s = line_end + (line_end < text_end);
    }
    // There is no dummy here because this text only be displayed when there is no content so no need for that
}
