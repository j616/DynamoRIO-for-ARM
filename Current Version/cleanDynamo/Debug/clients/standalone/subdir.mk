################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../clients/standalone/module_rsrc.c \
../clients/standalone/vista_hash.c \
../clients/standalone/winsysnums.c 

OBJS += \
./clients/standalone/module_rsrc.o \
./clients/standalone/vista_hash.o \
./clients/standalone/winsysnums.o 

C_DEPS += \
./clients/standalone/module_rsrc.d \
./clients/standalone/vista_hash.d \
./clients/standalone/winsysnums.d 


# Each subdirectory must supply rules for building sources it contributes
clients/standalone/%.o: ../clients/standalone/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


