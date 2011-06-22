################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../core/buildmark.c \
../core/config.c \
../core/dispatch.c \
../core/dynamo.c \
../core/emit.c \
../core/fcache.c \
../core/fragment.c \
../core/hashtable.c \
../core/heap.c \
../core/hotpatch.c \
../core/instrlist.c \
../core/io.c \
../core/link.c \
../core/loader_shared.c \
../core/module_list.c \
../core/moduledb.c \
../core/monitor.c \
../core/nudge.c \
../core/options.c \
../core/perfctr.c \
../core/perscache.c \
../core/rct.c \
../core/stats.c \
../core/synch.c \
../core/unit-rct.c \
../core/utils.c \
../core/vmareas.c 

OBJS += \
./core/buildmark.o \
./core/config.o \
./core/dispatch.o \
./core/dynamo.o \
./core/emit.o \
./core/fcache.o \
./core/fragment.o \
./core/hashtable.o \
./core/heap.o \
./core/hotpatch.o \
./core/instrlist.o \
./core/io.o \
./core/link.o \
./core/loader_shared.o \
./core/module_list.o \
./core/moduledb.o \
./core/monitor.o \
./core/nudge.o \
./core/options.o \
./core/perfctr.o \
./core/perscache.o \
./core/rct.o \
./core/stats.o \
./core/synch.o \
./core/unit-rct.o \
./core/utils.o \
./core/vmareas.o 

C_DEPS += \
./core/buildmark.d \
./core/config.d \
./core/dispatch.d \
./core/dynamo.d \
./core/emit.d \
./core/fcache.d \
./core/fragment.d \
./core/hashtable.d \
./core/heap.d \
./core/hotpatch.d \
./core/instrlist.d \
./core/io.d \
./core/link.d \
./core/loader_shared.d \
./core/module_list.d \
./core/moduledb.d \
./core/monitor.d \
./core/nudge.d \
./core/options.d \
./core/perfctr.d \
./core/perscache.d \
./core/rct.d \
./core/stats.d \
./core/synch.d \
./core/unit-rct.d \
./core/utils.d \
./core/vmareas.d 


# Each subdirectory must supply rules for building sources it contributes
core/%.o: ../core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


