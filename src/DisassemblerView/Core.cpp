#include "DisassemblerView/Core.hpp"

// C++ standard header
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stack>

// C header
#include "Internal.h"
#include "Z80Disassembler.h"

// C++ header
#include "EventLog.hpp"

static const uint8_t *mem = 0;
static __Vec2 bytes_range = {0};

static std::unordered_map<uint16_t, DisassembleInstr> parsed;
static std::unordered_map<uint16_t, uint16_t> parsed_jump_address;

static std::stack<uint16_t> ret_addr;
void Disassemble(uint16_t address)
{
    std::queue<uint16_t> more_address;
    std::unordered_set<uint16_t> visited;

    while (parsed.count(address) == 0 && visited.count(address) == 0)
    {
        visited.insert(address);

        if (address < bytes_range.x || address >= bytes_range.y)
            break;

        const uint8_t* instr = mem + address;
        uint8_t len = z80_get_instr_len(instr);
        DisassembleInstr& out = parsed[address];
        out.address = address;
        out.size = len;
        out.type = z80_get_type(instr);
        z80_get_operands(instr, out.operands[0], sizeof(out.operands[0]),
                                  out.operands[1], sizeof(out.operands[1]));

        switch (out.type)
        {
        case JP: {
            uint16_t jump_address = *(uint16_t*)(instr + 1);
            AddNewEvent("Disassembler: JP to " + to_hex(jump_address));
            parsed_jump_address[address] = jump_address;
            if (instr[0] == 0xC3) {
                out.flags |= FLAGS_JUMP_1;
                address = jump_address;
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(jump_address);
            break;
        }

        case JR: {
            int8_t rel = *(int8_t*)(instr + 1);
            uint16_t jump_address = address + 2 + rel;
            AddNewEvent("Disassembler: JR to " + to_hex(jump_address));
            parsed_jump_address[address] = jump_address;
            out.flags |= FLAGS_JUMP_RELATIVE;
            if (instr[0] == 0x18) {
                out.flags |= FLAGS_JUMP_1;
                address = jump_address;
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(jump_address);
            break;
        }

        case CALL: {
            uint16_t jump_address = *(uint16_t*)(instr + 1);
            AddNewEvent("Disassembler: CALL " + to_hex(jump_address));
            parsed_jump_address[address] = jump_address;
            out.flags |= instr[0] == 0xCD ? FLAGS_JUMP_1 : FLAGS_JUMP_2;
            if (parsed.count(jump_address) == 0) {
                if (instr[0] == 0xCD) {
                    ret_addr.push(address + len);
                    address = jump_address;
                    continue;
                }
                more_address.push(jump_address);
            }
            break;
        }

        case RET: {
            if(instr[0] != 0xC9) // normal ret, otherwise conditional ret
                break;
            if (ret_addr.empty()) {
                AddNewEvent("Disassembler: Empty return address stack!");
                break;
            }
            address = ret_addr.top();
            ret_addr.pop();
            AddNewEvent("Disassembler: RET to " + to_hex(address));
            continue;
        }
        }

        address += len;
    }

    while (!more_address.empty()) {
        uint16_t next = more_address.front();
        more_address.pop();
        if (!parsed.count(next))
            Disassemble(next);
    }
}

namespace DisassemblerView
{
void SetupDisassembler(const uint8_t* disassembler_memory, const uint32_t size, const uint16_t load_address)
{
    mem = disassembler_memory;
    bytes_range.y = size;
    bytes_range.x = load_address;

    Disassemble(load_address);
    // printf("%04X (%d)\n", parsed_jump_address[0xd000], parsed_jump_address[0xd000]);

}

const std::unordered_map<uint16_t, DisassembleInstr>& GetDisassemblerParsedGeneral() {
    return parsed;
}

const std::unordered_map<uint16_t, uint16_t>& GetDisassemblerParsedJumpAddress() {
    return parsed_jump_address;
}

const uint8_t*& GetDisassemblerMemory() {
    return mem;
}

const __Vec2& GetDisassemblerBytesRange() {
    return bytes_range;
}
};
