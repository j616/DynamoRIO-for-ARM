# 1 "/home/james/Uni/summer11/code/Running Version/cleanDynamo/core/x86/x86.asm"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/james/Uni/summer11/code/Running Version/cleanDynamo/core/x86/x86.asm"
# 32 "/home/james/Uni/summer11/code/Running Version/cleanDynamo/core/x86/x86.asm"
.text

.align 0 
.global dynamorio_sigreturn 
.hidden dynamorio_sigreturn 
.type dynamorio_sigreturn, %function
dynamorio_sigreturn:




.align 0 
.global dynamorio_sys_exit 
.hidden dynamorio_sys_exit 
.type dynamorio_sys_exit, %function
dynamorio_sys_exit:





.align 0 
.global hashlookup_null_handler 
.hidden hashlookup_null_handler 
.type hashlookup_null_handler, %function
hashlookup_null_handler:






        jmp hashlookup_null_handler


.align 0 
.global dynamorio_nonrt_sigreturn 
.hidden dynamorio_nonrt_sigreturn 
.type dynamorio_nonrt_sigreturn, %function
dynamorio_nonrt_sigreturn:






.align 0 
.global dynamorio_clone 
.hidden dynamorio_clone 
.type dynamorio_clone, %function
dynamorio_clone:



.align 0 
.global dr_setjmp 
.hidden dr_setjmp 
.type dr_setjmp, %function
dr_setjmp:






.align 0 
.global dynamorio_syscall 
.hidden dynamorio_syscall 
.type dynamorio_syscall, %function
dynamorio_syscall:

        push ebx
        push ebp
        push esi
        push edi




        mov ecx, [16+ 8 + esp]
        cmp ecx, 0
        je syscall_0args
        cmp ecx, 1
        je syscall_1args
        cmp ecx, 2
        je syscall_2args
        cmp ecx, 3
        je syscall_3args
        cmp ecx, 4
        je syscall_4args
        cmp ecx, 5
        je syscall_5args
        mov ebp, [16+32 + esp]
syscall_5args:
        mov edi, [16+28 + esp]
syscall_4args:
        mov esi, [16+24 + esp]
syscall_3args:
        mov edx, [16+20 + esp]
syscall_2args:
        mov ecx, [16+16 + esp]
syscall_1args:
        mov ebx, [16+12 + esp]
syscall_0args:
        mov eax, [16+ 4 + esp]

        int 0x80
        pop edi
        pop esi
        pop ebp
        pop ebx

        ret





.align 0 
.global atomic_swap 
.hidden atomic_swap 
.type atomic_swap, %function
atomic_swap:
        mov eax, dword ptr [8 + esp]
        mov ecx, dword ptr [4 + esp]
        xchg [ecx], eax
        ret




        .align 0 
.global cpuid_supported 
.hidden cpuid_supported 
.type cpuid_supported, %function
cpuid_supported:
        PUSHF
        pop eax
        mov ecx, eax
        xor eax, 0x200000
        push eax
        POPF
        PUSHF
        pop eax
        cmp ecx, eax
        mov eax, 0
        setne al
        push ecx
        POPF
        ret





        .align 0 
.global our_cpuid 
.hidden our_cpuid 
.type our_cpuid, %function
our_cpuid:
        mov edx, dword ptr [4 + esp]
        mov eax, dword ptr [8 + esp]
        push ebx
        push edi

        mov edi, edx
        cpuid
        mov [ 0 + edi], eax
        mov [ 4 + edi], ebx
        mov [ 8 + edi], ecx
        mov [12 + edi], edx
        pop edi
        pop ebx
        ret
