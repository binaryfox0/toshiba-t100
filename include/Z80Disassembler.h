#ifndef Z80_DISASSEMBLER_H
#define Z80_DISASSEMBLER_H

#include <stdint.h>

#include "Internal.h"

#ifdef __cplusplus
extern "C"  {
#endif

#define Z80_MAX_INSTR_LEN 4

enum {
    NOP, LD, INC, DEC, RLCA, EX, ADD, RRCA,
    DJNZ, RLA, JR, RRA,
    DAA, CPL,
    SCF, CCF,
    HALT,
    ADC,
    SUB, SBC,
    AND, XOR,
    OR, CP,
    RET, POP, JP, CALL, PUSH, RST,
    OUT, EXX,
    IN,
    DI, EI,

    RLC, RRC,
    RL, RR,
    SLA, SRA,
    SLL, SRL,
    BIT,
    RES,
    SET,

    IN0, TST, OUT0,
    NEG, RETN, IM, MLT, RETI,
    RRD, RLD,
    TSTIO, SLP,
    OTIM, OTDM,
    OTIMR, OTDMR,
    LDI, CPI, INI, OUTI, LDD, CPD, IND, OUTD,
    LDIR, CPIR, INIR, OTIR, LDDR, CPDR, INDR, OTDR,

    NON // Non-existent
};

extern const char*   z80_instr_mnemonics[];
extern const uint8_t inst_mnemonics_index[];
extern const uint8_t cb_instr_mnemonics_index[];
extern const uint8_t ed_instr_mnemonics_index[];

extern const uint8_t inst_len[];
extern const uint8_t ed_instr_len[];

// for test
extern const uint64_t insts_operand_size;

INLINE uint8_t z80_get_type(const uint8_t* opcode) {
    if(*opcode == 0xCB)
        return cb_instr_mnemonics_index[opcode[1]];
    else if(*opcode == 0xED)
        return ed_instr_mnemonics_index[opcode[1]];
    else
        return inst_mnemonics_index[*opcode];
}

INLINE const char* z80_get_mnemonic_from_bytes(const uint8_t* opcode) {
    return z80_instr_mnemonics[z80_get_type(opcode)];
}

INLINE const char* z80_get_mnemonic_from_index(const uint8_t index) {
    return z80_instr_mnemonics[index];
}

INLINE uint8_t z80_get_instr_len(const uint8_t *instr) {
    if(*instr == 0xED)
        return ed_instr_len[instr[1]];
    else
        return inst_len[instr[0]];
}

void z80_get_operands(const uint8_t* opcode, char* out, uint64_t maxlen, char* out2, uint64_t maxlen2);

#ifdef __cplusplus
};
#endif

#endif