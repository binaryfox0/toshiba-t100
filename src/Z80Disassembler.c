#include "Z80Disassembler.h"
#include <stdio.h>
#include <stdbool.h>

const char* inst_mnemonics[] =
{
    "nop", "ld", "inc", "dec", "rlca", "ex", "add", "rrca",
    "djnz", "rla", "jr", "rra",
    "daa", "cpl",
    "scf", "ccf",
    "halt",
    "adc",
    "sub", "sbc",
    "and", "xor",
    "or", "cp",
    "ret", "pop", "jp", "call", "push", "rst",
    "out", "exx", "in",
    "di", "ei", 

    "rlc", "rrc",
    "rl", "rr",
    "sla", "sra",
    "sll", "srl",
    "bit",
    "res",
    "set",

    "in0", "tst", "out0",
    "neg", "retn", "im", "mlt", "reti",
    "rrd", "rld",
    "tstio", "slp",
    "otim", "otdm",
    "otimr", "otdmr",
    "ldi", "cpi", "ini", "outi", "ldd", "cpd", "ind", "outd",
    "ldir", "cpir", "inir", "otir", "lddr", "cpdr", "indr", "otdr",

    "Non-existent"
};

const uint8_t inst_mnemonics_index[] = {
    /*       0     1   2    3    4     5     6      7    8    9    A    B    C     D     E    F */
    /* 0 */ NOP,  LD, LD,  INC, INC,  DEC,  LD,   RLCA, EX,  ADD, LD,  DEC, INC,  DEC,  LD,  RRCA,
    /* 1 */ DJNZ, LD, LD,  INC, INC,  DEC,  LD,   RLA,  JR,  ADD, LD,  DEC, INC,  DEC,  LD,  RRA,
    /* 2 */ JR,   LD, LD,  INC, INC,  DEC,  LD,   DAA,  JR,  ADD, LD,  DEC, INC,  DEC,  LD,  CPL,
    /* 3 */ JR,   LD, LD,  INC, INC,  DEC,  LD,   SCF,  JR,  ADD, LD,  DEC, INC,  DEC,  LD,  CCF,
    /* 4 */ LD,   LD, LD,  LD,  LD,   LD,   LD,   LD,   LD,  LD,  LD,  LD,  LD,   LD,   LD,  LD,
    /* 5 */ LD,   LD, LD,  LD,  LD,   LD,   LD,   LD,   LD,  LD,  LD,  LD,  LD,   LD,   LD,  LD,
    /* 6 */ LD,   LD, LD,  LD,  LD,   LD,   LD,   LD,   LD,  LD,  LD,  LD,  LD,   LD,   LD,  LD,
    /* 7 */ LD,   LD, LD,  LD,  LD,   LD,   HALT, LD,   LD,  LD,  LD,  LD,  LD,   LD,   LD,  LD,
    /* 8 */ ADD, ADD, ADD, ADD, ADD,  ADD,  ADD,  ADD,  ADC, ADC, ADC, ADC, ADC,  ADC,  ADC, ADC,
    /* 9 */ SUB, SUB, SUB, SUB, SUB,  SUB,  SUB,  SUB,  SBC, SBC, SBC, SBC, SBC,  SBC,  SBC, SBC,
    /* A */ AND, AND, AND, AND, AND,  AND,  AND,  AND,  XOR, XOR, XOR, XOR, XOR,  XOR,  XOR, XOR, 
    /* B */ OR,  OR,  OR,  OR,  OR,   OR,   OR,   OR,   CP,  CP,  CP,  CP,  CP,   CP,   CP,  CP,
    /* C */ RET, POP, JP,  JP,  CALL, PUSH, ADD,  RST,  RET, RET, JP,  NON, CALL, CALL, ADC, RST,
    /* D */ RET, POP, JP,  OUT, CALL, PUSH, SUB,  RST,  RET, EXX, JP,  IN,  CALL, NON,  SBC, RST,
    /* E */ RET, POP, JP,  EX,  CALL, PUSH, AND,  RST,  RET, JP,  JP,  EX,  CALL, NON,  XOR, RST,
    /* F */ RET, POP, JP,  DI,  CALL, PUSH, OR,   RST,  RET, LD,  JP,  EI,  CALL, NON,  CP,  RST
};

