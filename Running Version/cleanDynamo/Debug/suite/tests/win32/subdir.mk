################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../suite/tests/win32/aslr-dll.c \
../suite/tests/win32/callback.c \
../suite/tests/win32/crtprcs.c \
../suite/tests/win32/debugger.c \
../suite/tests/win32/delaybind.c \
../suite/tests/win32/delaybind.dll.c \
../suite/tests/win32/dll.c \
../suite/tests/win32/dll.dll.c \
../suite/tests/win32/except.c \
../suite/tests/win32/fiber-rac.c \
../suite/tests/win32/finally.c \
../suite/tests/win32/fpe.c \
../suite/tests/win32/getthreadcontext.c \
../suite/tests/win32/hooker-secur32.c \
../suite/tests/win32/hookerfirst.c \
../suite/tests/win32/hookerfirst.dll.c \
../suite/tests/win32/multisec.c \
../suite/tests/win32/multisec.dll.c \
../suite/tests/win32/nativeexec.c \
../suite/tests/win32/nativeexec.dll.c \
../suite/tests/win32/nativeterminate.c \
../suite/tests/win32/nativeterminate.dll.c \
../suite/tests/win32/oomtest.c \
../suite/tests/win32/partial_map.c \
../suite/tests/win32/protect-datasec.c \
../suite/tests/win32/rebased.c \
../suite/tests/win32/rebased.dll.c \
../suite/tests/win32/reload-newaddr.c \
../suite/tests/win32/reload-newaddr.dll.c \
../suite/tests/win32/reload-race.c \
../suite/tests/win32/reload-race.dll.c \
../suite/tests/win32/reload.c \
../suite/tests/win32/reload.dll.c \
../suite/tests/win32/rsbtest.c \
../suite/tests/win32/section-max.c \
../suite/tests/win32/section-max.dll.c \
../suite/tests/win32/setcxtsyscall.c \
../suite/tests/win32/setthreadcontext.c \
../suite/tests/win32/suspend.c \
../suite/tests/win32/threadchurn.c \
../suite/tests/win32/threadexit.c \
../suite/tests/win32/threadinjection.c \
../suite/tests/win32/tls.c \
../suite/tests/win32/virtualfree.c \
../suite/tests/win32/virtualreserve.c \
../suite/tests/win32/winapc.c \
../suite/tests/win32/winthread.c 

OBJS += \
./suite/tests/win32/aslr-dll.o \
./suite/tests/win32/callback.o \
./suite/tests/win32/crtprcs.o \
./suite/tests/win32/debugger.o \
./suite/tests/win32/delaybind.o \
./suite/tests/win32/delaybind.dll.o \
./suite/tests/win32/dll.o \
./suite/tests/win32/dll.dll.o \
./suite/tests/win32/except.o \
./suite/tests/win32/fiber-rac.o \
./suite/tests/win32/finally.o \
./suite/tests/win32/fpe.o \
./suite/tests/win32/getthreadcontext.o \
./suite/tests/win32/hooker-secur32.o \
./suite/tests/win32/hookerfirst.o \
./suite/tests/win32/hookerfirst.dll.o \
./suite/tests/win32/multisec.o \
./suite/tests/win32/multisec.dll.o \
./suite/tests/win32/nativeexec.o \
./suite/tests/win32/nativeexec.dll.o \
./suite/tests/win32/nativeterminate.o \
./suite/tests/win32/nativeterminate.dll.o \
./suite/tests/win32/oomtest.o \
./suite/tests/win32/partial_map.o \
./suite/tests/win32/protect-datasec.o \
./suite/tests/win32/rebased.o \
./suite/tests/win32/rebased.dll.o \
./suite/tests/win32/reload-newaddr.o \
./suite/tests/win32/reload-newaddr.dll.o \
./suite/tests/win32/reload-race.o \
./suite/tests/win32/reload-race.dll.o \
./suite/tests/win32/reload.o \
./suite/tests/win32/reload.dll.o \
./suite/tests/win32/rsbtest.o \
./suite/tests/win32/section-max.o \
./suite/tests/win32/section-max.dll.o \
./suite/tests/win32/setcxtsyscall.o \
./suite/tests/win32/setthreadcontext.o \
./suite/tests/win32/suspend.o \
./suite/tests/win32/threadchurn.o \
./suite/tests/win32/threadexit.o \
./suite/tests/win32/threadinjection.o \
./suite/tests/win32/tls.o \
./suite/tests/win32/virtualfree.o \
./suite/tests/win32/virtualreserve.o \
./suite/tests/win32/winapc.o \
./suite/tests/win32/winthread.o 

C_DEPS += \
./suite/tests/win32/aslr-dll.d \
./suite/tests/win32/callback.d \
./suite/tests/win32/crtprcs.d \
./suite/tests/win32/debugger.d \
./suite/tests/win32/delaybind.d \
./suite/tests/win32/delaybind.dll.d \
./suite/tests/win32/dll.d \
./suite/tests/win32/dll.dll.d \
./suite/tests/win32/except.d \
./suite/tests/win32/fiber-rac.d \
./suite/tests/win32/finally.d \
./suite/tests/win32/fpe.d \
./suite/tests/win32/getthreadcontext.d \
./suite/tests/win32/hooker-secur32.d \
./suite/tests/win32/hookerfirst.d \
./suite/tests/win32/hookerfirst.dll.d \
./suite/tests/win32/multisec.d \
./suite/tests/win32/multisec.dll.d \
./suite/tests/win32/nativeexec.d \
./suite/tests/win32/nativeexec.dll.d \
./suite/tests/win32/nativeterminate.d \
./suite/tests/win32/nativeterminate.dll.d \
./suite/tests/win32/oomtest.d \
./suite/tests/win32/partial_map.d \
./suite/tests/win32/protect-datasec.d \
./suite/tests/win32/rebased.d \
./suite/tests/win32/rebased.dll.d \
./suite/tests/win32/reload-newaddr.d \
./suite/tests/win32/reload-newaddr.dll.d \
./suite/tests/win32/reload-race.d \
./suite/tests/win32/reload-race.dll.d \
./suite/tests/win32/reload.d \
./suite/tests/win32/reload.dll.d \
./suite/tests/win32/rsbtest.d \
./suite/tests/win32/section-max.d \
./suite/tests/win32/section-max.dll.d \
./suite/tests/win32/setcxtsyscall.d \
./suite/tests/win32/setthreadcontext.d \
./suite/tests/win32/suspend.d \
./suite/tests/win32/threadchurn.d \
./suite/tests/win32/threadexit.d \
./suite/tests/win32/threadinjection.d \
./suite/tests/win32/tls.d \
./suite/tests/win32/virtualfree.d \
./suite/tests/win32/virtualreserve.d \
./suite/tests/win32/winapc.d \
./suite/tests/win32/winthread.d 


# Each subdirectory must supply rules for building sources it contributes
suite/tests/win32/%.o: ../suite/tests/win32/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


