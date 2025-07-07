#ifndef DV_UNSELECTABLE_TABLE_HPP
#define DV_UNSELECTABLE_TABLE_HPP

#include "imgui.h"

namespace DisassemblerView
{
// constant
constexpr ImU32 breakpoint_disabled = IM_COL32(132, 132, 132, 255);
constexpr ImU32 breakpoint_activate = IM_COL32(229, 20, 0, 255);

void DrawUnselectableTable();
void DrawBreakpointCircle(ImDrawList *draw_list, ImU32 color);
};

#endif