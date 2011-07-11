cmake_minimum_required (VERSION 2.6)
project (Tutorial)

# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings 
# to the source code
configure_file (
 "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
 "${PROJECT_BINARY_DIR}/TutorialConfig.h"
)

#Should we use our own math Functions
option (USE_ALTERNATIVE "Use a different output statement" ON)

# add the binary tree to the searh path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

if (USE_ALTERNATIVE)
  include_directories ("${PROJECT_SOURCE_DIR}/alternative")
  add_subdirectory (alternative)
  set (EXTRA_LIBS ${EXTRA_LIBS} alternativeOutput)
endif (USE_ALTERNATIVE)

add_executable(Tutorial tutorial.c)
target_link_libraries(Tutorial ${EXTRA_LIBS})
