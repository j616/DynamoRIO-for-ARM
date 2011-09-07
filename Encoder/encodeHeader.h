#include <stdio.h>

#define CAT_UNCONDITIONAL 	1
#define CAT_DATAPROC				2
#define CAT_LOADSTORE1			3
#define CAT_LOADSTORE2			4
#define	CAT_BRANCH					5
#define CAT_SVC							6
#define CAT_UNDEFINED				7
#define CAT_NUMBER					6

#define DP_REGISTER					1
#define DP_REGISTERSR				2
#define	DP_MISC							3
#define DP_HWMMA						4
#define DP_MULTIPLYMA				5
#define	DP_SP								6
#define DP_EXTRALS					7
#define DP_EXTRALSUP				8
#define DP_IMM							9
#define DP_IMMLD					 	10
#define DP_IMMHILD					11
#define	DP_MSRIMM						12
#define DP_UNDEFINED				13
#define DP_NUMBER						14

#define UNKNOWN_APSR	0
enum {
	TYPE_NONE,
	Reg,
	Immediate,
};

enum{
/* 0 */ OP_AND,
/* 1 */ OP_EOR,
/* 2 */ OP_SUB,
/* 3 */ OP_RSB,
/* 4 */ OP_ADD,
/* 5 */ OP_ADC,
/* 6 */ OP_SBC,
/* 7 */ OP_RSC,
/* 8 */ OP_TST,
/* 9 */ OP_TEQ,
/* 10 */ OP_CMP,
/* 11 */ OP_CMN,
/* 12 */ OP_ORR,
/* 13 */ OP_MOV,
/* 14 */ OP_LSL,
/* 15 */ OP_LSR,
/* 16 */ OP_ASR,
/* 17 */ OP_RRX,
/* 18 */ OP_ROR,
/* 19 */ OP_BIC,
/* 20 */ OP_MVN,
/* 21 */ OP_ADR,
/* 22 */ OP_MUL,
/* 23 */ OP_MLA,
/* 24 */ OP_UMAAL,
/* 25 */ OP_MLS,
/* 26 */ OP_UMULL,
/* 27 */ OP_UMLAL,
/* 28 */ OP_SMULL,
/* 29 */ OP_SMLAL,
/* 30 */ OP_B,
/* 31 */ OP_BL,
/* 32 */ OP_STRT,
/* 33 */ OP_STRB,
/* 34 */ OP_STRBT,
/* 35 */ OP_LDRT,
/* 36 */ OP_LDRB,
/* 37 */ OP_LDRBT,
/* 38 */ OP_LDR,
/* 39 */ OP_STR,
/* 40 */ OP_SVC,
				 OP_NULL,
};

// Sturcture from the decode header
typedef struct  {
  int type;
  unsigned int concernedBits;
  unsigned int maskedBits;
  char *name;
  unsigned char dst1_type; unsigned int dst1_mask;
  unsigned char dst2_type; unsigned int dst2_mask;
  unsigned char src1_type; unsigned int src1_mask;
  unsigned char src2_type; unsigned int src2_mask;
  unsigned char src3_type; unsigned int src3_mask;
  unsigned int APSR;
} instr_info_t;

typedef struct
{
  unsigned int opcode;
  unsigned int instructionCategory;
  
  // This is an optional field that is only used if instructions is a
  // data processing instruction
  unsigned int processingCategory;
  
  // The Values of the Destinations
  unsigned int dst1;
  unsigned int dst2;
  unsigned int src1;
  unsigned int src2;
  unsigned int src3;
  unsigned int immed;
} instr_t;

extern const instr_info_t DataProcessingInstructionsR[];
extern const instr_info_t MultipyAndMultiplyAcc[];
extern const instr_info_t DataProcessingInstructionsIMM[];
extern const instr_info_t BranchInstructions[];
extern const instr_info_t loadStore[];
extern const instr_info_t serviceCalls[];
