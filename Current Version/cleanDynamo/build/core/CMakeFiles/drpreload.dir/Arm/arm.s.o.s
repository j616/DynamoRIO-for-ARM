# 1 "/home/james/Uni/summer11/DynamoRIO-for-ARM/Current Version/cleanDynamo/core/Arm/arm.s"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/james/Uni/summer11/DynamoRIO-for-ARM/Current Version/cleanDynamo/core/Arm/arm.s"
.global dynamorio_sigreturn
dynamorio_sigreturn:
 mov pc, lr

.global dynamorio_sys_exit
dynamorio_sys_exit:
 mov pc, lr

.global hashlookup_null_handler
hashlookup_null_handler:
 B hashlookup_null_handler

.global dynamorio_nonrt_sigreturn
dynamorio_nonrt_sigreturn:
 mov pc, lr

.global dynamorio_clone
dynamorio_clone:
 mov pc, lr

.global dr_setjmp
dr_setjmp:
 mov pc, lr






.global dynamorio_syscall
dynamorio_syscall:

 push {r4,r5,r6,r7, lr}


    mov r7, r0
    mov r4, r1
    mov r1, r3
    mov r0, r2

 cmp r4, #3
 blt syscall_2args
 beq syscall_3args
 cmp r4, #4
 beq syscall_4args
 cmp r4, #5
 beq syscall_5args

 ldr r5, [sp, #32]
syscall_5args:
 ldr r4, [sp, #28]
syscall_4args:
 ldr r3, [sp, #24]
syscall_3args:
 ldr r2, [sp, #20]
syscall_2args:
  svc #0
  pop {r4, r5, r6, r7, lr}
 mov pc, lr



.global ATOMIC_COMPARE_EXCHANGE_int
ATOMIC_COMPARE_EXCHANGE_int:

 push {r4}
 ldrex r3, [r0]
 cmp r3, r1
 strexeq r4, r2, [r0]
 cmp r4, #0
 bne ATOMIC_COMPARE_EXCHANGE_int
 mov r0, r3
 pop {r4}
 mov pc, lr





.global atomic_swap
atomic_swap:
  ldrex r2, [r0]
  mov r3, r1
  mov r1, r2
  mov r2, r3
  strex r3, r2, [r0]
  cmp r3, #0
  bne atomic_swap
  mov pc, lr



.global ATOMIC_INC_int
ATOMIC_INC_int:
  ldrex r1, [r0]
  adds r1, r1, #1
  strex r2, r1, [r0]
  cmp r2, #0
  bne ATOMIC_INC_int
  movs r1, r1
  mov pc, lr



.global ATOMIC_DEC_int
ATOMIC_DEC_int:
  ldrex r1, [r0]
  subs r1, r1, #1
  strex r2, r1, [r0]
  cmp r2, #0
  bne ATOMIC_DEC_int
  movs r1, r1
  mov pc, lr


.global SET_FLAG
SET_FLAG:
  mrs r1, CPSR
  AND r0, r0, r1
  mov pc, lr



.global ATOMIC_ADD_int
ATOMIC_ADD_int:
  ldrex r2, [r0]
  add r2, r2, r1
  strex r3, r2, [r0]
  cmp r3, #0
  bne ATOMIC_ADD_int
  mov pc, lr



.global ATOMIC_MAX_int
ATOMIC_MAX_int:
    ldrex r2, [r0]
    cmp r2, r1
    movlt r2, r1
    strex r3, r2, [r0]
    cmp r3, #0
    bne ATOMIC_MAX_int
    mov pc, lr




.global atomic_4byte_write_asm
atomic_4byte_write_asm:
    str r1, [r0]
    mov pc, lr


.global get_stack_ptr
get_stack_ptr:
  mov r0, r13
  mov pc, lr
