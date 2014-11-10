# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
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
CMAKE_COMMAND = /apps/cmake/2.8.8/bin/cmake

# The command to remove a file.
RM = /apps/cmake/2.8.8/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /apps/cmake/2.8.8/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/ybao/nektar++

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/ybao/nektar++/builds

# Include any dependencies generated for this target.
include library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/depend.make

# Include the progress variables for this target.
include library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/progress.make

# Include the compile flags for this target's objects.
include library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/flags.make

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/flags.make
library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o: ../library/Demos/StdRegions/StdProject_Diff2D.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /tmp/ybao/nektar++/builds/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o"
	cd /tmp/ybao/nektar++/builds/library/Demos/StdRegions && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -fPIC -pthread -o CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o -c /tmp/ybao/nektar++/library/Demos/StdRegions/StdProject_Diff2D.cpp

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.i"
	cd /tmp/ybao/nektar++/builds/library/Demos/StdRegions && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -fPIC -pthread -E /tmp/ybao/nektar++/library/Demos/StdRegions/StdProject_Diff2D.cpp > CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.i

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.s"
	cd /tmp/ybao/nektar++/builds/library/Demos/StdRegions && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -fPIC -pthread -S /tmp/ybao/nektar++/library/Demos/StdRegions/StdProject_Diff2D.cpp -o CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.s

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.requires:
.PHONY : library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.requires

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.provides: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.requires
	$(MAKE) -f library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/build.make library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.provides.build
.PHONY : library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.provides

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.provides.build: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o

# Object files for target StdProject_Diff2D
StdProject_Diff2D_OBJECTS = \
"CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o"

# External object files for target StdProject_Diff2D
StdProject_Diff2D_EXTERNAL_OBJECTS =

library/Demos/StdRegions/StdProject_Diff2D-3.4.0: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/build.make
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: library/StdRegions/libStdRegions.so.3.4.0
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: library/LibUtilities/libLibUtilities.so.3.4.0
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/fftw/3.3.2/lib/libfftw3.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/boost/1.49.0/lib/libboost_thread.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/boost/1.49.0/lib/libboost_iostreams.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/boost/1.49.0/lib/libboost_date_time.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/boost/1.49.0/lib/libboost_program_options.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/boost/1.49.0/lib/libboost_filesystem.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /apps/boost/1.49.0/lib/libboost_system.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libz.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/openmpi/lib/libmpi_cxx.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/openmpi/lib/libmpi.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/openmpi/lib/libopen-rte.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/openmpi/lib/libopen-pal.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libdl.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libnsl.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libutil.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libm.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libdl.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libnsl.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libutil.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/x86_64-linux-gnu/libm.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/liblapack.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: /usr/lib/libblas.so
library/Demos/StdRegions/StdProject_Diff2D-3.4.0: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable StdProject_Diff2D"
	cd /tmp/ybao/nektar++/builds/library/Demos/StdRegions && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/StdProject_Diff2D.dir/link.txt --verbose=$(VERBOSE)
	cd /tmp/ybao/nektar++/builds/library/Demos/StdRegions && $(CMAKE_COMMAND) -E cmake_symlink_executable StdProject_Diff2D-3.4.0 StdProject_Diff2D

library/Demos/StdRegions/StdProject_Diff2D: library/Demos/StdRegions/StdProject_Diff2D-3.4.0

# Rule to build all files generated by this target.
library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/build: library/Demos/StdRegions/StdProject_Diff2D
.PHONY : library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/build

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/requires: library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/StdProject_Diff2D.cpp.o.requires
.PHONY : library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/requires

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/clean:
	cd /tmp/ybao/nektar++/builds/library/Demos/StdRegions && $(CMAKE_COMMAND) -P CMakeFiles/StdProject_Diff2D.dir/cmake_clean.cmake
.PHONY : library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/clean

library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/depend:
	cd /tmp/ybao/nektar++/builds && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/ybao/nektar++ /tmp/ybao/nektar++/library/Demos/StdRegions /tmp/ybao/nektar++/builds /tmp/ybao/nektar++/builds/library/Demos/StdRegions /tmp/ybao/nektar++/builds/library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : library/Demos/StdRegions/CMakeFiles/StdProject_Diff2D.dir/depend

