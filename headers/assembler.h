#ifndef NEJLEPSI_ASSEMBLER
#define NEJLEPSI_ASSEMBLER

#include <stdint.h>

typedef enum { RAW, ELF } EOutFormat;

typedef enum {
    A, C, D, B,
    AH_SP, CH_BP, DH_SI, BH_DI,
    R8, R9, R10, R11, R12, R13, R14, R15
} EGPRegister;

typedef enum {
    BYTE = 1,
    WORD = 2,
    DWORD = 4,
    QWORD = 8
} ESize;

typedef enum {
    INSTRUCTION,
    IMMEDIATE,
    MEMACCESS,
    REGISTER,
    PREFIX,
    LABEL
} ETokenType;

struct InstrFuncs;
typedef struct {
    struct InstrFuncs *funcs;
    struct Token *operands;
    int operand_count;
} Instruction;

struct Immediate {
    int64_t value;
    ESize size;
};

typedef struct {
    ESize target_size;
    ESize scale;
    EGPRegister index;
    EGPRegister base;
    int32_t displacement;
} MemAccess;

struct Register {
    ESize size;
    EGPRegister type;
};

struct LegPrefixes {
    
};

struct Label {
    char *value;
};

struct Token {
    ETokenType type;
    union {
        Instruction *instr;
        struct Immediate imm;
        MemAccess *mem;
        struct Register reg;
        struct LegPrefixes prefix;
        struct Label label;
    };
};

struct Operand {
};

struct InstrFuncs {
    int (*validate)(int cnt, struct Token tokens[cnt]);
    int16_t (*encode)(Instruction *instr);
};

#endif
