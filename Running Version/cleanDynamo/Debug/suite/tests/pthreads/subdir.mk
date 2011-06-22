################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/pthreads/pthreads.c \
../suite/tests/pthreads/ptsig.c 

OBJS += \
./suite/tests/pthreads/pthreads.o \
./suite/tests/pthreads/ptsig.o 

C_DEPS += \
./suite/tests/pthreads/pthreads.d \
./suite/tests/pthreads/ptsig.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/pthreads/%.o: ../suite/tests/pthreads/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


