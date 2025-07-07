#ifndef DV_INSTR_TABLE_MULTISELECT
#define DV_INSTR_TABLE_MULTISELECT

#include <vector>

#include "imgui.h"

struct display_instr;
struct InstructionTableMultiselect : ImGuiSelectionBasicStorage {
    void WriteSelectionToClipboard(const std::vector<display_instr>& display_instrs);
};

#endif