const uint8_t cb_instr_mnemonics_index[] = {
    /*       0     1   2    3    4    5    6    7    8    9    A    B    C    D    E    F */
    /* 0 */ RLC, RLC, RLC, RLC, RLC, RLC, RLC, RLC, RRC, RRC, RRC, RRC, RRC, RRC, RRC, RRC,
    /* 1 */ RL,  RL,  RL,  RL,  RL,  RL,  RL,  RL,  RR,  RR,  RR,  RR,  RR,  RR,  RR,  RR,
    /* 2 */ SLA, SLA, SLA, SLA, SLA, SLA, SLA, SLA, SRA, SRA, SRA, SRA, SRA, SRA, SRA, SRA,
    /* 3 */ SLL, SLL, SLL, SLL, SLL, SLL, SLL, SLL, SRL, SRL, SRL, SRL, SRL, SRL, SRL, SRL,
    /* 4 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
    /* 5 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
    /* 6 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
    /* 7 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
    /* 8 */ RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES,
    /* 9 */ RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES,
    /* A */ RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES,
    /* B */ RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES, RES,
    /* C */ SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET,
    /* D */ SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET,
    /* E */ SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET,
    /* F */ SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET, SET
};

const uint8_t ed_instr_mnemonics_index[] = {
    IN0,  OUT0, NON,  NON,   TST,   NON,  NON, NON, IN0,  OUT0, NON,  NON,   TST, NON,  NON, NON,
    IN0,  OUT0, NON,  NON,   TST,   NON,  NON, NON, IN0,  OUT0, NON,  NON,   TST, NON,  NON, NON,
    IN0,  OUT0, NON,  NON,   TST,   NON,  NON, NON, IN0,  OUT0, NON,  NON,   TST, NON,  NON, NON,
    NON,  NON,  NON,  NON,   TST,   NON,  NON, NON, IN0,  OUT0, NON,  NON,   TST, NON,  NON, NON,
    IN,   OUT,  SBC,  LD,    NEG,   RETN, IM,  LD,  IN,   OUT,  ADC,  LD,    MLT, RETI, NON, LD,
    IN,   OUT,  SBC,  LD,    NON,   NON,  IM,  LD,  IN,   OUT,  ADC,  LD,    MLT, NON,  IM,  LD,
    IN,   OUT,  SBC,  LD,    TST,   NON,  NON, RRD, IN,   OUT,  ADC,  LD,    MLT, NON,  NON, RLD,
    IN,   OUT,  SBC,  LD,    TSTIO, NON,  SLP, NON, IN,   OUT,  ADC,  LD,    MLT, NON,  NON, NON,
    NON,  NON,  NON,  OTIM,  NON,   NON,  NON, NON, NON,  NON,  NON,  OTDM,  NON, NON,  NON, NON,
    NON,  NON,  NON,  OTIMR, NON,   NON,  NON, NON, NON,  NON,  NON,  OTDMR, NON, NON,  NON, NON,
    LDI,  CPI,  INI,  OUTI,  NON,   NON,  NON, NON, LDD,  CPD,  IND,  OUTD,  NON, NON,  NON, NON,
    LDIR, CPIR, INIR, OTIR,  NON,   NON,  NON, NON, LDDR, CPDR, INDR, OTDR,  NON, NON,  NON, NON
};

