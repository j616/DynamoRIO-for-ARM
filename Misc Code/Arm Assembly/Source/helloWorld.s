.syntax unified
.global _start

_start:
	ldr 	r1, =message
	mov	r0, #1
	mov	r2, #12
	mov	r7, #4
	svc	#0

	mov	r7, #1
	svc	#0
message:
.asciz	"Hello World\n"
