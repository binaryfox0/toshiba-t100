#include "SidePanel.hpp"

#include <vector>
#include <algorithm>

#include "DisassemblerView/InstructionTable.hpp"
#include "DisassemblerView/UnselectableTable.hpp"
#include "DeviceResources.hpp"
#include "Internal.h"
#include "UIHelpers.hpp"

#include "imgui.h"

static std::vector<std::pair<const uint16_t, bool>*> bpoints_sorted;
void CheckBreakpointListChanged()
{
    static size_t last_size = 0;
    size_t current_size = DeviceResources::CPUBreak.size();

    if (last_size != current_size) {
        bpoints_sorted.clear();
        bpoints_sorted.reserve(DeviceResources::CPUBreak.size());

        for (auto& entry : DeviceResources::CPUBreak) {
            bpoints_sorted.push_back(&entry); // store pointer to map entry
        }

        std::sort(bpoints_sorted.begin(), bpoints_sorted.end(),
                  [](const auto* a, const auto* b) {
                      return a->first < b->first;
                  });
        last_size = current_size;
    }
}

void DrawSidePanel()
{
    CheckBreakpointListChanged();

    ImGui::SetNextWindowSize(ImVec2(314, 460), ImGuiCond_FirstUseEver);
    ImGui::Begin("Side panel demo");

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));

    if(ImGui::BeginTable("##breakpoints_table", 3))
    {
        ImGui::TableSetupColumn("##breakpoint_disable-able_col", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##breakpoint_disabled_checkbox-col", ImGuiTableColumnFlags_WidthFixed);

        ImGuiListClipper clipper;
        clipper.Begin(bpoints_sorted.size());
        while(clipper.Step())
        {
            for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                const auto pair = bpoints_sorted[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("-");
                DisassemblerView::DrawBreakpointCircle(draw_list, pair->second ? DisassemblerView::breakpoint_activate : DisassemblerView::breakpoint_disabled);
                ImGui::TableNextColumn();
                ImGui::PushID(pair->first);
                ImGui::Checkbox("##checkbox", &pair->second);
                ImGui::PopID();
                ImGui::TableNextColumn();
                if(DrawHyperlinkButton((to_hex(pair->first)).c_str()))
                {
                    DisassemblerView::UpdateDisplayRange(pair->first, true);
                    DisassemblerView::FocusAddress(pair->first, false);
                }
                // ImGui::Text("%04X", pair->first);
            }
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::End();
}