const uint8_t inst_len[] = {
    /*      0 1 2 3 4 5 6 7 8 9 A B C D E F */
    /* 0 */ 1,3,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
    /* 1 */ 2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
    /* 2 */ 2,3,3,1,1,1,2,1,2,1,3,1,1,1,2,1,
    /* 3 */ 2,3,3,1,1,1,2,1,2,1,3,1,1,1,2,1,
    /* 4 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* 5 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* 6 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* 7 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* 8 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* 9 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* A */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* B */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    /* C */ 1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
    /* D */ 1,1,3,2,3,1,2,1,1,1,3,2,3,0,2,1,
    /* E */ 1,1,3,1,3,1,2,1,1,1,3,1,3,0,2,1,
    /* F */ 1,1,3,1,3,1,2,1,1,1,3,1,3,0,2,1

};

const uint8_t ed_instr_len[] = {
    3, 3, 0, 0, 2, 0, 0, 0, 3, 3, 0, 0, 2, 0, 0, 0,
    3, 3, 0, 0, 2, 0, 0, 0, 3, 3, 0, 0, 2, 0, 0, 0,
    3, 3, 0, 0, 2, 0, 0, 0, 3, 3, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 2, 0, 0, 0, 3, 3, 0, 0, 2, 0, 0, 0,
    2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 0, 2,
    2, 2, 2, 4, 0, 0, 2, 2, 2, 2, 2, 4, 2, 0, 2, 2,
    2, 2, 2, 4, 3, 0, 0, 2, 2, 2, 2, 4, 2, 0, 0, 2,
    2, 2, 2, 4, 3, 0, 2, 0, 2, 2, 2, 4, 2, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
    2, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0,
    2, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0
};

enum {
    A,
    B,
    C,  // 0b10
    D,
    E,
    H,
    L,
    AF,
    BC, // 0b100
    DE,
    HL,
    Z,
    NZ,
    NC,
    SP,     // 0b00001110
    PO,
    PE,     // 0b00010000
    P,
    M,
    N8       = 0b00100000,
    N16      = 0b01000000,
    NUM      = 0b01100000,
    ADDR     = 0b10000000,
    HEX      = ADDR,
    // N16 | ADDR does exist
    SIGNED8  = N8 | B,
    NONE = 0xff
};

