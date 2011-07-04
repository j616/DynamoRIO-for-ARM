#include "../globals.h"
#include "arch.h"
#include "instr.h"
#include "decode.h"
#include "decode_fast.h"
#include <string.h> /* for memcpy */

byte *
decode(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	return 0;
}

/* Disassembles the instruction at pc into the data structures ret_info
 * and di.  Does NOT set or read di->len.
 * Returns a pointer to the pc of the next instruction.
 * If just_opcode is true, does not decode the immeds and returns NULL
 * (you must call decode_next_pc to get the next pc, but that's faster
 * than decoding the immeds)
 * Returns NULL on an invalid instruction
 */
/* Disassembles the instruction at pc into the data structures ret_info
 * and di.  Does NOT set or read di->len.
 * Returns a pointer to the pc of the next instruction.
 * If just_opcode is true, does not decode the immeds and returns NULL
 * (you must call decode_next_pc to get the next pc, but that's faster
 * than decoding the immeds)
 * Returns NULL on an invalid instruction
 */
#ifdef ARM

// This function just works out what type of Data Processing Instruction we have
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

// This function just works out which of the top layer the instruction is
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

// Read the operands of the instruction
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

static byte *
read_instruction(byte *pc, byte *orig_pc,
                 const instr_info_t **ret_info, decode_info_t *di,
                 bool just_opcode _IF_DEBUG(bool report_invalid))
{
	// INPROCESSS read_instruction Has been gotten to but not ready to do yet
	unsigned int instruction = *pc;
	const instr_info_t *decodedInstruction = NULL;
	di->start_pc = pc;
	di->size_immed = OPSZ_NA;
  di->regDst1 = TYPE_NONE;
  di->regDst2 = TYPE_NONE;
  di->regSrc1 = TYPE_NONE;
  di->regSrc2 = TYPE_NONE;
  di->regSrc3 = TYPE_NONE;
  di->immed = 0;

  // First of all we need to deal with the instruction as a whole not just a byte
  instruction = instruction << 8 | *(pc+1);
  instruction = instruction << 8 | *(pc+2);
  instruction = instruction << 8 | *(pc+3);

  // First we need to check what type of instruction it is
  int instrCat = decodeCommon(instruction);

  // With this information get the appropriate array to decode the instruction
  switch (instrCat)
    {
      case CAT_UNCONDITIONAL:
      {
        printf("Instruction is an Unconditional Instruction\n");
        decodedInstruction = NullInstruction;
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
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_MISC:
          {
            printf("Instruction is a Misc Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_HWMMA:
          {
            printf("Instruction is a Halfword Multiply Accumumlate Instructions\n");
            decodedInstruction = NullInstruction;
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
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_EXTRALS:
          {
            printf("Instruction is an Extra Load/ Store Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_EXTRALSUP:
          {
            printf("Instruction is an Extra Load Store Unprivlaged Instruction\n");
            decodedInstruction = NullInstruction;
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
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_IMMHILD:
          {
            printf("Instruction is a High Halfword Immediate Load Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_MSRIMM:
          {
            printf("Instruction is a MSR Immedate Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_UNDEFINED:
          {
            printf("Instruction is Undefined\n");
            decodedInstruction = NullInstruction;
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
        decodedInstruction = NullInstruction;
        break;
      }
      case CAT_UNDEFINED:
      {
        printf("Instruction is Undefined\n");
        decodedInstruction = NullInstruction;
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

  /* if just want opcode, stop here!  faster for caller to
   * separately call decode_next_pc than for us to decode immeds!
   */
  if (just_opcode) {
      *ret_info = decodedInstruction;
      return NULL;
  }

  read_operands(pc,di, decodedInstruction);
  printf("Destination Operand 1 is: %d\n", di->regDst1);
  printf("Destination Operand 2 is: %d\n", di->regDst2);
  printf("Source Operand 1 is: %d\n", di->regSrc1);
  printf("Source Operand 2 is: %d\n", di->regSrc2);
  printf("Source Operand 3 is: %d\n", di->regSrc3);
  printf("Immediate Operand is: %d\n", di->immed);
  /* return values */
  *ret_info = decodedInstruction;
  return pc+4;
}
#else
/* Disassembles the instruction at pc into the data structures ret_info
 * and di.  Does NOT set or read di->len.
 * Returns a pointer to the pc of the next instruction.
 * If just_opcode is true, does not decode the immeds and returns NULL
 * (you must call decode_next_pc to get the next pc, but that's faster
 * than decoding the immeds)
 * Returns NULL on an invalid instruction
 */

opnd_size_t
resolve_var_reg_size(opnd_size_t sz, bool is_reg)
{
    switch (sz) {
    case OPSZ_1_reg4: return (is_reg ? OPSZ_4 : OPSZ_1);
    case OPSZ_2_reg4: return (is_reg ? OPSZ_4 : OPSZ_2);
    case OPSZ_4_reg16: return (is_reg ? OPSZ_16 : OPSZ_4);
    }
    return sz;
}

/* Like all our code, we assume cs specifies default data and address sizes.
 * This routine assumes the size varies by data, NOT by address!
 */
opnd_size_t
resolve_variable_size(decode_info_t *di/*IN: x86_mode, prefixes*/,
                      opnd_size_t sz, bool is_reg)
{
    switch (sz) {
    case OPSZ_2_short1: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_1 : OPSZ_2);
    case OPSZ_4_short2: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4);
    case OPSZ_4x8: return (X64_MODE(di) ? OPSZ_8 : OPSZ_4);
    case OPSZ_4x8_short2:
        return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 :
                (X64_MODE(di) ? OPSZ_8 : OPSZ_4));
    case OPSZ_4x8_short2xi8:
        return (X64_MODE(di) ? (proc_get_vendor() == VENDOR_INTEL ? OPSZ_8 :
                              (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_8)) :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_short2xi4:
        return ((X64_MODE(di) && proc_get_vendor() == VENDOR_INTEL) ? OPSZ_4 :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_rex8_short2: /* rex.w trumps data prefix */
        return (TEST(PREFIX_REX_W, di->prefixes) ? OPSZ_8 :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_rex8: return (TEST(PREFIX_REX_W, di->prefixes) ? OPSZ_8 : OPSZ_4);
    case OPSZ_6_irex10_short4: /* rex.w trumps data prefix, but is ignored on AMD */
        DODEBUG({
            /* less annoying than a CURIOSITY assert when testing */
            if (TEST(PREFIX_REX_W, di->prefixes))
                SYSLOG_INTERNAL_INFO_ONCE("curiosity: rex.w on OPSZ_6_irex10_short4!");
        });
        return ((TEST(PREFIX_REX_W, di->prefixes) && proc_get_vendor() != VENDOR_AMD) ?
                OPSZ_10 : (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_4 : OPSZ_6));
    case OPSZ_6x10: return (X64_MODE(di) ? OPSZ_10 : OPSZ_6);
    case OPSZ_8_short2: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_8);
    case OPSZ_8_short4: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_4 : OPSZ_8);
    case OPSZ_28_short14:
        return (TEST(PREFIX_DATA, di->prefixes) ?  OPSZ_14 : OPSZ_28);
    case OPSZ_108_short94:
        return (TEST(PREFIX_DATA, di->prefixes) ?  OPSZ_94 : OPSZ_108);
    case OPSZ_1_reg4:
    case OPSZ_2_reg4:
    case OPSZ_4_reg16:
        return resolve_var_reg_size(sz, is_reg);
    }
    return sz;
}

opnd_size_t
resolve_variable_size_dc(dcontext_t *dcontext, uint prefixes, opnd_size_t sz, bool is_reg)
{
    decode_info_t di;
    IF_X64(di.x86_mode = get_x86_mode(dcontext));
    di.prefixes = prefixes;
    return resolve_variable_size(&di, sz, is_reg);
}

opnd_size_t
resolve_addr_size(decode_info_t *di/*IN: x86_mode, prefixes*/)
{
    if (TEST(PREFIX_ADDR, di->prefixes))
        return (X64_MODE(di) ? OPSZ_4 : OPSZ_2);
    else
        return (X64_MODE(di) ? OPSZ_8 : OPSZ_4);
}

static byte *
read_immed(byte *pc, decode_info_t *di, opnd_size_t size, ptr_int_t *result)
{
    size = resolve_variable_size(di, size, false);

    /* all data immediates are sign-extended.  we use the compiler's casts with
     * signed types to do our sign extensions for us.
     */
    switch (size) {
    case OPSZ_1:
        *result = (ptr_int_t) (char) *pc; /* sign-extend */
        pc++;
        break;
    case OPSZ_2:
        *result = (ptr_int_t) *((short*)pc); /* sign-extend */
        pc += 2;
        break;
    case OPSZ_4:
        *result = (ptr_int_t) *((int*)pc); /* sign-extend */
        pc += 4;
        break;
    case OPSZ_8:
        CLIENT_ASSERT(X64_MODE(di), "decode immediate: invalid size");
        CLIENT_ASSERT(sizeof(ptr_int_t) == 8, "decode immediate: internal size error");
        *result = *((ptr_int_t*)pc);
        pc += 8;
        break;
    default:
        /* called internally w/ instr_info_t fields or hardcoded values,
         * so ok to assert */
        CLIENT_ASSERT(false, "decode immediate: unknown size");
    }
    return pc;
}
/* reads any trailing immed bytes */
static byte *
read_operand(byte *pc, decode_info_t *di, byte optype, opnd_size_t opsize)
{
    ptr_int_t val = 0;
    opnd_size_t size = opsize;
    switch (optype) {
    case TYPE_A:
        {
            CLIENT_ASSERT(!X64_MODE(di), "x64 has no type A instructions");
#ifdef IA32_ON_IA64
            /* somewhat hacked dispatch on size */
            if (opsize == OPSZ_4_short2) {
                pc = read_immed(pc, di, opsize, &val);
                break;
            }
#endif
            /* ok b/c only instr_info_t fields passed */
            CLIENT_ASSERT(opsize == OPSZ_6_irex10_short4, "decode A operand error");
            if (TEST(PREFIX_DATA, di->prefixes)) {
                /* 4-byte immed */
                pc = read_immed(pc, di, OPSZ_4, &val);
#ifdef X64
                if (!X64_MODE(di)) {
                    /* we do not want the sign extension that read_immed() applied */
                    val &= (ptr_int_t) 0x00000000ffffffff;
                }
#endif
                /* ok b/c only instr_info_t fields passed */
                CLIENT_ASSERT(di->size_immed == OPSZ_NA &&
                              di->size_immed2 == OPSZ_NA, "decode A operand error");
                di->size_immed = resolve_variable_size(di, opsize, false);
                ASSERT(di->size_immed == OPSZ_4);
                di->immed = val;
            } else {
                /* 6-byte immed */
                ptr_int_t val2 = 0;
                /* little-endian: segment comes last */
                pc = read_immed(pc, di, OPSZ_4, &val2);
                pc = read_immed(pc, di, OPSZ_2, &val);
#ifdef X64
                if (!X64_MODE(di)) {
                    /* we do not want the sign extension that read_immed() applied */
                    val2 &= (ptr_int_t) 0x00000000ffffffff;
                }
#endif
                /* ok b/c only instr_info_t fields passed */
                CLIENT_ASSERT(di->size_immed == OPSZ_NA &&
                              di->size_immed2 == OPSZ_NA, "decode A operand error");
                di->size_immed = resolve_variable_size(di, opsize, false);
                ASSERT(di->size_immed == OPSZ_6);
                di->size_immed2 = resolve_variable_size(di, opsize, false);
                di->immed = val;
                di->immed2 = val2;
            }
            return pc;
        }
    case TYPE_I:
        {
            pc = read_immed(pc, di, opsize, &val);
            break;
        }
    case TYPE_J:
        {
            byte *end_pc;
            pc = read_immed(pc, di, opsize, &val);
            if (di->orig_pc != di->start_pc) {
                CLIENT_ASSERT(di->start_pc != NULL,
                              "internal decode error: start pc not set");
                end_pc = di->orig_pc + (pc - di->start_pc);
            } else
                end_pc = pc;
            /* convert from relative offset to absolute target pc */
            val = ((ptr_int_t)end_pc) + val;
            if ((!X64_MODE(di) || proc_get_vendor() != VENDOR_INTEL) &&
                TEST(PREFIX_DATA, di->prefixes)) {
                /* need to clear upper 16 bits */
                val &= (ptr_int_t) 0x0000ffff;
            } /* for x64 Intel, always 64-bit addr ("f64" in Intel table) */
            break;
        }
    case TYPE_O:
        {
            /* no modrm byte, offset follows directly.  this is address-sized,
             * so 64-bit for x64, and addr prefix affects it. */
            size = resolve_addr_size(di);
            pc = read_immed(pc, di, size, &val);
            if (TEST(PREFIX_ADDR, di->prefixes)) {
                /* need to clear upper bits */
                if (X64_MODE(di))
                    val &= (ptr_int_t) 0xffffffff;
                else
                    val &= (ptr_int_t) 0x0000ffff;
            }
#ifdef X64
            if (!X64_MODE(di)) {
                /* we do not want the sign extension that read_immed() applied */
                val &= (ptr_int_t) 0x00000000ffffffff;
            }
#endif
            break;
        }
    default:
        return pc;
    }
    if (di->size_immed == OPSZ_NA) {
        di->size_immed = size;
        di->immed = val;
    } else {
        /* ok b/c only instr_info_t fields passed */
        CLIENT_ASSERT(di->size_immed2 == OPSZ_NA, "decode operand error");
        di->size_immed2 = size;
        di->immed2 = val;
    }
    return pc;
}

/* reads the modrm byte and any following sib and offset bytes */
static byte *
read_modrm(byte *pc, decode_info_t *di)
{
    byte modrm = *pc;
    pc++;
    di->modrm = modrm;
    di->mod = (byte)((modrm >> 6) & 0x3); /* top 2 bits */
    di->reg = (byte)((modrm >> 3) & 0x7); /* middle 3 bits */
    di->rm  = (byte)(modrm & 0x7);        /* bottom 3 bits */

    /* addr16 displacement */
    if (!X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes)) {
        di->has_sib = false;
        if ((di->mod == 0 && di->rm == 6) || di->mod == 2) {
            /* 2-byte disp */
            di->has_disp = true;
            if (di->mod == 0 && di->rm == 6) {
                /* treat absolute addr as unsigned */
                di->disp = (int) *((ushort *)pc); /* zero-extend */
            } else {
                /* treat relative addr as signed */
                di->disp = (int) *((short *)pc); /* sign-extend */
            }
            pc += 2;
        } else if (di->mod == 1) {
            /* 1-byte disp */
            di->has_disp = true;
            di->disp = (int) (char) *pc; /* sign-extend */
            pc++;
        } else {
            di->has_disp = false;
        }
    } else {
        /* 32-bit, which sometimes has a SIB */
        if (di->rm == 4 && di->mod != 3) {
            /* need SIB */
            byte sib = *pc;
            pc++;
            di->has_sib = true;
            di->scale = (byte)((sib >> 6) & 0x3); /* top 2 bits */
            di->index = (byte)((sib >> 3) & 0x7); /* middle 3 bits */
            di->base  = (byte)(sib & 0x7);        /* bottom 3 bits */
        } else {
            di->has_sib = false;
        }

        /* displacement */
        if ((di->mod == 0 && di->rm == 5) ||
            (di->has_sib && di->mod == 0 && di->base == 5) ||
            di->mod == 2) {
            /* 4-byte disp */
            di->has_disp = true;
            di->disp = *((int *)pc);
            IF_X64(di->disp_abs = pc); /* used to set instr->rip_rel_pos */
            pc += 4;
        } else if (di->mod == 1) {
            /* 1-byte disp */
            di->has_disp = true;
            di->disp = (int) (char) *pc; /* sign-extend */
            pc++;
        } else {
            di->has_disp = false;
        }
    }
    return pc;
}
static byte *
read_instruction(byte *pc, byte *orig_pc,
                 const instr_info_t **ret_info, decode_info_t *di,
                 bool just_opcode _IF_DEBUG(bool report_invalid))
{
    DEBUG_DECLARE(byte *post_suffix_pc = NULL;)
    byte first_instr_byte;
    const instr_info_t *info;
    /* these 3 prefixes may be part of opcode: */
    bool data_prefix = false;
    bool rep_prefix = false;
    bool repne_prefix = false;

    /* initialize di */
    /* though we only need di->start_pc for full decode rip-rel (and
     * there only post-read_instruction()) and decode_from_copy(), and
     * di->orig_pc only for decode_from_copy(), we assume that
     * high-perf decoding uses decode_cti() and live w/ the extra
     * writes here for decode_opcode() and decode_eflags_usage().
     */
    di->start_pc = pc;
    di->orig_pc = orig_pc;
    di->size_immed = OPSZ_NA;
    di->size_immed2 = OPSZ_NA;
    di->seg_override = REG_NULL;
    /* FIXME: set data and addr sizes to current mode
     * for now I assume always 32-bit mode (or 64 for X64_MODE(di))!
     */
    di->prefixes = 0;

    do {
        first_instr_byte = *pc;
        pc++;
        info = &first_byte[first_instr_byte];
        if (info->type == X64_EXT) {
            /* discard old info, get new one */
            info = &x64_extensions[info->code][X64_MODE(di) ? 1 : 0];
        }
        if (info->type == PREFIX) {
            if (TESTANY(PREFIX_REX_ALL, di->prefixes)) {
                /* rex.* must come after all other prefixes (including those that are
                 * part of the opcode, xref PR 271878): so discard them if before
                 * matching the behavior of decode_sizeof().  This in effect nops
                 * improperly placed rex prefixes which (xref PR 241563 and Intel Manual
                 * 2A 2.2.1) is the correct thing to do. NOTE - windbg shows early bytes
                 * as ??, objdump as their prefix names, separate from the next instr.
                 */
                di->prefixes &= ~PREFIX_REX_ALL;
            }
            if (info->code == PREFIX_REP) {
                /* see if used as part of opcode before considering prefix */
                rep_prefix = true;
            } else if (info->code == PREFIX_REPNE) {
                /* see if used as part of opcode before considering prefix */
                repne_prefix = true;
            } else if (REG_START_SEGMENT <= info->code &&
                       info->code <= REG_STOP_SEGMENT) {
                CLIENT_ASSERT_TRUNCATE(di->seg_override, byte, info->code,
                                       "decode error: invalid segment override");
                di->seg_override = (byte) info->code;
            } else if (info->code == PREFIX_DATA) {
                /* see if used as part of opcode before considering prefix */
                data_prefix = true;
            } else if (TESTANY(PREFIX_REX_ALL | PREFIX_ADDR | PREFIX_LOCK,
                               info->code)) {
                di->prefixes |= info->code;
            }
        } else
            break;
    } while (true);

    if (info->type == ESCAPE) {
        /* discard first byte, move to second */
        first_instr_byte = *pc; /* really 2nd, just reusing var */
        pc++;
        info = &second_byte[first_instr_byte];
    }
    if (info->type == ESCAPE_3BYTE_38 ||
        info->type == ESCAPE_3BYTE_3a) {
        /* discard second byte, move to third */
        first_instr_byte = *pc; /* really 3rd, just reusing var */
        pc++;
        if (info->type == ESCAPE_3BYTE_38)
            info = &third_byte_38[third_byte_38_index[first_instr_byte]];
        else
            info = &third_byte_3a[third_byte_3a_index[first_instr_byte]];
    }

    /* all FLOAT_EXT and PREFIX_EXT (except nop & pause) and EXTENSION need modrm,
     * get it now
     */
    if ((info->flags & HAS_MODRM) != 0)
        pc = read_modrm(pc, di);

    if (info->type == FLOAT_EXT) {
        if (di->modrm <= 0xbf) {
            int offs = (first_instr_byte - 0xd8) * 8 + di->reg;
            info = &float_low_modrm[offs];
        } else {
            int offs1 = (first_instr_byte - 0xd8);
            int offs2 = di->modrm - 0xc0;
            info = &float_high_modrm[offs1][offs2];
        }
    }
    else if (info->type == PREFIX_EXT) {
        /* discard old info, get new one */
        int code = (int) info->code;
        int idx = (rep_prefix?1 :(data_prefix?2 :(repne_prefix?3 :0)));
        info = &prefix_extensions[code][idx];
        if (rep_prefix)
            rep_prefix = false;
        else if (data_prefix)
            data_prefix = false;
        else if (repne_prefix)
            repne_prefix = false;
        if (info->type == REX_EXT) {
            /* discard old info, get new one */
            int code = (int) info->code;
            /* currently indexed by rex.b only */
            int idx = (TEST(PREFIX_REX_B, di->prefixes) ? 1 : 0);
            info = &rex_extensions[code][idx];
        }
    }
    else if (info->type == REP_EXT) {
        /* discard old info, get new one */
        int code = (int) info->code;
        int idx = (rep_prefix ? 2 : 0);
        info = &rep_extensions[code][idx];
        if (rep_prefix)
            rep_prefix = false;
    }
    else if (info->type == REPNE_EXT) {
        /* discard old info, get new one */
        int code = (int) info->code;
        int idx = (rep_prefix? 2 : (repne_prefix? 4 :0));
        info = &repne_extensions[code][idx];
        rep_prefix = false;
        repne_prefix = false;
    }
    else if (info->type == EXTENSION) {
        /* discard old info, get new one */
        info = &extensions[info->code][di->reg];
        /* absurd cases of using prefix on top of reg opcode extension
         * (pslldq, psrldq)
         */
        if (info->type == PREFIX_EXT) {
            /* discard old info, get new one */
            int code = (int) info->code;
            int idx = (rep_prefix?1 :(data_prefix?2 :(repne_prefix?3 :0)));
            info = &prefix_extensions[code][idx];
            if (rep_prefix)
                rep_prefix = false;
            else if (data_prefix)
                data_prefix = false;
            else if (repne_prefix)
                repne_prefix = false;
        } else if (info->type == MOD_EXT) {
            info = &mod_extensions[info->code][(di->mod==3) ? 1 : 0];
            /* Yes, we have yet another layer, thanks to Intel's poor choice
             * in opcodes -- why didn't they fill out the PREFIX_EXT space?
             */
            if (info->type == RM_EXT) {
                info = &rm_extensions[info->code][di->rm];
            }
        }
    }
    else if (info->type == SUFFIX_EXT) {
        /* Discard old info, get new one for complete opcode, which includes
         * a suffix byte where an immed would be (yes, ugly!).
         * We should have already read in the modrm (+ sib).
         */
        CLIENT_ASSERT(TEST(HAS_MODRM, info->flags), "decode error on 3DNow instr");
        info = &suffix_extensions[suffix_index[*pc]];
        pc++;
        DEBUG_DECLARE(post_suffix_pc = pc;)
    }

    if (TEST(REQUIRES_PREFIX, info->flags)) {
        byte required = (byte)(info->opcode >> 24);
        bool *prefix_var = NULL;
        CLIENT_ASSERT(info->opcode > 0xffffff, "decode error in SSSE3/SSE4 instr");
        if (required == DATA_PREFIX_OPCODE)
            prefix_var = &data_prefix;
        else if (required == REPNE_PREFIX_OPCODE)
            prefix_var = &repne_prefix;
        else if (required == REP_PREFIX_OPCODE)
            prefix_var = &rep_prefix;
        else
            CLIENT_ASSERT(false, "internal required-prefix error");
        if (prefix_var == NULL || !*prefix_var) {
            /* Invalid instr.  TODO: have processor w/ SSE4, confirm that
             * an exception really is raised.
             */
            info = NULL;
        } else
            *prefix_var = false;
    }

    /* at this point should be an instruction, so type should be an OP_ constant */
    if (info == NULL || info->type < OP_FIRST || info->type > OP_LAST ||
        (X64_MODE(di) && TEST(X64_INVALID, info->flags)) ||
        (!X64_MODE(di) && TEST(X86_INVALID, info->flags))) {
        /* invalid instruction: up to caller to decide what to do with it */
        /* FIXME case 10672: provide a runtime option to specify new
         * instruction formats */
        DODEBUG({
            /* don't report when decoding DR addresses, as we sometimes try to
             * decode backward (e.g., interrupted_inlined_syscall(): PR 605161)
             * XXX: better to pass in a flag when decoding that we are
             * being speculative!
             */
            if (report_invalid && !is_dynamo_address(di->start_pc)) {
                SYSLOG_INTERNAL_WARNING_ONCE("Invalid opcode encountered");
                if (info != NULL && info->type == INVALID) {
                    LOG(THREAD_GET, LOG_ALL, 1, "Invalid opcode @"PFX": 0x%x\n",
                        di->start_pc, info->opcode);
                } else {
                    int i;
                    dcontext_t *dcontext = get_thread_private_dcontext();
                    IF_X64(bool old_mode = set_x86_mode(dcontext, di->x86_mode);)
                    int sz = decode_sizeof(dcontext, di->start_pc, NULL _IF_X64(NULL));
                    IF_X64(set_x86_mode(dcontext, old_mode));
                    LOG(THREAD_GET, LOG_ALL, 1, "Error decoding "PFX" == ", di->start_pc);
                    for (i=0; i<sz; i++) {
                        LOG(THREAD_GET, LOG_ALL, 1, "0x%x ", *(di->start_pc+i));
                    }
                    LOG(THREAD_GET, LOG_ALL, 1, "\n");
                }
            }
        });
        *ret_info = &invalid_instr;
        return NULL;
    }

#ifdef INTERNAL
    DODEBUG({ /* rep & repne should have been completely handled by now */
        /* processor will typically ignore extra prefixes, but we log this internally
         * in case it's our decode messing up instead of weird app instrs
         */
        if (report_invalid &&
            ((rep_prefix &&
              /* case 6861: AMD64 opt: "rep ret" used if br tgt or after cbr */
              (pc != di->start_pc+2 || *(di->start_pc+1) != RAW_OPCODE_ret))
             || repne_prefix)) {
            char bytes[17*3];
            int i;
            dcontext_t *dcontext = get_thread_private_dcontext();
            IF_X64(bool old_mode = set_x86_mode(dcontext, di->x86_mode);)
            int sz = decode_sizeof(dcontext, di->start_pc, NULL _IF_X64(NULL));
            IF_X64(set_x86_mode(dcontext, old_mode));
            CLIENT_ASSERT(sz <= 17, "decode rep/repne error: unsupported opcode?");
            for (i=0; i<sz; i++)
                snprintf(&bytes[i*3], 3, "%02x ", *(di->start_pc+i));
            bytes[sz*3-1] = '\0'; /* -1 to kill trailing space */
            SYSLOG_INTERNAL_WARNING_ONCE("spurious rep/repne prefix @"PFX" (%s): "
                                         "decoding error?", di->start_pc, bytes);
        }
    });
#endif

    /* if just want opcode, stop here!  faster for caller to
     * separately call decode_next_pc than for us to decode immeds!
     */
    if (just_opcode) {
        *ret_info = info;
        return NULL;
    }

    if (data_prefix) {
        /* prefix was not part of opcode, it's a real prefix */
        /* From Intel manual:
         *   "For non-byte operations: if a 66H prefix is used with
         *   prefix (REX.W = 1), 66H is ignored."
         * That means non-byte-specific operations, for which 66H is
         * ignored as well, right?
         * Xref PR 593593.
         * Note that this means we could assert or remove some of
         * the "rex.w trumps data prefix" logic elsewhere in this file.
         */
        if (TEST(PREFIX_REX_W, di->prefixes)) {
            LOG(THREAD_GET, LOG_ALL, 3,
                "Ignoring 0x66 in presence of rex.w @"PFX"\n", di->start_pc);
        } else {
            di->prefixes |= PREFIX_DATA;
        }
    }

    /* read any trailing immediate bytes */
    if (info->dst1_type != TYPE_NONE)
        pc = read_operand(pc, di, info->dst1_type, info->dst1_size);
    if (info->dst2_type != TYPE_NONE)
        pc = read_operand(pc, di, info->dst2_type, info->dst2_size);
    if (info->src1_type != TYPE_NONE)
        pc = read_operand(pc, di, info->src1_type, info->src1_size);
    if (info->src2_type != TYPE_NONE)
        pc = read_operand(pc, di, info->src2_type, info->src2_size);
    if (info->src3_type != TYPE_NONE)
        pc = read_operand(pc, di, info->src3_type, info->src3_size);

    if (info->type == SUFFIX_EXT) {
        /* Shouldn't be any more bytes (immed bytes) read after the modrm+suffix! */
        DODEBUG({CLIENT_ASSERT(pc == post_suffix_pc, "decode error on 3DNow instr");});
    }

    /* return values */
    *ret_info = info;
    return pc;
}
#endif

/* Decodes the opcode and eflags usage of instruction at address pc
 * into instr.
 * This corresponds to a Level 2 decoding.
 * Assumes that instr is already initialized, but uses the x86/x64 mode
 * for the current thread rather than that set in instr.
 * If caller is re-using same instr struct over multiple decodings,
 * should call instr_reset or instr_reuse.
 * Returns the address of the next byte after the decoded instruction.
 * Returns NULL on decoding an invalid instruction.
 */
byte *
decode_opcode(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	// INPROCESSS decode_opcode
	printf("Starting decode.c decode_opcode\n");
//    const instr_info_t *info;
//    decode_info_t di;
//    int sz;
//    /* when pass true to read_instruction it doesn't decode immeds,
//     * so have to call decode_next_pc, but that ends up being faster
//     * than decoding immeds!
//     */
//    read_instruction(pc, pc, &info, &di, true /* just opcode */
//                     _IF_DEBUG(!TEST(INSTR_IGNORE_INVALID, instr->flags)));
//    sz = decode_sizeof(dcontext, pc, NULL _IF_X64(&rip_rel_pos));
//    IF_X64(instr_set_x86_mode(instr, get_x86_mode(dcontext)));
//    instr_set_opcode(instr, info->type);
//    /* read_instruction sets opcode to OP_INVALID for illegal instr.
//     * decode_sizeof will return 0 for _some_ illegal instrs, so we
//     * check it first since it's faster than instr_valid, but we have to
//     * also check instr_valid to catch all illegal instrs.
//     */
//    if (sz == 0 || !instr_valid(instr)) {
//        CLIENT_ASSERT(!instr_valid(instr), "decode_opcode: invalid instr");
//        return NULL;
//    }
//    instr->eflags = info->eflags;
//    instr_set_eflags_valid(instr, true);
//    /* operands are NOT set */
//    instr_set_operands_valid(instr, false);
//    /* raw bits are valid though and crucial for encoding */
//    instr_set_raw_bits(instr, pc, sz);
//    /* must set rip_rel_pos after setting raw bits */
//    IF_X64(instr_set_rip_rel_pos(instr, rip_rel_pos));
//    return pc + sz;
}
