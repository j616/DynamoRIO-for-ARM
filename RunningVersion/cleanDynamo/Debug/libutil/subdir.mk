################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libutil/config.c \
../libutil/detach.c \
../libutil/dr_config.c \
../libutil/dumpevts.c \
../libutil/elm.c \
../libutil/mfapi.c \
../libutil/parser.c \
../libutil/policy.c \
../libutil/processes.c \
../libutil/services.c \
../libutil/tester.c \
../libutil/tests.c \
../libutil/utils.c 

OBJS += \
./libutil/config.o \
./libutil/detach.o \
./libutil/dr_config.o \
./libutil/dumpevts.o \
./libutil/elm.o \
./libutil/mfapi.o \
./libutil/parser.o \
./libutil/policy.o \
./libutil/processes.o \
./libutil/services.o \
./libutil/tester.o \
./libutil/tests.o \
./libutil/utils.o 

C_DEPS += \
./libutil/config.d \
./libutil/detach.d \
./libutil/dr_config.d \
./libutil/dumpevts.d \
./libutil/elm.d \
./libutil/mfapi.d \
./libutil/parser.d \
./libutil/policy.d \
./libutil/processes.d \
./libutil/services.d \
./libutil/tester.d \
./libutil/tests.d \
./libutil/utils.d 


# Each subdirectory must supply rules for building sources it contributes
libutil/%.o: ../libutil/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


