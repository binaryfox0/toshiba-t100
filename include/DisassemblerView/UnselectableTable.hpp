#ifndef DISASSEMBLER_VIEW_UNSELECTABLE_TABLE_HPP
#define DISASSEMBLER_VIEW_UNSELECTABLE_TABLE_HPP

#include "imgui.h"

namespace DisassemblerView
{
void DrawUnselectableTable();
void DrawBreakpointCircle(ImDrawList *draw_list, ImU32 color);
};

#endif