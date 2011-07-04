.global dynamorio_sigreturn
dynamorio_sigreturn:
	mov	pc,	lr

.global dynamorio_sys_exit
dynamorio_sys_exit:
	mov pc, lr

.global hashlookup_null_handler
hashlookup_null_handler:
	B	hashlookup_null_handler

.global dynamorio_nonrt_sigreturn
dynamorio_nonrt_sigreturn:
	mov pc, lr

.global dynamorio_clone
dynamorio_clone:
	mov pc, lr

.global dr_setjmp
dr_setjmp:
	mov pc, lr

/* to avoid libc wrappers we roll our own syscall here
 * hardcoded to use int 0x80 for 32-bit -- FIXME: use something like do_syscall
 * and syscall for 64-bit.
 * signature: dynamorio_syscall(sysnum, num_args, arg1, arg2, ...)
 */
.global dynamorio_syscall
dynamorio_syscall:
	// We may have upto 4 extra parameters on the stack so need to save variables to make room
	push {r4,r5,r6,r7, lr}

    // Place the sys Number in right register
    mov		r7,	r0
    mov     r4, r1
    mov 	r1, r3
    mov     r0, r2
	// Need to see how many arguments there are
	cmp		r4, #3
	blt		syscall_2args
	beq		syscall_3args
	cmp		r4, #4
	beq		syscall_4args
	cmp		r4,	#5
	beq		syscall_5args

	ldr		r5,	[sp, #32]
syscall_5args:
	ldr		r4,	[sp, #28]
syscall_4args:
	ldr		r3,	[sp, #24]
syscall_3args:
	ldr		r2, [sp, #20]
syscall_2args:
 	svc		#0
 	pop 	{r4, r5, r6, r7, lr}
	mov 	pc, lr

/* ARM Equivalent of the Compare and set operation in X86.
   ATOMIC_COMPARE_EXCHANGE(*var, compare, exchange)*/
.global	ATOMIC_COMPARE_EXCHANGE_int
ATOMIC_COMPARE_EXCHANGE_int:
	// Load the value into memory
	push	{r4}
	ldrex	r3, [r0]
	cmp		r3,	r1			// Compare with old value
	strexeq	r4, r2, [r0]	// If successful then swap values
	cmp		r4, #0
	bne ATOMIC_COMPARE_EXCHANGE_int
	mov		r0,	r3		// Move the old value so it can be returned
	pop {r4}
	mov		pc,	lr		// return

/* uint atomic_swap(uint *addr, uint value)
 * return current contents of addr and replace contents with value.
 * on win32 could use InterlockedExchange intrinsic instead.
 */
.global atomic_swap
atomic_swap:
		ldrex	r2,	[r0]
		mov		r3, r1		// Copy value to temp
		mov		r1, r2		// replace value
		mov		r2,	r3		// replace other value
		strex	r3, r2,	[r0]
		cmp		r3, #0		// See if atomic instruction was sucessful
		bne atomic_swap
		mov 	pc, lr      // Return

// Load address in R0 and increment it atomically and store the result.
// NASTY FIX Had to do a movs r1, r1 to reset the condition code after strex
.global ATOMIC_INC_int
ATOMIC_INC_int:
		ldrex r1, [r0]		// Load value to be incremented
		adds	  r1, r1, #1	// Increment r1
		strex r2, r1, [r0]		// Store the result back in r1
		cmp	  r2, #0		// See if the store was successfull
		bne ATOMIC_INC_int
		movs  r1,  r1		// Re evaluate the condition code
		mov	  pc, lr		// Return

// Load address in R0 and decrement it atomically and store the result
// NASTY FIX Had to do a movs r1, r1 to reset the condition code after strex
.global ATOMIC_DEC_int
ATOMIC_DEC_int:
		ldrex r1, [r0]		// Load value to be decremented
		subs	  r1, r1, #1	// Decrement r1
		strex r2, r1, [r0]		// Store the result back in r1
		cmp	  r2, #0		// See if the store was successfull
		bne	ATOMIC_DEC_int
		movs  r1, r1		// Re evaluate the condition codes
		mov	  pc, lr		// Return

// Load the APSR and then test to see if desired flag is set
.global SET_FLAG
SET_FLAG:
		mrs		r1, CPSR	// Load the APSR into R1
		AND		r0, r0, r1	// See if the desired Bit is set
		mov		pc, lr		// Return

// Note this function isn't entirely atomic. As it assumes operand 2 isn't edited in any
// way
.global ATOMIC_ADD_int
ATOMIC_ADD_int:
		ldrex r2, [r0]		// Load the first operand
		add	  r2, r2, r1	// Add the two numbers together
		strex r3, r2, [r0]	// Store the result back again
		cmp   r3, #0
		bne   ATOMIC_ADD_int
		mov	  pc, lr		// return

// Function takes in a memory address and a value and then sets the max value to the same
// memory addresss (param 1 = address, param 2 = value)
.global ATOMIC_MAX_int
ATOMIC_MAX_int:
  		ldrex r2, [r0]		// Load the first operand
  		cmp	  r2,  r1		// Compare the 2 operands
  		movlt r2,  r1		// If r2 less than r1 swap it
  		strex r3, r2, [r0]	// Store the result
  		cmp   r3, #0
  		bne	  ATOMIC_MAX_int
  		mov	  pc, lr		// return

// Atomic 4 byte write. Use a simple str instruction as 4 bytes is the standard unit.
// R0 = target R1 = Value. I think this can be just a standard store as its only one instruction

.global atomic_4byte_write_asm
atomic_4byte_write_asm:
  		str		r1, [r0]		// Store the 4 bytes
  		mov		pc, lr			// Return

// Need to return the address of the stack
.global get_stack_ptr
get_stack_ptr:
		mov		r0, r13			// Return the address of the stack
		mov		pc, lr			// Return


