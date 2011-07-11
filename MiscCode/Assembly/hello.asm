section .text
	global _start										; Must be declared for linker (ld)
	
_start:														; tell linker entry point

		mov		edx, len ; Message length
		mov		ecx, msg ; Message to write
		mov		ebx, 1	 ; File descriptor (stdout)
		mov		eax, 4	 ; System call number (sys_write)
		int		0x80		 ; call kernel
	
		mov		eax, 1 	 ; System call number (sys_exit)
		int		0x80		 ; call kernel
	
section .data

msg	db	'Hello, world!',0xa					; Our String
len	equ $ - msg											; Length of our string
