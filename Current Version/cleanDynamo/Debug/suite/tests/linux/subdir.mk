################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/linux/clone.c \
../suite/tests/linux/execve-null.c \
../suite/tests/linux/execve-rec.c \
../suite/tests/linux/execve-sub.c \
../suite/tests/linux/execve.c \
../suite/tests/linux/exit.c \
../suite/tests/linux/fork.c \
../suite/tests/linux/infinite.c \
../suite/tests/linux/infloop.c \
../suite/tests/linux/longjmp.c \
../suite/tests/linux/mmap.c \
../suite/tests/linux/signal0000.c \
../suite/tests/linux/signal0001.c \
../suite/tests/linux/signal0010.c \
../suite/tests/linux/signal0011.c \
../suite/tests/linux/signal0100.c \
../suite/tests/linux/signal0101.c \
../suite/tests/linux/signal0110.c \
../suite/tests/linux/signal0111.c \
../suite/tests/linux/signal1000.c \
../suite/tests/linux/signal1001.c \
../suite/tests/linux/signal1010.c \
../suite/tests/linux/signal1011.c \
../suite/tests/linux/signal1100.c \
../suite/tests/linux/signal1101.c \
../suite/tests/linux/signal1110.c \
../suite/tests/linux/signal1111.c \
../suite/tests/linux/sigplain000.c \
../suite/tests/linux/sigplain001.c \
../suite/tests/linux/sigplain010.c \
../suite/tests/linux/sigplain011.c \
../suite/tests/linux/sigplain100.c \
../suite/tests/linux/sigplain101.c \
../suite/tests/linux/sigplain110.c \
../suite/tests/linux/sigplain111.c \
../suite/tests/linux/thread.c \
../suite/tests/linux/threadexit.c \
../suite/tests/linux/threadexit2.c \
../suite/tests/linux/vfork-fib.c \
../suite/tests/linux/vfork.c 

OBJS += \
./suite/tests/linux/clone.o \
./suite/tests/linux/execve-null.o \
./suite/tests/linux/execve-rec.o \
./suite/tests/linux/execve-sub.o \
./suite/tests/linux/execve.o \
./suite/tests/linux/exit.o \
./suite/tests/linux/fork.o \
./suite/tests/linux/infinite.o \
./suite/tests/linux/infloop.o \
./suite/tests/linux/longjmp.o \
./suite/tests/linux/mmap.o \
./suite/tests/linux/signal0000.o \
./suite/tests/linux/signal0001.o \
./suite/tests/linux/signal0010.o \
./suite/tests/linux/signal0011.o \
./suite/tests/linux/signal0100.o \
./suite/tests/linux/signal0101.o \
./suite/tests/linux/signal0110.o \
./suite/tests/linux/signal0111.o \
./suite/tests/linux/signal1000.o \
./suite/tests/linux/signal1001.o \
./suite/tests/linux/signal1010.o \
./suite/tests/linux/signal1011.o \
./suite/tests/linux/signal1100.o \
./suite/tests/linux/signal1101.o \
./suite/tests/linux/signal1110.o \
./suite/tests/linux/signal1111.o \
./suite/tests/linux/sigplain000.o \
./suite/tests/linux/sigplain001.o \
./suite/tests/linux/sigplain010.o \
./suite/tests/linux/sigplain011.o \
./suite/tests/linux/sigplain100.o \
./suite/tests/linux/sigplain101.o \
./suite/tests/linux/sigplain110.o \
./suite/tests/linux/sigplain111.o \
./suite/tests/linux/thread.o \
./suite/tests/linux/threadexit.o \
./suite/tests/linux/threadexit2.o \
./suite/tests/linux/vfork-fib.o \
./suite/tests/linux/vfork.o 

C_DEPS += \
./suite/tests/linux/clone.d \
./suite/tests/linux/execve-null.d \
./suite/tests/linux/execve-rec.d \
./suite/tests/linux/execve-sub.d \
./suite/tests/linux/execve.d \
./suite/tests/linux/exit.d \
./suite/tests/linux/fork.d \
./suite/tests/linux/infinite.d \
./suite/tests/linux/infloop.d \
./suite/tests/linux/longjmp.d \
./suite/tests/linux/mmap.d \
./suite/tests/linux/signal0000.d \
./suite/tests/linux/signal0001.d \
./suite/tests/linux/signal0010.d \
./suite/tests/linux/signal0011.d \
./suite/tests/linux/signal0100.d \
./suite/tests/linux/signal0101.d \
./suite/tests/linux/signal0110.d \
./suite/tests/linux/signal0111.d \
./suite/tests/linux/signal1000.d \
./suite/tests/linux/signal1001.d \
./suite/tests/linux/signal1010.d \
./suite/tests/linux/signal1011.d \
./suite/tests/linux/signal1100.d \
./suite/tests/linux/signal1101.d \
./suite/tests/linux/signal1110.d \
./suite/tests/linux/signal1111.d \
./suite/tests/linux/sigplain000.d \
./suite/tests/linux/sigplain001.d \
./suite/tests/linux/sigplain010.d \
./suite/tests/linux/sigplain011.d \
./suite/tests/linux/sigplain100.d \
./suite/tests/linux/sigplain101.d \
./suite/tests/linux/sigplain110.d \
./suite/tests/linux/sigplain111.d \
./suite/tests/linux/thread.d \
./suite/tests/linux/threadexit.d \
./suite/tests/linux/threadexit2.d \
./suite/tests/linux/vfork-fib.d \
./suite/tests/linux/vfork.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/linux/%.o: ../suite/tests/linux/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


