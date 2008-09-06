#CMakeLists.txt

# Minimum version of CMake
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(VERSION 2.6)
endif()
# Source dir
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
# Project name - wmfs
project(wmfs C)
# Definition of the wmfs source
set(wmfs_src
  wmfs.c
  config.c)

# Set the executable from the wmfs_src
add_executable(wmfs ${wmfs_src})
# Set the version - NOT USED AT THE MOMENT
set(VERSION wmfs-devel)
# CFLAGS
set(CFLAGS "-g -Wall")
set(CMAKE_C_FLAGS ${CFLAGS})
# Link Libraries
set(LIBRARIES_TO_LINK
  X11
  confuse)
target_link_libraries(wmfs ${LIBRARIES_TO_LINK})

# Messages
message("Project version : ${VERSION}")
message("Using these CFLAGS : ${CFLAGS}")
message("Linking with theses libraries : ${LIBRARIES_TO_LINK}")

# Includes
include(FindPkgConfig)
#include(CPack) # need to configure it for packaging
# Use pkgconfig to get required libraries
pkg_check_modules(wmfs_required
  x11
  libconfuse)
# Find exterbal programs
find_program(GIT_EXECUTABLE git)
# Remplace strings in configs
set(WMFS_VERSION ${VERSION})
set(WMFS_COMPILE_MACHINE ${CMAKE_SYSTEM_PROCESSOR})
set(WMFS_COMPILE_BY $ENV{USER})
set(WMFS_COMPILE_FLAGS ${CFLAGS})
set(WMFS_LINKED_LIBS ${LIBRARIES_TO_LINK})
# Configure files
set(wmfs_configure_files
  config.h.in)

macro(a_configure_file file)
    string(REGEX REPLACE ".in\$" "" outfile ${file})
    message(STATUS "Configuring ${outfile}")
    configure_file(${SOURCE_DIR}/${file}
                   ${BUILD_DIR}/${outfile}
                   ESCAPE_QUOTE
                   @ONLY)
endmacro()

foreach(file ${wmfs_configure_files})
    a_configure_file(${file})
endforeach()

# Generating CHANGELOG

if(EXISTS ${SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
  # generate
execute_process(
  COMMAND ${GIT_EXECUTABLE} log
  WORKING_DIRECTORY ${SOURCE_DIR}
  #COMMENT "Generating CHANGELOG"
  OUTPUT_VARIABLE WMFS_CHANGELOG
  )
set(CHANGELOG_FILE ${SOURCE_DIR}/CHANGELOG)
file(WRITE ${CHANGELOG_FILE} ${WMFS_CHANGELOG})
endif()
