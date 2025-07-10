#include "Splitter.hpp"

#include "imgui.h"

ImGuiStyle *Splitter::style = 0;

void Splitter::InitStyle() {
    style = &ImGui::GetStyle();
}

static const float splitter_thickness = 4.0f;
void Splitter::Draw()
{
    ImVec2 panel1_pos = ImGui::GetCursorPos();
    ImVec2 splitter_pos = ImVec2(
        panel1_pos.x + (vertical ? panel_size.x : 0),
        panel1_pos.y + (!vertical ? panel_size.x : 0)
    );
    ImGui::SetCursorPos(splitter_pos);
    
    ImGui::PushID(id);
    ImGui::Button("##splitter", 
        vertical ? ImVec2(splitter_thickness, -1) : ImVec2(-1, splitter_thickness));
    ImGui::PopID();

    ImGuiMouseCursor type = vertical ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
    if(ImGui::IsItemHovered())
        ImGui::SetMouseCursor(type);
    if(ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(type);
        float new_ratio = ratio + (vertical ? ImGui::GetIO().MouseDelta.x : ImGui::GetIO().MouseDelta.y) / window_size;
        if(new_ratio > ratio_range.y)
            new_ratio = ratio_range.y;
        if(new_ratio < ratio_range.x)
            new_ratio = ratio_range.x;
        ratio = new_ratio;
        UpdateSize(window_size);
    }

    ImGui::SetCursorPos(panel1_pos);
    panel1_draw(panel_size.x, this);
    ImGui::SetCursorPos(ImVec2(
        panel1_pos.x + (vertical  ? panel_size.x + splitter_thickness : 0),
        panel1_pos.y + (!vertical ? panel_size.x + splitter_thickness : 0)
    ));
    panel2_draw(panel_size.y, this);
}

void Splitter::UpdateSize(const float new_size)
{
    if(!style)
        return;
    window_size = new_size;
    float unusable = (vertical ? 
        (splitter_thickness + style->WindowPadding.x * padding_count) :
        (splitter_thickness + style->WindowPadding.y * padding_count)
    );
    float real_space = new_size - unusable;
    panel_size.x = (float)(int)(real_space * ratio);
    panel_size.y = (real_space - panel_size.x);
}