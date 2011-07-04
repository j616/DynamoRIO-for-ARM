################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include tools/subdir.mk
-include suite/tests/win32/subdir.mk
-include suite/tests/subdir.mk
-include suite/tests/security-win32/subdir.mk
-include suite/tests/security-linux/subdir.mk
-include suite/tests/security-common/subdir.mk
-include suite/tests/runall/subdir.mk
-include suite/tests/pthreads/subdir.mk
-include suite/tests/probe-api/subdir.mk
-include suite/tests/linux/subdir.mk
-include suite/tests/common/subdir.mk
-include suite/tests/client-interface/subdir.mk
-include suite/tests/api/subdir.mk
-include libutil/subdir.mk
-include ext/drsyms/subdir.mk
-include ext/drcontainers/subdir.mk
-include core/x86/subdir.mk
-include core/win32/subdir.mk
-include core/subdir.mk
-include core/linux/subdir.mk
-include core/lib/subdir.mk
-include clients/standalone/subdir.mk
-include build/tools/CMakeFiles/runstats.dir/subdir.mk
-include build/tools/CMakeFiles/run_in_bg.dir/subdir.mk
-include build/tools/CMakeFiles/nudgeunix.dir/subdir.mk
-include build/ext/drcontainers/CMakeFiles/drcontainers.dir/subdir.mk
-include build/core/CMakeFiles/dynamorio.dir/x86/subdir.mk
-include build/core/CMakeFiles/dynamorio.dir/subdir.mk
-include build/core/CMakeFiles/dynamorio.dir/linux/subdir.mk
-include build/core/CMakeFiles/drpreload.dir/x86/subdir.mk
-include build/core/CMakeFiles/drpreload.dir/linux/subdir.mk
-include build/core/CMakeFiles/drpreload.dir/subdir.mk
-include build/api/samples/CMakeFiles/tracedump.dir/subdir.mk
-include build/api/samples/CMakeFiles/strace.dir/subdir.mk
-include build/api/samples/CMakeFiles/stl_test.dir/subdir.mk
-include build/api/samples/CMakeFiles/signal.dir/subdir.mk
-include build/api/samples/CMakeFiles/prefetch.dir/subdir.mk
-include build/api/samples/CMakeFiles/memtrace.dir/subdir.mk
-include build/api/samples/CMakeFiles/instrcalls.dir/subdir.mk
-include build/api/samples/CMakeFiles/inline.dir/subdir.mk
-include build/api/samples/CMakeFiles/inc2add.dir/subdir.mk
-include build/api/samples/CMakeFiles/empty.dir/subdir.mk
-include build/api/samples/CMakeFiles/div.dir/subdir.mk
-include build/api/samples/CMakeFiles/countcalls.dir/subdir.mk
-include build/api/samples/CMakeFiles/cbr.dir/subdir.mk
-include build/api/samples/CMakeFiles/bbsize.dir/subdir.mk
-include build/api/samples/CMakeFiles/bbcount.dir/subdir.mk
-include build/CMakeFiles/CompilerIdC/subdir.mk
-include api/samples/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: cleanDynamo

# Tool invocations
cleanDynamo: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"cleanDynamo" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) cleanDynamo
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
