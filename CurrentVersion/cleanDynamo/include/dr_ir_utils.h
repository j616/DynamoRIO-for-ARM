/* **********************************************************
 * Copyright (c) 2002-2009 VMware, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of VMware, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _DR_IR_UTILS_H_
#define _DR_IR_UTILS_H_ 1


/** 
 * Decodes only enough of the instruction at address \p pc to determine its size.
 * Returns that size.
 * If \p num_prefixes is non-NULL, returns the number of prefix bytes.
 * If \p rip_rel_pos is non-NULL, returns the offset into the instruction
 * of a rip-relative addressing displacement (for data only: ignores
 * control-transfer relative addressing), or 0 if none.
 * May return 0 size for certain invalid instructions.
 */
int 
decode_sizeof(void *drcontext, byte *pc, int *num_prefixes
              _IF_X64(uint *rip_rel_pos));

/** 
 * Decodes only enough of the instruction at address \p pc to determine its size.
 * Returns the address of the byte following the instruction.
 * Returns NULL on decoding an invalid instruction.
 */
byte *
decode_next_pc(void *drcontext, byte *pc);


#endif /* _DR_IR_UTILS_H_ */

/** 
 * Decodes only enough of the instruction at address \p pc to determine its size.
 * Returns that size.
 * If \p num_prefixes is non-NULL, returns the number of prefix bytes.
 * If \p rip_rel_pos is non-NULL, returns the offset into the instruction
 * of a rip-relative addressing displacement (for data only: ignores
 * control-transfer relative addressing), or 0 if none.
 * May return 0 size for certain invalid instructions.
 */
int 
decode_sizeof(void *drcontext, byte *pc, int *num_prefixes
              _IF_X64(uint *rip_rel_pos));

/** 
 * Decodes only enough of the instruction at address \p pc to determine its size.
 * Returns the address of the byte following the instruction.
 * Returns NULL on decoding an invalid instruction.
 */
byte *
decode_next_pc(void *drcontext, byte *pc);


#endif /* _DR_IR_UTILS_H_ */
