#ifndef DISASSEMBLER_VIEW_HPP
#define DISASSEMBLER_VIEW_HPP

#include <cstdint>
extern bool show_disassembler;
void DrawDisassemblerView();

void SetupDisassembler(const uint8_t* memory_in, uint16_t address);

#endif