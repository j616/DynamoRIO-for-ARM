################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/runall/detach_test.c \
../suite/tests/runall/earlythread.c \
../suite/tests/runall/earlythread.dll.c \
../suite/tests/runall/initapc.c \
../suite/tests/runall/initapc.dll.c \
../suite/tests/runall/preunload.c \
../suite/tests/runall/processchain.c 

OBJS += \
./suite/tests/runall/detach_test.o \
./suite/tests/runall/earlythread.o \
./suite/tests/runall/earlythread.dll.o \
./suite/tests/runall/initapc.o \
./suite/tests/runall/initapc.dll.o \
./suite/tests/runall/preunload.o \
./suite/tests/runall/processchain.o 

C_DEPS += \
./suite/tests/runall/detach_test.d \
./suite/tests/runall/earlythread.d \
./suite/tests/runall/earlythread.dll.d \
./suite/tests/runall/initapc.d \
./suite/tests/runall/initapc.dll.d \
./suite/tests/runall/preunload.d \
./suite/tests/runall/processchain.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/runall/%.o: ../suite/tests/runall/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


