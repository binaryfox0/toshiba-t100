#ifndef DISASSEMBLER_VIEW_CORE_HPP
#define DISASSEMBLER_VIEW_CORE_HPP

#include <stdint.h>

#include <unordered_map>

#include "Internal.h"


enum DisassembleInstrFlags {
    FLAGS_NONE,
    FLAGS_JUMP_1 = 1,
    FLAGS_JUMP_2 = 2,
    FLAGS_JUMP_RELATIVE = 1 << 2
};

struct DisassembleInstr {
    uint16_t address;
    uint8_t size;
    uint8_t type;
    char operands[2][12]; 
    uint8_t flags;
};

namespace DisassemblerView
{
void SetupDisassembler(const uint8_t* disassembler_memory, const uint32_t size, const uint16_t load_address);

// MARK: Getter function
// return a hashmap contain general info of instruction at address
const std::unordered_map<uint16_t, DisassembleInstr>& GetDisassemblerParsedGeneral();
// return a hashmap contain address where instruction will be jumped to (jp, jr, call, etc, exclude ret and variations)
const std::unordered_map<uint16_t, uint16_t>& GetDisassemblerParsedJumpAddress();
// return pointer to memory that disassembler use to disassemble program
const uint8_t*& GetDisassemblerMemory();
// return range of the loaded data
const __Vec2& GetDisassemblerBytesRange();
};

#endif