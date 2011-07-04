################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ext/drcontainers/hashtable.c 

OBJS += \
./ext/drcontainers/hashtable.o 

C_DEPS += \
./ext/drcontainers/hashtable.d 


# Each subdirectory must supply rules for building sources it contributes
ext/drcontainers/%.o: ../ext/drcontainers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


