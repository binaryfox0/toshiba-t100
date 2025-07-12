#include "DisassemblerView/Breakpoint.hpp"

#include <stdint.h>

#include "DeviceResources.hpp"

namespace DisassemblerView
{
void DrawBreakpointCircle(ImDrawList *draw_list, ImU32 color) {
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();
    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    draw_list->AddCircleFilled(center, ImGui::GetFontSize() * 0.25f, color, 8);
}

void DrawBreakpointButton(uint16_t address, ImDrawList* draw_list) {
    char label[24] = {0};
    snprintf(label, sizeof(label), "-##break_%04X", address);

    bool breakpoint = false;
    bool bp_exist = DeviceResources::CPUBreak.count(address);
    if(bp_exist)
        breakpoint = DeviceResources::CPUBreak[address];
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::SmallButton(label)) {
        if(bp_exist && breakpoint)
            DeviceResources::CPUBreak.erase(address);
        else {
            DeviceResources::CPUBreak[address] = true;
            bp_exist = true;
            breakpoint = true;
        }
    }
    ImGui::PopStyleColor();

    if (ImGui::IsItemHovered() && !bp_exist) {
        // non-existent, color: #6e1b13
        DisassemblerView::DrawBreakpointCircle(draw_list, IM_COL32(110, 27, 19, 255));
    } else if(bp_exist) {
        if(breakpoint) // color: rgb(229, 20, 0)
            DisassemblerView::DrawBreakpointCircle(draw_list, DisassemblerView::breakpoint_activate);
        else // color: #848484
            DisassemblerView::DrawBreakpointCircle(draw_list, DisassemblerView::breakpoint_disabled);
    }
}
}