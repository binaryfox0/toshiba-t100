#ifndef DISASSEMBLER_VIEW_INSTRUCTION_TABLE_HPP
#define DISASSEMBLER_VIEW_INSTRUCTION_TABLE_HPP

#include <stdint.h>

#include <vector>

struct display_instr {
    bool is_separator;
    uint16_t address;
};

namespace DisassemblerView
{
void UpdateDisplayRange(uint16_t address, bool push);
void InitInstructionTable();
void DrawInstructionTable();

void FocusAddress(uint16_t address, bool highlight);

const std::vector<display_instr>& GetInstructionDisplayRange();
std::vector<uint16_t>& GetDisassemblerViewHistory();
};

#endif