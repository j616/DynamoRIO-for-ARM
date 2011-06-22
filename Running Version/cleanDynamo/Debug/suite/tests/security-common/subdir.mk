################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/security-common/TestAllocWE.c \
../suite/tests/security-common/TestMemProtChg.c \
../suite/tests/security-common/codemod.c \
../suite/tests/security-common/decode-bad-stack.c \
../suite/tests/security-common/jmp_from_trace.c \
../suite/tests/security-common/ret_noncall_trace.c \
../suite/tests/security-common/retexisting.c \
../suite/tests/security-common/retnonexisting.c \
../suite/tests/security-common/selfmod-big.c \
../suite/tests/security-common/selfmod.c \
../suite/tests/security-common/selfmod2.c \
../suite/tests/security-common/vbjmp-rac-test.c 

OBJS += \
./suite/tests/security-common/TestAllocWE.o \
./suite/tests/security-common/TestMemProtChg.o \
./suite/tests/security-common/codemod.o \
./suite/tests/security-common/decode-bad-stack.o \
./suite/tests/security-common/jmp_from_trace.o \
./suite/tests/security-common/ret_noncall_trace.o \
./suite/tests/security-common/retexisting.o \
./suite/tests/security-common/retnonexisting.o \
./suite/tests/security-common/selfmod-big.o \
./suite/tests/security-common/selfmod.o \
./suite/tests/security-common/selfmod2.o \
./suite/tests/security-common/vbjmp-rac-test.o 

C_DEPS += \
./suite/tests/security-common/TestAllocWE.d \
./suite/tests/security-common/TestMemProtChg.d \
./suite/tests/security-common/codemod.d \
./suite/tests/security-common/decode-bad-stack.d \
./suite/tests/security-common/jmp_from_trace.d \
./suite/tests/security-common/ret_noncall_trace.d \
./suite/tests/security-common/retexisting.d \
./suite/tests/security-common/retnonexisting.d \
./suite/tests/security-common/selfmod-big.d \
./suite/tests/security-common/selfmod.d \
./suite/tests/security-common/selfmod2.d \
./suite/tests/security-common/vbjmp-rac-test.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/security-common/%.o: ../suite/tests/security-common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


