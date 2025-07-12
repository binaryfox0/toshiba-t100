#ifndef DV_IMGUI_MODIFIED_HPP
#define DV_IMGUI_MODIFIED_HPP

namespace DisassemblerView
{
    // Strpped from imgui internal, commit 89b5a2c3d50e4ca6a0a88378f096d7d05ff1c962
    // Default flag ImGuiSelectableFlags_AllowItemOverlap/AllowOverlap (renamed), customized version of ImGuiSelectableFlags_SpanAllColumns
    bool Selectable(const char* label, bool selected);
}

#endif