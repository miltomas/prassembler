#ifndef NEJLEPSI_ASSEMBLER
#define NEJLEPSI_ASSEMBLER

#include <stdint.h>

typedef enum { RAW, ELF } EOutFormat;

typedef enum {
    INSTRUCTION,
    IMMEDIATE,
    MEMACCESS,
    REGISTER,
    PREFIX,
    LABEL
} ETokenType;

struct Token {
    ETokenType type;
    void *value;
};

typedef enum {
    BYTE = 1,
    WORD = 2,
    DWORD = 4,
    QWORD = 8
} ESize;

typedef enum {
    A, C, D, B,
    AH_SP, CH_BP, DH_SI, BH_DI,
    R8, R9, R10, R11, R12, R13, R14, R15
} EGPRegister;

typedef struct {
    ESize target_size;
    ESize scale;
    EGPRegister index;
    EGPRegister base;
    int32_t displacement;
} MemAccess;

typedef struct {
    ESize size;
    EGPRegister type;
} Register;
/*
typedef struct {
        
} Instruction;
*/

#endif
