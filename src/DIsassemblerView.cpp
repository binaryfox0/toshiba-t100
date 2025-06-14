#include "DisassemblerView.hpp"

#include <cstdint>
#include <cstdio>
#include <stack>
#include <imgui.h>
#include <string>

#include "Z80Disassembler.h"

#define SIZEOF_ARR(arr) (sizeof(arr) / sizeof(arr[0]))

bool show_disassembler = true;
struct DisassembleInstr {
    uint16_t address;
    uint8_t size;
    const char* mnemonics;
    char operands[16];
    bool jump;
};
const uint8_t *memory = 0;
static DisassembleInstr cache[32] = {0};
static const int cache_size = SIZEOF_ARR(cache);
static std::stack<uint16_t> addresses;

void Disassemble(uint16_t address);

void DrawDisassembleRow(int index)
{
    const DisassembleInstr& row = cache[index];
    ImGui::SameLine();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%04X", row.address);
    char tmp[12] = {0};
    for(int i = 0; i < row.size - 1; i++)
        sprintf(tmp + (i * 3), "%02X ", memory[row.address + i]);
    sprintf(tmp + ((row.size - 1) * 3), "%02X", memory[row.address + (row.size - 1)]);
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", tmp);
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%s", row.mnemonics);
    ImGui::TableSetColumnIndex(3);
    if(row.jump) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        if(ImGui::SmallButton(row.operands))
            Disassemble(std::stoi(row.operands, nullptr, 16));
        ImGui::PopStyleColor(3);
    }
    else
        ImGui::Text("%s", row.operands);
}

void DisassembleWhenNeed()
{
    
}

void DrawDisassemblerView()
{
    float button_height = ImGui::GetFrameHeight();
    // Undo button
    if(addresses.size() == 1)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(154, 154, 154, 255));
        ImGui::Button("<", ImVec2(button_height, button_height));
        ImGui::PopStyleColor(3);
    } else {
        if(ImGui::Button("<", ImVec2(button_height, button_height)))
        {
            addresses.pop();
            uint16_t new_address = addresses.top();
            Disassemble(new_address);
        }
    }
    ImGui::BeginChild("ScrollReigon", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    if (ImGui::BeginTable("MyTable", 4,
        ImGuiTableFlags_SizingStretchProp     // Optional: make columns stretch
    )) {
        // Optional: Set column widths or behavior
        ImGui::TableSetupColumn("##col1"); // Hidden name, no header
        ImGui::TableSetupColumn("##col2");
        ImGui::TableSetupColumn("##col3");

        // Temporarily override style for hover + selection
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_Header,       ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));

        float rowHeight = ImGui::GetTextLineHeightWithSpacing();
        float scrollY = ImGui::GetScrollY();
        int visible_start_row = (int)(scrollY / rowHeight);

        printf("First visible row: %d\n", visible_start_row);
        
        //for (int row = 0; row < cache_size; ++row) {
        //    ImGui::TableNextRow();
//
        //    // Span the row by beginning at column 0
        //    ImGui::TableSetColumnIndex(0);
        //    ImGui::PushID(row); // Unique ID per row
        //    if (ImGui::Selectable("##row_select", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 0))) {
        //        // Row clicked
        //    }
        //    ImGui::PopID();
//
        //    DrawDisassembleRow(row);
        //}

        ImGui::PopStyleColor(2); // Restore hover/active/header colors
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void Disassemble(uint16_t address)
{
    addresses.push(address);
    memset(cache, 0, sizeof(cache));
    for(int i = 0; i < cache_size; i++)
    {
        const uint8_t* opcode = memory + address;
        DisassembleInstr &row = cache[i];
        row.address = address;
        uint8_t len = z80_get_instr_len(opcode);
        row.size = len;
        row.mnemonics = z80_get_mnemonic(opcode);
        z80_get_operands(opcode, row.operands, sizeof(row.operands));
        row.jump = inst_mnemonics_index[*opcode] == JP;
        address += len;
    }
}

void SetupDisassembler(const uint8_t* memory_in, uint16_t address)
{
    memory = memory_in;
    Disassemble(address);
}