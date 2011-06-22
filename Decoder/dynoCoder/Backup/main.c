#include "decodeHeader.h"
#include <stdio.h>
unsigned char theProgram [] = {
0x00, 0x92, 0x40, 0x03, // ADDS R4, R2, R3
0x00, 0xC2, 0x34, 0x91,  // SMULL R2, R3 , R4, R1 
0x01, 0xA0, 0x71, 0x83, // LSL R7, R3, #3
0x01, 0x12, 0x00, 0x01, // TST R2, R1
0x02, 0xF3, 0x40, 0x05, // RSC R4, R3, #5
0x0A, 0x00, 0x00, 0xFF, // B #255
0x0B, 0x00, 0x00, 0x0A, // BL #10
0x04, 0x19, 0x30, 0x0C, // LDR R3, R9, #12
0x06, 0x02, 0x10, 0x05};// STR R1, R2, R5
#define INSTR_NUMBER	9														 
int decodeCommon(unsigned int instruction)
{
  int i = 0;
  for(i = 0; i < CAT_NUMBER; i++)
  {
  //  printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction, Catergories[i].maskedBits, Catergories[i].concernedBits);
    if((instruction & Catergories[i].concernedBits) == Catergories[i].maskedBits)
      return Catergories[i].catergoryNumber;
  }
  return CAT_UNDEFINED;
}

int decodeDataProcessing(unsigned int instruction)
{
  int i = 0;
  for(i = 0; i < DP_NUMBER; i++)
  {
  //  printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction,
   //   DataProcCats[i].concernedBits, DataProcCats[i].maskedBits);
    if((instruction & DataProcCats[i].maskedBits) == DataProcCats[i].concernedBits)
      return DataProcCats[i].catergoryNumber;
  }
  return DP_UNDEFINED;
}

void read_operands(unsigned char * PC, decode_info_t *di, const instr_info_t  * info)
{
  // First of all get the full instruction again
  unsigned int instruction = *PC;
  int bitShifter = 0;
  int temp;
  int i = 0;
  instruction = instruction << 8 | *(PC+1);
  instruction = instruction << 8 | *(PC+2);
  instruction = instruction << 8 | *(PC+3);
  
  printf("Instruction %.8x\n", instruction);
  
  // Go through the availible operands one by one and write the values into DI.
  if (info->dst1_type != TYPE_NONE)
  {
 //   printf("Destination 1 Exists!!\n");
    di->regDst1 = (instruction) & (info->dst1_mask);
  //  printf("Destion1 Hex: %x\n", info->dst1_mask);
    //printf("Dest1 Hex %x\n", di->regDst1);
    temp = info->dst1_mask;
    for(i = 0 ; i < 32; i++)
      if(temp & 0x1 == 1)
        break;
      else
      {
        temp = temp >> 1;
        bitShifter ++;
      }
    
    di->regDst1 = di->regDst1 >> bitShifter;
    bitShifter = 0;
  }
  if (info->dst2_type != TYPE_NONE)
  {
  //  printf("Destination 2 Exists!!\n");
    di->regDst2 = instruction & info->dst2_mask;
 //  printf("Dest2 Hex %x\n", di->regDst2);
    temp = info->dst2_mask;
    for(i = 0 ; i < 32; i++)
      if(temp & 0x1 == 1)
        break;
      else
      {
        temp = temp >> 1;
        bitShifter ++;
      }
    
    di->regDst2 = di->regDst2 >> bitShifter;
    bitShifter = 0;
  }
  if (info->src1_type != TYPE_NONE)
  {
    printf("Source 1 Exists!!\n");
    if(info->src1_type != Reg)
    {
      di->immed = instruction & info->src1_mask;
      temp = info->src1_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
    
      di->immed = di->immed >> bitShifter;
      bitShifter = 0;
    }
    else
    {
      di->regSrc1 = instruction & info->src1_mask;
     // printf("Src1 Hex %x\n", di->regSrc1);
      temp = info->src1_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
    
      di->regSrc1 = di->regSrc1 >> bitShifter;
      bitShifter = 0;
    }
  }
  if (info->src2_type != TYPE_NONE)
  {
 //   printf("Source 2 Exists!!\n");
    if(info->src2_type != Reg)
    {
      di->immed = instruction & info->src2_mask;
      temp = info->src2_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
    
      di->immed = di->immed >> bitShifter;
      bitShifter = 0;
    }
    else
    {
      di->regSrc2 = instruction & info->src2_mask;
  //           printf("Src2 Hex %x\n", di->regSrc2);
      temp = info->src2_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      di->regSrc2 = di->regSrc2 >> bitShifter;
    }
  }
  if (info->src3_type != TYPE_NONE)
  {
 //   printf("Source 3 Exists!!\n");
    if(info->src3_type != Reg)
    {
      di->immed = instruction & info->src3_mask;
      temp = info->src3_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      di->immed = di->immed >> bitShifter;
      bitShifter = 0;
    }
    else
    {
      di->regSrc3 = instruction & info->src3_mask;
      temp = info->src3_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      di->regSrc3 = di->regSrc3 >> bitShifter;
    }
  }
  return;
}

