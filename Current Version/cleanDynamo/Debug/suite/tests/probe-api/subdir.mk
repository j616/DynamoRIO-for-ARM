################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/probe-api/insert_liboffs.c \
../suite/tests/probe-api/insert_liboffs.client.c 

OBJS += \
./suite/tests/probe-api/insert_liboffs.o \
./suite/tests/probe-api/insert_liboffs.client.o 

C_DEPS += \
./suite/tests/probe-api/insert_liboffs.d \
./suite/tests/probe-api/insert_liboffs.client.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/probe-api/%.o: ../suite/tests/probe-api/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


