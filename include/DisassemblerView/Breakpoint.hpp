#ifndef DV_BREAKPOINT_HPP
#define DV_BREAKPOINT_HPP

#include "imgui.h"

#include <stdint.h>

namespace DisassemblerView
{
    // constant
    constexpr ImU32 breakpoint_disabled = IM_COL32(132, 132, 132, 255);
    constexpr ImU32 breakpoint_activate = IM_COL32(229, 20, 0, 255);
    
    void DrawBreakpointCircle(ImDrawList *draw_list, ImU32 color);
    void DrawBreakpointButton(uint16_t address, ImDrawList* draw_list);
};

#endif