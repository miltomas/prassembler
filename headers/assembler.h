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
	REG_S,
	REG_x87,
	REG_MMX,
	REG_XMM,
	REG_YMM,
	REG_D,
	REG_C,
	REG_GP
} ERegisterType;

typedef enum {
	S_REG_ES,
	S_REG_CS,
	S_REG_SS,
	S_REG_DS,
	S_REG_FS,
	S_REG_GS
} ESRegister;

typedef enum {
	x87_REG_ST0,
	x87_REG_ST1,
	x87_REG_ST2,
	x87_REG_ST3,
	x87_REG_ST4,
	x87_REG_ST5,
	x87_REG_ST6,
	x87_REG_ST7
} Ex87Register;

typedef enum {
	MMX_REG_MM0,
	MMX_REG_MM1,
	MMX_REG_MM2,
	MMX_REG_MM3,
	MMX_REG_MM4,
	MMX_REG_MM5,
	MMX_REG_MM6,
	MMX_REG_MM7
} EMMXRegister;

typedef enum {
	XMM_REG_XMM0,
	XMM_REG_XMM1,
	XMM_REG_XMM2,
	XMM_REG_XMM3,
	XMM_REG_XMM4,
	XMM_REG_XMM5,
	XMM_REG_XMM6,
	XMM_REG_XMM7,
	XMM_REG_XMM8,
	XMM_REG_XMM9,
	XMM_REG_XMM10,
	XMM_REG_XMM11,
	XMM_REG_XMM12,
	XMM_REG_XMM13,
	XMM_REG_XMM14,
	XMM_REG_XMM15
} EXMMRegister;

typedef enum {
	YMM_REG_YMM0,
	YMM_REG_YMM1,
	YMM_REG_YMM2,
	YMM_REG_YMM3,
	YMM_REG_YMM4,
	YMM_REG_YMM5,
	YMM_REG_YMM6,
	YMM_REG_YMM7,
	YMM_REG_YMM8,
	YMM_REG_YMM9,
	YMM_REG_YMM10,
	YMM_REG_YMM11,
	YMM_REG_YMM12,
	YMM_REG_YMM13,
	YMM_REG_YMM14,
	YMM_REG_YMM15
} EYMMRegister;

typedef enum {
	D_REG_DR0,
	D_REG_DR1,
	D_REG_DR2,
	D_REG_DR3,
	D_REG_DR4,
	D_REG_DR5,
	D_REG_DR6,
	D_REG_DR7,
	D_REG_DR8,
	D_REG_DR9,
	D_REG_DR10,
	D_REG_DR11,
	D_REG_DR12,
	D_REG_DR13,
	D_REG_DR14,
	D_REG_DR15
} EDRegister;

typedef enum {
	C_REG_CR0,
	C_REG_CR1,
	C_REG_CR2,
	C_REG_CR3,
	C_REG_CR4,
	C_REG_CR5,
	C_REG_CR6,
	C_REG_CR7,
	C_REG_CR8,
	C_REG_CR9,
	C_REG_CR10,
	C_REG_CR11,
	C_REG_CR12,
	C_REG_CR13,
	C_REG_CR14,
	C_REG_CR15
} ECRegister;

typedef enum {
	GP_REG_A,
	GP_REG_C,
	GP_REG_D,
	GP_REG_B,
	GP_REG_AH_SP,
	GP_REG_CH_BP,
	GP_REG_DH_SI,
	GP_REG_BH_DI,
	GP_REG_R8,
	GP_REG_R9,
	GP_REG_R10,
	GP_REG_R11,
	GP_REG_R12,
	GP_REG_R13,
	GP_REG_R14,
	GP_REG_R15
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

struct Token;
struct Instruction;
struct InstrFuncs {
	int (*validate)(int cnt, struct Token **tokens);
	int16_t (*encode)(int *byte_count, struct Token **tokens);
};
struct Instruction {
	struct InstrFuncs funcs;
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

typedef enum {
	SIZE_NONE = 0,
	SIZE_BYTE = 1 << 0,
	SIZE_WORD = 1 << 1,
	SIZE_DWORD = 1 << 2,
	SIZE_QWORD = 1 << 3,
} EOptionalSize;


struct Register {
	ESize size;
	ERegisterType type;
	union {
		ESRegister segment_reg;
		Ex87Register x87_reg;
		EMMXRegister mmx_reg;
		EXMMRegister xmm_reg;
		EYMMRegister ymm_reg;
		EDRegister debug_reg;
		ECRegister control_reg;
		EGPRegister gp_reg;
	};
};

struct Label {
	char *value;
};

typedef struct {
	uint8_t is_base : 1;
	uint8_t is_si : 1;
	uint8_t is_displacement : 1;
	uint8_t is_label : 1;
	struct Register base;
	struct Register si_reg;
	struct Immediate si_imm;
	union {
		struct Immediate displacement;
		struct Label label;
	};
	EOptionalSize size;
} MemAccess;

// max 4 prefixes per instruction - more than 1 of a single group = undefined
typedef enum {
	// group 1
	PRE_LOCK = 0xF0,
	PRE_REPNE_REPNZ = 0xF2,
	PRE_REP_REPE_REPZ = 0xF3,
	// group 2
	// segment overrides
	PRE_CS = 0x2E,
	PRE_SS = 0x36,
	PRE_DS = 0x3E,
	PRE_ES = 0x26,
	PRE_FS = 0x64,
	PRE_GS = 0x65,
	// segment overrides
	PRE_BRANCH_TAKEN = 0x2E,
	PRE_BRANCH_NOT_TAKEN = 0x3E,
	// group 3
	PRE_OPERAND = 0x66,
	// group 4
	PRE_ADDRESS = 0x67
} EPrefixType;

struct Prefix {
	EPrefixType type;
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

#endif
