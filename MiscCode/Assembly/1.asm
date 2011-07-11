section .data
	msg:		db	'Hello World',	10 ; The constant msg will contain Hello World
	msgLen: equ $-msg							 ; The constant msgLen will contain the length of msg
	
section .text
	global _start									 ; Tells the assembler which is the starting point
	
_start:
	
	mov eax, 4										 ; System call with number 4 is a system write
	mov ebx, 1										 ; First Parameter File Descriptor. 1= Standard output
	mov	ecx, msg									 ; Second parameter. What to Write. 'Hello World'
	mov edx, msgLen								 ; Third Parameter. Length of characters to write
	int 80h												 ; Interrupt. Perform System call
	
	mov	eax,	1										 ; System Call with number 1 is a system exit
	int 80h
