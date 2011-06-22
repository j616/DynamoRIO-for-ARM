################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/api/dis.c \
../suite/tests/api/ir.c \
../suite/tests/api/startstop.c 

OBJS += \
./suite/tests/api/dis.o \
./suite/tests/api/ir.o \
./suite/tests/api/startstop.o 

C_DEPS += \
./suite/tests/api/dis.d \
./suite/tests/api/ir.d \
./suite/tests/api/startstop.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/api/%.o: ../suite/tests/api/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


