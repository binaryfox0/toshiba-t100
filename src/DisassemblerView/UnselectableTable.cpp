#include "DisassemblerView/UnselectableTable.hpp"

#include <cmath>


#include "DisassemblerView/Main.hpp"
#include "imgui.h"

#include "DisassemblerView/InstructionTable.hpp"
#include "DeviceResources.hpp"
#include "Internal.h"

static const auto& display_ranges = DisassemblerView::GetInstructionDisplayRange();

void DrawCircle(ImDrawList *draw_list, ImU32 color) {
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();
    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    draw_list->AddCircleFilled(center, ImGui::GetFontSize() * 0.2f, color, 8);
}

void DrawBreakpointButton(uint16_t address, ImDrawList* draw_list) {
    char label[24] = {0};
    snprintf(label, sizeof(label), "-##break_%04X", address);

    bool &breakpoint = DeviceResources::CPUBreak[address];
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::SmallButton(label)) {
        breakpoint = !breakpoint;
    }
    ImGui::PopStyleColor();

    if (ImGui::IsItemHovered() && !breakpoint) {
        DrawCircle(draw_list, IM_COL32(110, 27, 19, 255));
    } else if(breakpoint)
        DrawCircle(draw_list, IM_COL32(229, 20, 0, 255));
}

static int rows_count = 0;

static int rows_render_count = 0;
static int current_display_ranges_size = 0;
INLINE void ResizeCheck()
{
    static ImVec2 last_size = ImVec2();
    ImVec2 current_size = ImGui::GetWindowSize();
    if(current_size.x != last_size.x || current_size.y != last_size.y) {
        rows_count = std::round(ImGui::GetContentRegionAvail().y / ImGui::GetTextLineHeightWithSpacing());
        last_size = current_size;
    }

    static int last_display_ranges_size = 0;
    current_display_ranges_size = display_ranges.size();
    if(current_display_ranges_size != last_display_ranges_size) {
        rows_render_count = std::max(rows_count, current_display_ranges_size);
        last_display_ranges_size = current_display_ranges_size;
    }
}

// bruh need to be in beginframe to work
static bool initialized = false;
static float child_width = 0.0f; // need to have this in order to next child can be rendered
INLINE void InitUnselectableTable() {
    const ImGuiStyle& style = ImGui::GetStyle();
    child_width = 
        style.CellPadding.x * 2 + style.FramePadding.x * 2 + ImGui::CalcTextSize("-").x + // first col
        style.CellPadding.x * 2 + ImGui::CalcTextSize(">").x +
        1.0f * 3; // table border size, seems to hard-coded
}

static auto& scroll_synced = DisassemblerView::GetScrollSynced();
INLINE void ScrollSync()
{
    static float last_scroll = 0.0f;
    float current_scroll = ImGui::GetScrollY();
    if(std::abs(current_scroll - last_scroll) > 0.0f) {
        scroll_synced = current_scroll;
    } else {
        ImGui::SetScrollY(scroll_synced);
    }
    last_scroll = ImGui::GetScrollY();
}

namespace DisassemblerView
{
void DrawUnselectableTable()
{
    if(!initialized) {
        InitUnselectableTable();
        initialized = false;
    }
    ImGui::BeginChild("##unselctable_view", ImVec2(child_width + 1, 0), 0, ImGuiWindowFlags_NoScrollbar);
    ResizeCheck();
    ScrollSync();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if(ImGui::BeginTable("##unselectable_table", 2, ImGuiTableFlags_BordersV, ImVec2(child_width, 100)))
    {
        ImGui::TableSetupColumn("##breakpoints_col", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##current_pc_col", ImGuiTableColumnFlags_WidthFixed);

        ImGuiListClipper clipper;
        clipper.Begin(rows_render_count);

        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                if(i >= current_display_ranges_size) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(" ");
                    continue;
                }
                const display_instr& _instr = display_ranges[i];
                if(_instr.is_separator) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(" "); // extend the row height
                    continue;
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                DrawBreakpointButton(_instr.address, draw_list);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(_instr.address == DeviceResources::CPU.pc ? ">" : " ");
            }
        }
        clipper.End();

        ImGui::EndTable();
    }
    ImGui::EndChild();    
}
};