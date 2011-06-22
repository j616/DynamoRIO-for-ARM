################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/common/broadfun.c \
../suite/tests/common/conflict.c \
../suite/tests/common/decode-bad.c \
../suite/tests/common/decode.c \
../suite/tests/common/eflags.c \
../suite/tests/common/fib.c \
../suite/tests/common/getretaddr.c \
../suite/tests/common/hello.c \
../suite/tests/common/ops.c \
../suite/tests/common/protect-dstack.c \
../suite/tests/common/recurse.c \
../suite/tests/common/segfault.c 

OBJS += \
./suite/tests/common/broadfun.o \
./suite/tests/common/conflict.o \
./suite/tests/common/decode-bad.o \
./suite/tests/common/decode.o \
./suite/tests/common/eflags.o \
./suite/tests/common/fib.o \
./suite/tests/common/getretaddr.o \
./suite/tests/common/hello.o \
./suite/tests/common/ops.o \
./suite/tests/common/protect-dstack.o \
./suite/tests/common/recurse.o \
./suite/tests/common/segfault.o 

C_DEPS += \
./suite/tests/common/broadfun.d \
./suite/tests/common/conflict.d \
./suite/tests/common/decode-bad.d \
./suite/tests/common/decode.d \
./suite/tests/common/eflags.d \
./suite/tests/common/fib.d \
./suite/tests/common/getretaddr.d \
./suite/tests/common/hello.d \
./suite/tests/common/ops.d \
./suite/tests/common/protect-dstack.d \
./suite/tests/common/recurse.d \
./suite/tests/common/segfault.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/common/%.o: ../suite/tests/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


