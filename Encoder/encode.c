#include "encodeHeader.h"

// Global Varible that resembles the address space
unsigned char codeBlock[4];
unsigned char * currentPosition;

unsigned int encodeOperands(instr_t instruction, const instr_info_t * instructionInformation)
{
  unsigned int operandEncoding = 0;
  unsigned int bitShifter = 0;
  unsigned int temp = 0;
  int i = 0;
  if (instructionInformation->dst1_type != TYPE_NONE)
  {
    temp = instructionInformation->dst1_mask;
    for(i = 0 ; i < 32; i++)
      if(temp & 0x1 == 1)
        break;
      else
      {
        temp = temp >> 1;
        bitShifter ++;
      }
    temp = instruction.dst1;
    temp = temp << bitShifter;
    operandEncoding = operandEncoding | temp;
    bitShifter = 0;
  }
  if (instructionInformation->dst2_type != TYPE_NONE)
  {
    temp = instructionInformation->dst2_mask;
    for(i = 0 ; i < 32; i++)
      if(temp & 0x1 == 1)
        break;
      else
      {
        temp = temp >> 1;
        bitShifter ++;
      }
    temp = instruction.dst2;
    temp = temp << bitShifter;
    operandEncoding = operandEncoding | temp;
    bitShifter = 0;
  }
  if (instructionInformation->src1_type != TYPE_NONE)
  {
    if(instructionInformation->src1_type != Reg)
    {
      temp = instructionInformation->src1_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      temp = instruction.immed;
      temp = temp << bitShifter;
      operandEncoding = operandEncoding | temp;
      bitShifter = 0;
    }
    else
    {
      temp = instructionInformation->src1_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
    
      temp = instruction.src1;
      temp = temp << bitShifter;
      operandEncoding = operandEncoding | temp;
      bitShifter = 0;
    }
  }
  if (instructionInformation->src2_type != TYPE_NONE)
  {
    if(instructionInformation->src2_type != Reg)
    {
      temp = instructionInformation->src2_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      temp = instruction.immed;
      temp = temp << bitShifter;
      operandEncoding = operandEncoding | temp;
      bitShifter = 0;
    }
    else
    {
      temp = instructionInformation->src2_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
    
      temp = instruction.src2;
      temp = temp << bitShifter;
      operandEncoding = operandEncoding | temp;
      bitShifter = 0;
    }
  }
  if (instructionInformation->src3_type != TYPE_NONE)
  {
    if(instructionInformation->src3_type != Reg)
    {
      temp = instructionInformation->src3_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      temp = instruction.immed;
      temp = temp << bitShifter;
      operandEncoding = operandEncoding | temp;
      bitShifter = 0;
    }
    else
    {
      temp = instructionInformation->src3_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
    
      temp = instruction.src3;
      temp = temp << bitShifter;
      operandEncoding = operandEncoding | temp;
      bitShifter = 0;
    }
  }
  
  return operandEncoding;
}

void encode(instr_t instruction, unsigned char * pc)
{
  const instr_info_t *instructionInformation;
  unsigned int instructionEncoding;
  unsigned int operandEncoding;
  // First job is to get the right array to be able to encode
  switch(instruction.instructionCategory)
  {
    case CAT_UNCONDITIONAL:
    {
      break;
    }
    case CAT_DATAPROC:
    {
      switch(instruction.processingCategory)
      {
        case DP_REGISTER:
        {
          instructionInformation = DataProcessingInstructionsR;
          break;
        }
        case DP_REGISTERSR:
        {
          break;
        }
        case DP_MISC:
        {
          break;
        }
        case DP_HWMMA:
        {
          break;
        }
        case DP_MULTIPLYMA:
        {
          instructionInformation = MultipyAndMultiplyAcc;
          break;
        }
        case DP_SP:
        {
          break;
        }
        case DP_EXTRALS:
        {
          break;
        }
        case DP_EXTRALSUP:
        {
          break;
        }
        case DP_IMM:
        {
          instructionInformation = DataProcessingInstructionsIMM;
          break;
        }
        case DP_IMMLD:
        {
          break;
        }
        case DP_IMMHILD:
        {
          break;
        }
        case DP_MSRIMM:
        {
          break;
        }
        default:
        {
          printf("This Instruction Can't Be Encoded");
          break;
        }
      }
      break;
    }
    case CAT_LOADSTORE1:
    {
    }
    case CAT_LOADSTORE2:
    {
      instructionInformation = loadStore;
      break;
    }
    case CAT_BRANCH:
    {
      instructionInformation = BranchInstructions;
      break;
    }
    case CAT_SVC:
    {
      instructionInformation = serviceCalls;
      break;
    }
    default:
    {
      printf("This Instruction Can't Be Encoded");
      break;
    }
  }
  
  // Now Cycle through the appropriate op codes until we find a match.
    // Now cycle through instructions to find exact one
  while (instructionInformation->type != OP_NULL)
  {
   //printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction,
     // decodedInstruction->concernedBits, decodedInstruction->maskedBits);
      
    if(instructionInformation->type == instruction.opcode)
    break;
    
    instructionInformation++;
  }
  
  printf("Your Instruction is %s\n", instructionInformation->name);
  
  instructionEncoding = 0;
  instructionEncoding = instructionInformation->concernedBits;
  
  printf("Opcode Encoding Complete. Instruction is: %.8X\n", instructionEncoding);
  
  // Now we need to deal with operands
  operandEncoding = encodeOperands(instruction,instructionInformation);
  
  printf("Operand Encoding Complete. Instruction is: %.8X\n", operandEncoding);
  
  instructionEncoding = instructionEncoding | operandEncoding;
  
  printf("Final Instruction is: %.8X\n", instructionEncoding);
  return;
}

int main()
{
  instr_t theInstruction;
  currentPosition = codeBlock;
  theInstruction.opcode = OP_ADD;
  theInstruction.instructionCategory = CAT_DATAPROC;
  theInstruction.processingCategory = DP_REGISTER;
  theInstruction.dst1 = 4;
  theInstruction.src1 = 2;
  theInstruction.src2 = 3;
  encode(theInstruction, currentPosition);
  return 0;
}
