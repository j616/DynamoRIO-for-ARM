.global dynamorio_syscall
dynamorio_syscall:

  push {r4, r5 ,r6, r7, lr}
  # Place system number in right register
  mov r7, r0
  mov 	r4 , r1
  mov   r1, r3
  mov   r0, r2
  # Need to see how many arguements there are
  cmp 	r4, #3
  blt	syscall_2args
  cmp   r4, #3
  beq 	syscall_3args
  cmp   r4, #4
  beq   syscall_4args
  cmp   r4, #5
  beq   syscall_5args
  
  ldr   r5, [sp, #32]
syscall_5args:
  ldr 	r4, [sp, #28]
syscall_4args:
  ldr	r3, [sp, #24]
syscall_3args:
  ldr	r2, [sp, #20]

syscall_2args:
#  cmp	r7, #4
#  bne	here
#  cmp   r0, #1
#  bne   here
#  cmp   r2, #10
#  bne   here
#  ldr   r1, =message1
#  mov   r2, #26
#  svc   0
here:
  svc	#0
  pop   {r4, r5, r6, r7, lr}
  mov   pc, lr

message1: 
.asciz "CORRECT!!\n"
