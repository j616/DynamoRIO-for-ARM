# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/james/Uni/summer11/code/Running Version/cleanDynamo"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/james/Uni/summer11/code/Running Version/cleanDynamo"

# Include any dependencies generated for this target.
include api/samples/CMakeFiles/stl_test.dir/depend.make

# Include the progress variables for this target.
include api/samples/CMakeFiles/stl_test.dir/progress.make

# Include the compile flags for this target's objects.
include api/samples/CMakeFiles/stl_test.dir/flags.make

api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o: api/samples/CMakeFiles/stl_test.dir/flags.make
api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o: api/samples/stl_test.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report "/home/james/Uni/summer11/code/Running Version/cleanDynamo/CMakeFiles" $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o"
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -m32 -fvisibility=internal -O3    -DX86_32 -DLINUX -DUSE_VISIBILITY_ATTRIBUTES -fno-stack-protector -o CMakeFiles/stl_test.dir/stl_test.cpp.o -c "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples/stl_test.cpp"

api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stl_test.dir/stl_test.cpp.i"
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -m32 -fvisibility=internal -O3    -DX86_32 -DLINUX -DUSE_VISIBILITY_ATTRIBUTES -fno-stack-protector -E "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples/stl_test.cpp" > CMakeFiles/stl_test.dir/stl_test.cpp.i

api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stl_test.dir/stl_test.cpp.s"
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -m32 -fvisibility=internal -O3    -DX86_32 -DLINUX -DUSE_VISIBILITY_ATTRIBUTES -fno-stack-protector -S "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples/stl_test.cpp" -o CMakeFiles/stl_test.dir/stl_test.cpp.s

api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.requires:
.PHONY : api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.requires

api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.provides: api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.requires
	$(MAKE) -f api/samples/CMakeFiles/stl_test.dir/build.make api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.provides.build
.PHONY : api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.provides

api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.provides.build: api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o

# Object files for target stl_test
stl_test_OBJECTS = \
"CMakeFiles/stl_test.dir/stl_test.cpp.o"

# External object files for target stl_test
stl_test_EXTERNAL_OBJECTS =

api/samples/bin/libstl_test.so: api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o
api/samples/bin/libstl_test.so: lib/libdynamorio.so.2.0
api/samples/bin/libstl_test.so: api/samples/CMakeFiles/stl_test.dir/build.make
api/samples/bin/libstl_test.so: api/samples/CMakeFiles/stl_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX shared library bin/libstl_test.so"
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/stl_test.dir/link.txt --verbose=$(VERBOSE)
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" && /usr/bin/cmake -E echo "Usage: pass to drconfig or drrun: -client /home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples/bin/libstl_test.so 0 \"\""

# Rule to build all files generated by this target.
api/samples/CMakeFiles/stl_test.dir/build: api/samples/bin/libstl_test.so
.PHONY : api/samples/CMakeFiles/stl_test.dir/build

api/samples/CMakeFiles/stl_test.dir/requires: api/samples/CMakeFiles/stl_test.dir/stl_test.cpp.o.requires
.PHONY : api/samples/CMakeFiles/stl_test.dir/requires

api/samples/CMakeFiles/stl_test.dir/clean:
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" && $(CMAKE_COMMAND) -P CMakeFiles/stl_test.dir/cmake_clean.cmake
.PHONY : api/samples/CMakeFiles/stl_test.dir/clean

api/samples/CMakeFiles/stl_test.dir/depend:
	cd "/home/james/Uni/summer11/code/Running Version/cleanDynamo" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/james/Uni/summer11/code/Running Version/cleanDynamo" "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" "/home/james/Uni/summer11/code/Running Version/cleanDynamo" "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples" "/home/james/Uni/summer11/code/Running Version/cleanDynamo/api/samples/CMakeFiles/stl_test.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : api/samples/CMakeFiles/stl_test.dir/depend

