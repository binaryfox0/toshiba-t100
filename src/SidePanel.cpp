#include "SidePanel.hpp"

#include <cstdint>
#include <unordered_map>

#include "DeviceResources.hpp"

#include "DisassemblerView/UnselectableTable.hpp"
#include "imgui.h"

static std::unordered_map<uint16_t, bool> disabled_breakpoint;

void DrawSidePanel()
{
    ImGui::SetNextWindowSize(ImVec2(314, 460), ImGuiCond_FirstUseEver);
    ImGui::Begin("Side panel demo");
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    if(ImGui::BeginTable("##breakpoints_table", 3))
    {
        ImGui::TableSetupColumn("##breakpoint_disable-able_col", ImGuiTableColumnFlags_WidthFixed);

        for(const auto& pair : DeviceResources::CPUBreak) {
            if(!pair.second)
                continue;
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("-");
            DisassemblerView::DrawBreakpointCircle(draw_list, IM_COL32(132, 132, 132, 255));
            ImGui::TableNextColumn();
            ImGui::PushID(pair.first);
            bool state = false;
            ImGui::Checkbox("##checkbox", &state);
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::Text("%04X", pair.first);
        }
        ImGui::EndTable();
    } 
    ImGui::PopStyleVar();
    ImGui::End();
}