#include "DisassemblerView/Main.hpp"

#include "DisassemblerView/Core.hpp"
#include "DisassemblerView/InstructionTable.hpp"
#include "DisassemblerView/UnselectableTable.hpp"

#include "imgui.h"

static float scroll_synced = 0.0f;

namespace DisassemblerView
{
void Init(const uint8_t* disassembler_memory, const uint32_t size, const uint16_t load_address)
{
    SetupDisassembler(disassembler_memory, size, load_address);
    InitInstructionTable();
}

void Draw()
{
    ImGui::Begin("Disassembler");
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("##disassembler_content", ImVec2(), ImGuiChildFlags_Borders);
    DrawUnselectableTable();
    ImGui::SameLine();
    DrawInstructionTable();
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::End();
}

float& GetScrollSynced() {
    return scroll_synced;
}
};