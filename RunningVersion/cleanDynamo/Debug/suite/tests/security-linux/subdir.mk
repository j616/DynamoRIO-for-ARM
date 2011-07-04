################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/security-linux/stacktest.c \
../suite/tests/security-linux/trampoline.c 

OBJS += \
./suite/tests/security-linux/stacktest.o \
./suite/tests/security-linux/trampoline.o 

C_DEPS += \
./suite/tests/security-linux/stacktest.d \
./suite/tests/security-linux/trampoline.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/security-linux/%.o: ../suite/tests/security-linux/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