char* regs_str[] = { 
    "a", "b", "c", "d", "e", "h", "l",
    "af", "bc", "de", "hl", "z", "nz", "nc", "sp", "po", "pe",
    "p", "m"
};
const uint8_t insts_operand[][2] = {
    // 00 - 0F
    {NONE,      NONE},         {BC,        N16},  {BC | ADDR,  A},          {BC,        NONE},
    {B,         NONE},         {B,         NONE}, {B,          N8},         {NONE,      NONE},
    {AF,        AF},           {HL,        BC},   {A,          BC | ADDR},  {BC,        NONE},
    {C,         NONE},         {C,         NONE}, {C,          N8},         {NONE,      NONE},
    // 10 - 1F
    {SIGNED8,   NONE},         {DE,        N16},  {DE | ADDR,  A},          {DE,        NONE},
    {D,         NONE},         {D,         NONE}, {D,          N8},         {NONE,      NONE},
    {D,         SIGNED8},      {HL,        DE},   {A,          DE | ADDR},  {DE,        NONE},
    {E,         NONE},         {E,         NONE}, {E,          N8},         {NONE,      NONE},
    // 20 - 2F
    {NZ,        SIGNED8},      {HL,        N16},  {N16 | ADDR, HL},         {HL,        NONE},
    {H,         NONE},         {H,         NONE}, {H,          N8},         {NONE,      NONE},
    {Z,         SIGNED8},      {HL,        HL},   {HL,         N16 | ADDR}, {HL,        NONE},
    {L,         NONE},         {L,         NONE}, {L,          N8},         {NONE,      NONE},
    // 30 - 3F
    {NC,        SIGNED8},      {SP,        N16},  {N16 | ADDR, A},          {SP,        NONE},
    {HL | ADDR, NONE},         {HL | ADDR, NONE}, {HL  | ADDR, N8},         {NONE,      NONE},
    {C,         SIGNED8},      {HL,        SP},   {A,          N16 | ADDR}, {SP,        NONE}, 
    {A,         NONE},         {A,         NONE}, {A,          N8},         {NONE,      NONE},
    // 40 - 4F
    {B,         B},            {B,         C},    {B,          D},          {B,         E},
    {B,         H},            {B,         L},    {B,          HL | ADDR},  {B,         A},
    {C,         B},            {C,         C},    {C,          D},          {C,         E},
    {C,         H},            {C,         L},    {C,          HL | ADDR},  {C,         A},
    // 50 - 5F
    {D,         B},            {D,         C},    {D,          D},          {D,         E},
    {D,         H},            {D,         L},    {D,          HL | ADDR},  {D,         A},
    {E,         B},            {E,         C},    {E,          D},          {E,         E},
    {E,         H},            {E,         L},    {E,          HL | ADDR},  {E,         A},
    // 60 - 6F
    {H,         B},            {H,         C},    {H,          D},          {H,         E},
    {H,         H},            {H,         L},    {H,          HL | ADDR},  {H,         A},
    {L,         B},            {L,         C},    {L,          D},          {L,         E},
    {L,         H},            {L,         L},    {L,          HL | ADDR},  {L,         A},
    // 70 - 7F
    {HL | ADDR, B},            {HL | ADDR, C},    {HL  | ADDR, D},          {HL | ADDR, E},
    {HL | ADDR, H},            {HL | ADDR, L},    {NONE,       NONE},       {HL | ADDR, A},
    {A,         B},            {A,         C},    {A,          D},          {A,         E},
    {A,         H},            {A,         L},    {A,          HL | ADDR},  {A,         A},
    // 80 - 8F
    {A,         B},            {A,         C},    {A,          D},          {A,         E},
    {A,         H},            {A,         L},    {A,          HL | ADDR},  {A,         A},
    {A,         B},            {A,         C},    {A,          D},          {A,         E},
    {A,         H},            {A,         L},    {A,          HL | ADDR},  {A,         A},
    // 90 - 9F
    {B,         NONE},         {C,         NONE}, {D,          NONE},       {E,         NONE},
    {H,         NONE},         {L,         NONE}, {HL | ADDR,  NONE},       {A,         NONE},
    {A,         B},            {A,         C},    {A,          D},          {A,         E},
    {A,         H},            {A,         L},    {A,          HL | ADDR},  {A,         A},
    // A0 - AF
    {B,         NONE},         {C,         NONE}, {D,          NONE},       {E,         NONE},
    {H,         NONE},         {L,         NONE}, {HL | ADDR,  NONE},       {A,         NONE},
    {B,         NONE},         {C,         NONE}, {D,          NONE},       {E,         NONE},
    {H,         NONE},         {L,         NONE}, {HL | ADDR,  NONE},       {A,         NONE},
    // B0 - BF
    {B,         NONE},         {C,         NONE}, {D,          NONE},       {E,         NONE},
    {H,         NONE},         {L,         NONE}, {HL | ADDR,  NONE},       {A,         NONE},
    {B,         NONE},         {C,         NONE}, {D,          NONE},       {E,         NONE},
    {H,         NONE},         {L,         NONE}, {HL | ADDR,  NONE},       {A,         NONE},
    // C0 - CF
    {NZ,        NONE},         {BC,        NONE}, {NZ,         N16},        {N16,       NONE},
    {NZ,        N16},          {BC,        NONE}, {A,          N8},         {HEX | NUM, NONE},
    {Z,         NONE},         {NONE,      NONE}, {Z,          N16},        {NONE,      NONE},
    {Z,         N16},          {N16,       NONE}, {A,          N8},         {7|HEX|NUM, NONE},
    // D0 - DF
    {NC,        NONE},         {DE,        NONE}, {NC,         N16},        {N8 | ADDR, A},
    {NC,        N16},          {DE,        NONE}, {N8,         NONE},       {8|HEX|NUM, NONE},
    {C,         NONE},         {NONE,      NONE}, {C,          N16},        {A,         N8 | ADDR},
    {C,         N16},          {NONE,      NONE}, {A,          N8},         {15|HEX|NUM,NONE},
    // E0 - EF
    {PO,        NONE},         {HL,        NONE}, {PO,         N16},        {SP | ADDR, HL},
    {PO,        N16},          {HL,        NONE}, {N8,         NONE},       {16|HEX|NUM,NONE},
    {PE,        NONE},         {HL | ADDR, NONE}, {PE,         N16},        {DE,        HL},
    {PE,        N16},          {NONE,      NONE}, {N8,         NONE},       {23|HEX|NUM,NONE},
    // F0 - FF
    {P,         NONE},         {AF,        NONE}, {P,          N16},        {NONE,      NONE},
    {P,         N16},          {AF,        NONE}, {N8,         NONE},       {24|HEX|NUM,NONE},
    {M,         NONE},         {SP,        HL},   {M,          N16},        {NONE,      NONE},
    {M,         N16},          {NONE,      NONE}, {N8,         NONE},       {31|HEX|NUM,NONE}
};


