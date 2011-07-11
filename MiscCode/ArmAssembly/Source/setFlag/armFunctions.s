.global ATOMIC_DEC_int
ATOMIC_DEC_int:
	ldrex	r1, 	[r0]
	subs 	r1,	r1,	#1
	strex	r2,	r1,	[r0]
	cmp	r2,	#0
	bne	ATOMIC_DEC_int
	movs 	r1,	r1
	mov	pc,	lr

.global SET_FLAG
SET_FLAG:
	mrs	r1,	CPSR
	AND	r0,	r0,	r1
	mov	pc,	lr
