#ifndef DV_MAIN_HPP
#define DV_MAIN_HPP

#include <stdint.h>

namespace DisassemblerView {
    void Init(const uint8_t* disassembler_memory, const uint32_t size, const uint16_t load_address);
    void Draw(const float height);

    float& GetScrollSynced();
};

#endif