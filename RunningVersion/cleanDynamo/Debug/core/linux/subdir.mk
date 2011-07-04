################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../core/linux/diagnost.c \
../core/linux/module.c \
../core/linux/os.c \
../core/linux/pcprofile.c \
../core/linux/preload.c \
../core/linux/signal.c \
../core/linux/stackdump.c \
../core/linux/symtab.c 

OBJS += \
./core/linux/diagnost.o \
./core/linux/module.o \
./core/linux/os.o \
./core/linux/pcprofile.o \
./core/linux/preload.o \
./core/linux/signal.o \
./core/linux/stackdump.o \
./core/linux/symtab.o 

C_DEPS += \
./core/linux/diagnost.d \
./core/linux/module.d \
./core/linux/os.d \
./core/linux/pcprofile.d \
./core/linux/preload.d \
./core/linux/signal.d \
./core/linux/stackdump.d \
./core/linux/symtab.d 


# Each subdirectory must supply rules for building sources it contributes
core/linux/%.o: ../core/linux/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


