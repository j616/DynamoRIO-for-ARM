################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../core/x86/arch.c \
../core/x86/decode.c \
../core/x86/decode_fast.c \
../core/x86/decode_table.c \
../core/x86/disassemble.c \
../core/x86/emit_utils.c \
../core/x86/encode.c \
../core/x86/instr.c \
../core/x86/instrument.c \
../core/x86/interp.c \
../core/x86/loadtoconst.c \
../core/x86/mangle.c \
../core/x86/optimize.c \
../core/x86/proc.c \
../core/x86/retcheck.c \
../core/x86/sideline.c \
../core/x86/steal_reg.c \
../core/x86/x86_code.c 

ASM_SRCS += \
../core/x86/asm_defines.asm \
../core/x86/pre_inject_asm.asm \
../core/x86/x86.asm 

OBJS += \
./core/x86/arch.o \
./core/x86/asm_defines.o \
./core/x86/decode.o \
./core/x86/decode_fast.o \
./core/x86/decode_table.o \
./core/x86/disassemble.o \
./core/x86/emit_utils.o \
./core/x86/encode.o \
./core/x86/instr.o \
./core/x86/instrument.o \
./core/x86/interp.o \
./core/x86/loadtoconst.o \
./core/x86/mangle.o \
./core/x86/optimize.o \
./core/x86/pre_inject_asm.o \
./core/x86/proc.o \
./core/x86/retcheck.o \
./core/x86/sideline.o \
./core/x86/steal_reg.o \
./core/x86/x86.o \
./core/x86/x86_code.o 

C_DEPS += \
./core/x86/arch.d \
./core/x86/decode.d \
./core/x86/decode_fast.d \
./core/x86/decode_table.d \
./core/x86/disassemble.d \
./core/x86/emit_utils.d \
./core/x86/encode.d \
./core/x86/instr.d \
./core/x86/instrument.d \
./core/x86/interp.d \
./core/x86/loadtoconst.d \
./core/x86/mangle.d \
./core/x86/optimize.d \
./core/x86/proc.d \
./core/x86/retcheck.d \
./core/x86/sideline.d \
./core/x86/steal_reg.d \
./core/x86/x86_code.d 


# Each subdirectory must supply rules for building sources it contributes
core/x86/%.o: ../core/x86/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

core/x86/%.o: ../core/x86/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	as  -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


