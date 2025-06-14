#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <stack>

#include "Z80Disassembler.h"

INLINE long min(long a, long b) {
    return (a < b) ? a : b;
}

int main()
{
    long load_adress = 0xd000;
    long file_offset = 0x1000;

    uint8_t memory[65536] = {0};
    FILE* file = fopen("/home/binaryfox0/proj/toshiba-t100-pc/images/TDISKBASIC.img", "rb");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, file_offset, SEEK_SET);
    fread(&memory[load_adress], 1, min(file_size - file_offset, sizeof(memory) - load_adress), file);

    std::stack<uint16_t> addresses;
    uint16_t PC = load_adress;
    while(1)
    {
        uint8_t *instr = memory + PC;
        uint8_t type = z80_get_type(instr);
        char operands[16] = {0};
        z80_get_operands(instr, operands, sizeof(operands));
        uint8_t len = z80_get_instr_len(instr);
        if(type == JP) {
            PC = strtol(operands, 0, 16);
            continue;
        } else if(type == CALL) {
            printf("CALL. Operands: %s\n", operands);
            addresses.push(PC + len);
            PC = strtol(operands, 0, 16);
            continue;
        } else if(type == RET) {
            if(addresses.empty()) {
                printf("Something was wrong\n");
                break;
            }
            PC = addresses.top();
            addresses.pop();
            continue;
        }
        printf("%s %s\n", inst_mnemonics[type], operands);
        PC += len;
    }
}