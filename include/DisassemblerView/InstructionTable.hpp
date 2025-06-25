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
void InitInstructionTable();
void DrawInstructionTable();

const std::vector<display_instr>& GetInstructionDisplayRange();
};

#endif