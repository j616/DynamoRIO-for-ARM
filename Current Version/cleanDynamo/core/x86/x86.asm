# define START_FILE .text

# define DECLARE_FUNC(symbol) \
.align 0 @N@\
.global symbol @N@\
.hidden symbol @N@\
.type symbol, %function

# define GLOBAL_LABEL(label) label

# define HEX(n) 0x##n

# define REG_XAX eax
# define REG_XBX ebx
# define REG_XCX ecx
# define REG_XDX edx
# define REG_XSI esi
# define REG_XDI edi
# define REG_XBP ebp
# define REG_XSP esp

# define DWORD dword ptr

/* Arguments are passed on stack right-to-left. */
# define ARG1 DWORD [4 + esp] /* includes ret addr */
# define ARG2 DWORD [8 + esp]
# define ARG3 DWORD [12 + esp]
# define ARG4 DWORD [16 + esp]
# define ARG5 DWORD [20 + esp]
# define ARG6 DWORD [24 + esp]

.text

DECLARE_FUNC(dynamorio_sigreturn)
GLOBAL_LABEL(dynamorio_sigreturn:)

/* we need to exit without using any stack, to support
 * THREAD_SYNCH_TERMINATED_AND_CLEANED.
 */
DECLARE_FUNC(dynamorio_sys_exit)
GLOBAL_LABEL(dynamorio_sys_exit:)

/* void hashlookup_null_handler(void)
 * PR 305731: if the app targets NULL, it ends up here, which indirects
 * through hashlookup_null_target to end up in an ibl miss routine.
 */
DECLARE_FUNC(hashlookup_null_handler)
GLOBAL_LABEL(hashlookup_null_handler:)
        /* We don't have any free registers to make this PIC so we patch
         * this up.  It would be better to generate than patch .text,
         * but we need a static address to reference in null_fragment
         * (though if we used shared ibl target_delete we could
         * set our final address prior to using null_fragment anywhere).
         */
        jmp      hashlookup_null_handler


DECLARE_FUNC(dynamorio_nonrt_sigreturn)
GLOBAL_LABEL(dynamorio_nonrt_sigreturn:)

/* SYS_clone swaps the stack so we need asm support to call it.
 * signature:
 *   thread_id_t dynamorio_clone(uint flags, byte *newsp, void *ptid, void *tls,
 *                               void *ctid, void (*func)(void))
 */
DECLARE_FUNC(dynamorio_clone)
GLOBAL_LABEL(dynamorio_clone:)

/* int cdecl dr_setjmp(dr_jmp_buf *buf);
 */
DECLARE_FUNC(dr_setjmp)
GLOBAL_LABEL(dr_setjmp:)

/* to avoid libc wrappers we roll our own syscall here
 * hardcoded to use int 0x80 for 32-bit -- FIXME: use something like do_syscall
 * and syscall for 64-bit.
 * signature: dynamorio_syscall(sysnum, num_args, arg1, arg2, ...)
 */
DECLARE_FUNC(dynamorio_syscall)
GLOBAL_LABEL(dynamorio_syscall:)
        /* x64 kernel doesn't clobber all the callee-saved registers */
        push     REG_XBX
        push     REG_XBP
        push     REG_XSI
        push     REG_XDI
        /* add 16 to skip the 4 pushes
         * FIXME: rather than this dispatch, could have separate routines
         * for each #args, or could just blindly read upward on the stack.
         * for dispatch, if assume size of mov instr can do single ind jmp */
        mov      ecx, [16+ 8 + esp] /* num_args */
        cmp      ecx, 0
        je       syscall_0args
        cmp      ecx, 1
        je       syscall_1args
        cmp      ecx, 2
        je       syscall_2args
        cmp      ecx, 3
        je       syscall_3args
        cmp      ecx, 4
        je       syscall_4args
        cmp      ecx, 5
        je       syscall_5args
        mov      ebp, [16+32 + esp] /* arg6 */
syscall_5args:
        mov      edi, [16+28 + esp] /* arg5 */
syscall_4args:
        mov      esi, [16+24 + esp] /* arg4 */
syscall_3args:
        mov      edx, [16+20 + esp] /* arg3 */
syscall_2args:
        mov      ecx, [16+16 + esp] /* arg2 */
syscall_1args:
        mov      ebx, [16+12 + esp] /* arg1 */
syscall_0args:
        mov      eax, [16+ 4 + esp] /* sysnum */
        /* PR 254280: we assume int$80 is ok even for LOL64 */
        int      HEX(80)
        pop      REG_XDI
        pop      REG_XSI
        pop      REG_XBP
        pop      REG_XBX
        /* return val is in eax for us */
        ret

/* uint atomic_swap(uint *addr, uint value)
 * return current contents of addr and replace contents with value.
 * on win32 could use InterlockedExchange intrinsic instead.
 */
DECLARE_FUNC(atomic_swap)
GLOBAL_LABEL(atomic_swap:)
        mov      REG_XAX, ARG2
        mov      REG_XCX, ARG1 /* nop on win64 (ditto for linux64 if used rdi) */
        xchg     [REG_XCX], eax
        ret

/* bool cpuid_supported(void)
 * Checks for existence of the cpuid instr by attempting to modify bit 21 of eflags
 */
        DECLARE_FUNC(cpuid_supported)
GLOBAL_LABEL(cpuid_supported:)
        PUSHF
        pop      REG_XAX
        mov      ecx, eax      /* original eflags in ecx */
        xor      eax, HEX(200000) /* try to modify bit 21 of eflags */
        push     REG_XAX
        POPF
        PUSHF
        pop      REG_XAX
        cmp      ecx, eax
        mov      eax, 0        /* zero out top bytes */
        setne    al
        push     REG_XCX         /* now restore original eflags */
        POPF
        ret

/* void our_cpuid(int res[4], int eax)
 * Executes cpuid instr, which is hard for x64 inline asm b/c clobbers rbx and can't
 * push in middle of func.
 */
        DECLARE_FUNC(our_cpuid)
GLOBAL_LABEL(our_cpuid:)
        mov      REG_XDX, ARG1
        mov      REG_XAX, ARG2
        push     REG_XBX /* callee-saved */
        push     REG_XDI /* callee-saved */
        /* not making a call so don't bother w/ 16-byte stack alignment */
        mov      REG_XDI, REG_XDX
        cpuid
        mov      [ 0 + REG_XDI], eax
        mov      [ 4 + REG_XDI], ebx
        mov      [ 8 + REG_XDI], ecx
        mov      [12 + REG_XDI], edx
        pop      REG_XDI /* callee-saved */
        pop      REG_XBX /* callee-saved */
        ret
