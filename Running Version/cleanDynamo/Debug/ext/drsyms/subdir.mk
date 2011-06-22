################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ext/drsyms/drsyms_windows.c 

OBJS += \
./ext/drsyms/drsyms_windows.o 

C_DEPS += \
./ext/drsyms/drsyms_windows.d 


# Each subdirectory must supply rules for building sources it contributes
ext/drsyms/%.o: ../ext/drsyms/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


