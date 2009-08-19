# -*- mode: cmake -*-
#Cmakelists.txt
# Minimum version of CMake
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(VERSION 2.6)
endif()

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Source and build dirs
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Project name - wmfs
set(PROJECT_NAME wmfs)
project(${PROJECT_NAME} C)

# Definition of the wmfs source
set(wmfs_src
  src/confparse/util.c
  src/confparse/confparse.c
  src/barwin.c
  src/client.c
# src/config_new.c
  src/config.c
  src/
  src/draw.c
  src/event.c
  src/ewmh.c
  src/frame.c
  src/getinfo.c
  src/infobar.c
  src/init.c
  src/launcher.c
  src/layout.c
  src/menu.c
  src/mouse.c
  src/screen.c
  src/tag.c
  src/util.c
  src/wmfs.c)

# Set the executable from the wmfs_src
add_executable(wmfs ${wmfs_src})

# Set the version
set(VERSION "WMFS-200908")

# FLAGS
set(CFLAGS "-g -Wall -ansi")
set(CMAKE_C_FLAGS ${CFLAGS})

# Linker FLAGS

set(DEFAULT_LDFLAGS "-L /usr/local/lib")

if(CMAKE_SYSTEM_NAME MATCHES NetBSD)
    message("-- NetBSD system found - Using /usr/pkg/lib for linker")
    set(LDFLAGS "${DEFAULT_LDFLAGS} -L /usr/pkg/lib")
else(CMAKE_SYSTEM_NAME MATCHES NetBSD)
    set(LDFLAGS ${DEFAULT_LDFLAGS})
endif(CMAKE_SYSTEM_NAME MATCHES NetBSD)

set_target_properties(wmfs PROPERTIES LINK_FLAGS ${LDFLAGS})
# Includes dir for libs in build_dir
include_directories(
  ${BUILD_DIR}/src
  )

# Package find
find_package(Freetype)
if(FREETYPE_FOUND)
  include_directories(${FREETYPE_INCLUDE_DIRS})
else (FREETYPE_FOUND)
  message(FATAL_ERROR "Could not find Freetype")
endif (FREETYPE_FOUND)

find_package(X11)
if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIR})
else (X11_FOUND)
  message(FATAL_ERROR "Could not find X11")
endif (X11_FOUND)

# Link Libraries
set(LIBRARIES_TO_LINK
  ${FREETYPE_LIBRARIES}
  ${X11_LIBRARIES}
  confuse
  Xft)

# Includes
include(FindDoxygen)
include(FindPkgConfig)

# Use pkgconfig to get required libraries
pkg_check_modules(WMFS_REQUIRED REQUIRED
  x11
  libconfuse
  freetype2
  xft)

# Check for xinerama
pkg_check_modules(HAVE_XINERAMA xinerama)
if(HAVE_XINERAMA_FOUND)
    set(WMFS_HAVE_XINERAMA "#define HAVE_XINERAMA")
    set(LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK} Xinerama)
else()
    set(WMFS_HAVE_XINERAMA "")
endif()

# Check for xrandr
pkg_check_modules(HAVE_XRANDR xrandr)
if(HAVE_XRANDR_FOUND)
    set(WMFS_HAVE_XRANDR "#define HAVE_XRANDR")
    set(LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK} Xrandr)
else()
    set(WMFS_HAVE_XRANDR "")
endif()

target_link_libraries(wmfs ${LIBRARIES_TO_LINK})

# Messages
message("Project version: ${VERSION}")
message("Using these CFLAGS: ${CFLAGS}")
message("Using these LDFLAGS: ${LDFLAGS}")
message("Linking with theses libraries : ${LIBRARIES_TO_LINK}")

# Generating man page
find_program(GZIP_EXECUTABLE gzip)
if(NOT GZIP_EXECUTABLE)
   message(STATUS "Looking for gzip -- not found")
   message(STATUS "Could not generating man page")
else()
   message(STATUS "Looking for gzip -- ${GZIP_EXECUTABLE}")
   message(STATUS "Generating man page")
   set(WMFS_MAN1_FILES ${BUILD_DIR}/wmfs.1.gz)
   execute_process(
      COMMAND ${GZIP_EXECUTABLE} -c wmfs.1
      WORKING_DIRECTORY ${SOURCE_DIR}
      OUTPUT_FILE ${WMFS_MAN1_FILES})
endif()

