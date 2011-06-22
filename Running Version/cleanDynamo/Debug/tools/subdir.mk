################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tools/DRcontrol.c \
../tools/DRkill.c \
../tools/DRload.c \
../tools/DRview.c \
../tools/balloon.c \
../tools/closewnd.c \
../tools/drdel.c \
../tools/drdeploy.c \
../tools/dummy.c \
../tools/hang_my_machine.c \
../tools/ldmp.c \
../tools/nudgeunix.c \
../tools/procstats.c \
../tools/run_in_bg.c \
../tools/runjob.c \
../tools/runstats.c \
../tools/svccntrl.c \
../tools/syscall.c \
../tools/win32injectall.c \
../tools/winstats.c 

OBJS += \
./tools/DRcontrol.o \
./tools/DRkill.o \
./tools/DRload.o \
./tools/DRview.o \
./tools/balloon.o \
./tools/closewnd.o \
./tools/drdel.o \
./tools/drdeploy.o \
./tools/dummy.o \
./tools/hang_my_machine.o \
./tools/ldmp.o \
./tools/nudgeunix.o \
./tools/procstats.o \
./tools/run_in_bg.o \
./tools/runjob.o \
./tools/runstats.o \
./tools/svccntrl.o \
./tools/syscall.o \
./tools/win32injectall.o \
./tools/winstats.o 

C_DEPS += \
./tools/DRcontrol.d \
./tools/DRkill.d \
./tools/DRload.d \
./tools/DRview.d \
./tools/balloon.d \
./tools/closewnd.d \
./tools/drdel.d \
./tools/drdeploy.d \
./tools/dummy.d \
./tools/hang_my_machine.d \
./tools/ldmp.d \
./tools/nudgeunix.d \
./tools/procstats.d \
./tools/run_in_bg.d \
./tools/runjob.d \
./tools/runstats.d \
./tools/svccntrl.d \
./tools/syscall.d \
./tools/win32injectall.d \
./tools/winstats.d 


# Each subdirectory must supply rules for building sources it contributes
tools/%.o: ../tools/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