const uint64_t insts_operand_size = sizeof(insts_operand) / sizeof(insts_operand[0]);
INLINE uint8_t extract_number(uint8_t operand_inst) { return operand_inst & 0b01100000; }
INLINE bool is_number(uint8_t operand_inst) { return extract_number(operand_inst) > 0b00011111; }

INLINE void operand_number(char* buffer, size_t maxlen, const uint8_t* operand_bytes, uint8_t operand_inst)
{
    uint8_t n = operand_inst & 0b01100000;
    bool sign = operand_inst & B; 
    bool addr = operand_inst & ADDR;
    switch(n)
    {
    case N8: {
        if(sign) {
            if(addr) snprintf(buffer, maxlen, "(%d)", operand_bytes[1]);
            else     snprintf(buffer, maxlen, "%d", operand_bytes[1]);
        } else {
            if(addr) snprintf(buffer, maxlen, "(0x%02X)", operand_bytes[1]);
            else     snprintf(buffer, maxlen, "0x%02X", operand_bytes[1]);
        }
        break;
    }
    case N16: {
        if(addr) snprintf(buffer, maxlen, "(0x%02X)", *(uint16_t*)&operand_bytes[1]);
        else     snprintf(buffer, maxlen, "0x%02X",   *(uint16_t*)&operand_bytes[1]);
        break;
    }
    case NUM: {
        uint8_t first = (operand_inst & 0b111);
        first += !!(operand_inst & HEX) && first;
        uint8_t second = ((operand_inst & 0b00011000) >> 3) & 0b11;
        snprintf(buffer, maxlen, "%c%c%s", second + '0', first + '0', operand_inst & HEX ? "h" : "");
        break;
    }
    }
}

INLINE void operand_to_str(char* buffer, size_t maxlen, const uint8_t *operand_bytes, uint8_t operand_inst, bool src)
{
    if(operand_inst == NONE && src)
        return;
    if(is_number(operand_inst))
        operand_number(buffer, maxlen, operand_bytes, operand_inst);
    else {
        if(operand_inst & ADDR) snprintf(buffer, maxlen, "(%s)", regs_str[operand_inst & 0b11111]);
        else                    snprintf(buffer, maxlen, "%s%s", regs_str[operand_inst & 0b11111], src && operand_bytes[0] == 0x08 ? "'" : "");
    }
}

void z80_get_operands(const uint8_t* opcode, char* buffer, uint64_t maxlen)
{
    const uint8_t* inst_operand = insts_operand[opcode[0]];
    if(inst_operand[0] == NONE && opcode[0] != 0xff)
        return;
    char buffer1[16] = {0};
    char buffer2[16] = {0};
    operand_to_str(buffer1, sizeof(buffer1), opcode, inst_operand[0], false);
    operand_to_str(buffer2, sizeof(buffer2), opcode, inst_operand[1], true);

    snprintf(buffer, maxlen, "%s%s%s", buffer1, *buffer2 ? ", " : "", *buffer2 ? buffer2 : "");
}