unsigned char * readInstruction(unsigned char * PC)
{
  unsigned int instruction = *PC;
  const instr_info_t *decodedInstruction = NULL;
  decode_info_t di;
  di.start_pc = PC;
  di.size_immed = OPSZ_NA;
  di.regDst1 = TYPE_NONE;
  di.regDst2 = TYPE_NONE;
  di.regSrc1 = TYPE_NONE;
  di.regSrc2 = TYPE_NONE;
  di.regSrc3 = TYPE_NONE;
  di.immed = 0;
  instruction = instruction << 8 | *(PC+1);
  instruction = instruction << 8 | *(PC+2);
  instruction = instruction << 8 | *(PC+3);
  
  // First we need to check what type of instruction it is
  int instrCat = decodeCommon(instruction);
  
  // Do the Appropriate
  switch (instrCat)
  {
    case CAT_UNCONDITIONAL:
    {
      printf("Instruction is an Unconditional Instruction\n");
      break;
    }
    case CAT_DATAPROC:
    {
      printf("Instruction is a Data Processing Instruction\n");
      instrCat = decodeDataProcessing(instruction);
      switch (instrCat)
      {
        case DP_REGISTER:
        {
          printf("Instruction is a Register Processing Instruction\n");
          decodedInstruction = DataProcessingInstructionsR;
          break;
        }
        case DP_REGISTERSR:
        {
          printf("Instruction is a Register Shifted Register Instruction\n");
          break;
        }
        case DP_MISC:
        {
          printf("Instruction is a Misc Instruction\n");
          break;
        }
        case DP_HWMMA:
        {
          printf("Instruction is a Halfword Multiply Accumumlate Instructions\n");
          break;
        }
        case DP_MULTIPLYMA:
        {
          printf("Instruction is a Multiply and Accumulate Instruction\n");
          decodedInstruction = MultipyAndMultiplyAcc;
          break;
        }
        case DP_SP:
        {
          printf("Instruction is a Synchronization Primitive\n");
          break;
        }
        case DP_EXTRALS:
        {
          printf("Instruction is an Extra Load/ Store Instruction\n");
          break;
        }
        case DP_EXTRALSUP:
        {
          printf("Instruction is an Extra Load Store Unprivlaged Instruction\n");
          break;
        }
        case DP_IMM:
        {
          printf("Instruction is a Data Processing IMM Instruction\n");
          decodedInstruction = DataProcessingInstructionsIMM;
          break;
        }
        case DP_IMMLD:
        {
          printf("Instruction is a 16 Bit Immediate load Instruction\n");
          break;
        }
        case DP_IMMHILD:
        {
          printf("Instruction is a High Halfword Immediate Load Instruction\n");
          break;
        }
        case DP_MSRIMM:
        {
          printf("Instruction is a MSR Immedate Instruction\n");
          break;
        }
        case DP_UNDEFINED:
        {
          printf("Instruction is Undefined\n");
          break;
        }
      }
      break;
    }
    case CAT_LOADSTORE1:
    {
      printf("Instruction is a Load Store Type 1 Instruction\n");
    }
    case CAT_LOADSTORE2:
    {
      printf("Instruction is a Load Store Type 2 Instruction\n");
      decodedInstruction = loadStore;
      break;
    }
    case CAT_BRANCH:
    {
      printf("Instruction is a Branch Insturction\n");
      decodedInstruction = BranchInstructions;
      break;
    }
    case CAT_SVC:
    {
      printf("Instruction is a Service Call Instruction\n");
      break;
    }
    case CAT_UNDEFINED:
    {
      printf("Instruction is Undefined\n");
      break;
    }
  }
  
  // Now cycle through instructions to find exact one
  while (decodedInstruction->type != OP_NULL)
  {
   //printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction,
     // decodedInstruction->concernedBits, decodedInstruction->maskedBits);
      
    if((instruction & decodedInstruction->maskedBits) == decodedInstruction->concernedBits)
    break;
    
    decodedInstruction++;
  }
  printf("Final Instruction Opcode is: %s\n", decodedInstruction->name);
  
  // Now we need to work out the Operands
   //PC = read_operands(PC, di, info, instr_cat);
   read_operands(PC,&di, decodedInstruction);
   printf("Destination Operand 1 is: %d\n", di.regDst1);
   printf("Destination Operand 2 is: %d\n", di.regDst2);
   printf("Source Operand 1 is: %d\n", di.regSrc1);
   printf("Source Operand 2 is: %d\n", di.regSrc2);
   printf("Source Operand 3 is: %d\n", di.regSrc3);
   printf("Immediate Operand is: %d\n", di.immed);   
   return PC + 4;
}

int main()
{
  unsigned int PC;
  int i;
  unsigned char * currentPC = theProgram;
  for(i = 0; i < INSTR_NUMBER; i++)
  {
    PC = *currentPC;
    PC = PC << 8 | *(currentPC+1);
    PC = PC << 8 | *(currentPC+2);
    PC = PC << 8 | *(currentPC+3);
    printf("Instruction %d is: %.8x \n",i, PC);
    currentPC = readInstruction(currentPC);
  }
  return 0;
}
