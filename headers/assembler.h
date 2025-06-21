#ifndef NEJLEPSI_ASSEMBLER
#define NEJLEPSI_ASSEMBLER

#include <stdint.h>
#include <sys/types.h>

#define WARN "\033[0;33mW\033[0m:"
#define ERR "\033[0;31mE\033[0m:"
#define LICO "%lu:%lu | "
#define PERRLICO(message, ...) fprintf(stderr, ERR LICO message, __VA_ARGS__)
#define PWARNLICO(message, ...) fprintf(stderr, WARN LICO message, __VA_ARGS__)

typedef enum { RAW, ELF } EOutFormat;

typedef enum {
	A,
	C,
	D,
	B,
	AH_SP,
	CH_BP,
	DH_SI,
	BH_DI,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15
} EGPRegister;

typedef enum { BYTE = 1, WORD = 2, DWORD = 4, QWORD = 8 } ESize;

#define ETOKEN_TYPE_COUNT 7
typedef enum {
	TKN_INSTRUCTION,
	TKN_IMMEDIATE,
	TKN_MEMACCESS,
	TKN_REGISTER,
	TKN_PREFIX,
	TKN_LABEL,
	TKN_OPERATOR
} ETokenType;

struct InstrFuncs;
struct Instruction {
	struct InstrFuncs *funcs;
};

struct Immediate {
	union {
		int64_t value64;
		int32_t value32;
		int16_t value16;
		int8_t value8;
	};
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

struct Prefix {};

struct Label {
	char *value;
};

struct Token {
	u_long column;
	union {
		MemAccess *mem;
		struct Instruction instr;
		struct Immediate imm;
		struct Register reg;
		struct Prefix prefix;
		struct Label label;
	};
	int8_t type;
};

struct Operand {};

struct InstrFuncs {
	int (*validate)(int cnt, struct Token tokens[cnt]);
	int16_t (*encode)(struct Instruction *instr);
};

#endif