# Generating CHANGELOG
find_program(GIT_EXECUTABLE git)
if(EXISTS ${SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
   message(STATUS "Looking for git -- ${GIT_EXECUTABLE}")
   message(STATUS "Git dir -- Generating changelog...")
   set(PROJECT_CHANGELOG ${SOURCE_DIR}/changelog)
   execute_process(
      COMMAND ${GIT_EXECUTABLE} log
      WORKING_DIRECTORY ${SOURCE_DIR}
      OUTPUT_FILE ${PROJECT_CHANGELOG})
else()
   message(STATUS "Looking for git -- not found")
   message(STATUS "Could not generating changelog")
endif()

# sets
# {{{ Install path and configuration variables
if(DEFINED PREFIX)
    set(PREFIX ${PREFIX} CACHE PATH "install prefix")
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
    set(PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "install prefix")
endif()

#If a sysconfdir is specified, use it instead
#of the default configuration dir.
if(DEFINED SYSCONFDIR)
    set(SYSCONFDIR ${SYSCONFDIR} CACHE PATH "config directory")
else()
    set(SYSCONFDIR /etc CACHE PATH "config directory")
endif()

#If an XDG Config Dir is specificed, use it instead
#of the default XDG configuration dir.
if(DEFINED XDG_CONFIG_DIR)
    set(XDG_CONFIG_DIR ${XDG_CONFIG_SYS} CACHE PATH "xdg config directory")
else()
    set(XDG_CONFIG_DIR ${SYSCONFDIR}/xdg CACHE PATH "xdg config directory")
endif()

# setting WMFS_XSESSION_PATH
if(DEFINED WMFS_XSESSION_PATH)
    set(WMFS_XSESSION_PATH ${WMFS_XSESSION_PATH} CACHE PATH "wmfs xsessions directory")
else()
    set(WMFS_XSESSION_PATH ${PREFIX}/share/xsessions CACHE PATH "wmfs xsessions directory")
endif()

if(DEFINED WMFS_MAN_PATH)
   set(WMFS_MAN_PATH ${WMFS_MAN_PATH} CACHE PATH "wmfs manpage directory")
else()
   set(WMFS_MAN_PATH ${PREFIX}/share/man CACHE PATH "wmfs manpage directory")
endif()

if(DOXYGEN_EXECUTABLE)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${SOURCE_DIR}/wmfs.doxygen
    WORKING_DIRECTORY ${BUILD_DIR})
endif()

find_program(URXVT_EXECUTABLE urxvt)
if(URXVT_EXECUTABLE)
   set(WMFS_TERM urxvt)
else()
   set(WMFS_TERM xterm)
endif()

# Remplace strings in configs
set(WMFS_VERSION ${VERSION})
set(WMFS_COMPILE_MACHINE ${CMAKE_SYSTEM_PROCESSOR})
set(WMFS_COMPILE_BY $ENV{USER})
set(WMFS_COMPILE_FLAGS ${CFLAGS})
set(WMFS_LINKED_LIBS ${LIBRARIES_TO_LINK})
set(WMFS_SYSCONFDIR ${XDG_CONFIG_DIR}/${PROJECT_NAME})
set(WMFS_SOURCE_DIR ${SOURCE_DIR})

# Configure files
set(wmfs_configure_files
  src/config.h.in
  wmfs.doxygen.in
  wmfsrc.in)

macro(a_configure_file file)
    string(REGEX REPLACE ".in\$" "" outfile ${file})
    message(STATUS "Configuring ${outfile}")
    configure_file(${SOURCE_DIR}/${file}
                   ${SOURCE_DIR}/${outfile}
                   ESCAPE_QUOTE
                   @ONLY)
endmacro()

foreach(file ${wmfs_configure_files})
    a_configure_file(${file})
endforeach()

set(PROJECT_DATA_PATH share/${PROJECT_NAME})
set(PROJECT_TODO ${SOURCE_DIR}/TODO)
set(PROJECT_README ${SOURCE_DIR}/README)
set(PROJECT_DEFAULT_CONF ${SOURCE_DIR}/wmfsrc)

# installs
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if(WMFS_MAN1_FILES)
   install(FILES ${WMFS_MAN1_FILES} DESTINATION ${WMFS_MAN_PATH}/man1)
endif()

if(PROJECT_CHANGELOG)
   install(FILES ${PROJECT_CHANGELOG} DESTINATION ${PROJECT_DATA_PATH})
endif()

install(FILES ${PROJECT_TODO} ${PROJECT_README}
  ${PROJECT_DEFAULT_CONF} DESTINATION ${PROJECT_DATA_PATH})
install(FILES ${PROJECT_DEFAULT_CONF} DESTINATION ${WMFS_SYSCONFDIR})
install(FILES "wmfs.desktop" DESTINATION ${WMFS_XSESSION_PATH})
