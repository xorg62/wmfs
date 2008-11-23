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

# Source dir
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

#set(CMAKE_BUILD_TYPE RELEASE)
option(WITH_LOGOS "install wmfs logos")
option(GENERATE_MANPAGES "generate manpages" ON)

# Project name - wmfs
set(PROJECT_NAME wmfs)
project(${PROJECT_NAME} C)

# Definition of the wmfs source
set(wmfs_src
  src/bar.c
  src/client.c
  src/config.c
  src/draw.c
  src/event.c
  src/frame.c
  src/infobar.c
  src/init.c
  src/layout.c
  src/mouse.c
  src/tag.c
  src/util.c
  src/wmfs.c)

# Set the executable from the wmfs_src
add_executable(wmfs ${wmfs_src})

# Set the version - NOT USED AT THE MOMENT
set(VERSION "0.1rc2 (Brain Damage)")

# FLAGS
set(CFLAGS "-g -Wall")
set(CMAKE_C_FLAGS ${CFLAGS})
set(LDFLAGS "-L /usr/local/lib")
set_target_properties(wmfs PROPERTIES LINK_FLAGS ${LDFLAGS})

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

target_link_libraries(wmfs ${LIBRARIES_TO_LINK})

# Messages
message("Project version: ${VERSION}")
message("Using these CFLAGS: ${CFLAGS}")
message("Using these LDFLAGS: ${LDFLAGS}")
message("Linking with theses libraries : ${LIBRARIES_TO_LINK}")

# Includes
include(FindDoxygen)
include(FindPkgConfig)

# Use pkgconfig to get required libraries
pkg_check_modules(wmfs_required
  x11
  libconfuse
  freetype2
  xft)

# Find exterbal programs
macro(a_find_program var prg req)
    set(required ${req})
    find_program(${var} ${prg})
    if(NOT ${var})
        message(STATUS "${prg} not found.")
        if(required)
            message(FATAL_ERROR "${prg} is required to build wmfs")
        endif()
    else()
        message(STATUS "${prg} -> ${${var}}")
    endif()
endmacro()
find_program(GIT_EXECUTABLE git)

# programs needed for man pages
a_find_program(ASCIIDOC_EXECUTABLE asciidoc FALSE)
a_find_program(XMLTO_EXECUTABLE xmlto FALSE)
a_find_program(GZIP_EXECUTABLE gzip FALSE)
a_find_program(CAT_EXECUTABLE cat TRUE)

# Generating CHANGELOG

if(EXISTS ${SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)

  # generate
message("Git dir -- Generating changelog...")
execute_process(
  COMMAND ${GIT_EXECUTABLE} log
  WORKING_DIRECTORY ${SOURCE_DIR}
  OUTPUT_VARIABLE WMFS_CHANGELOG
  )
set(CHANGELOG_FILE ${SOURCE_DIR}/CHANGELOG)
file(WRITE ${CHANGELOG_FILE} ${WMFS_CHANGELOG})

else()

message("Non-Git dir -- Generating fake changelog")
execute_process(
  COMMAND touch ${SOURCE_DIR}/CHANGELOG
  )

endif()

# {{{ Check if documentation can be build
if(GENERATE_MANPAGES)
    if(NOT ASCIIDOC_EXECUTABLE OR NOT XMLTO_EXECUTABLE OR NOT GZIP_EXECUTABLE)
        if(NOT ASCIIDOC_EXECUTABLE)
            SET(missing "asciidoc")
        endif()
        if(NOT XMLTO_EXECUTABLE)
            SET(missing ${missing} " xmlto")
        endif()
        if(NOT GZIP_EXECUTABLE)
            SET(missing ${missing} " gzip")
        endif()

        message(STATUS "Not generating manpages. Missing: " ${missing})
        set(GENERATE_MANPAGES OFF)
    endif()
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

# sets
# {{{ Install path and configuration variables
if(DEFINED PREFIX)
    set(PREFIX ${PREFIX} CACHE PATH "install prefix")
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
    set(PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "install prefix")
endif()

# set man path
set(WMFS_MAN_SRCS
  ${SOURCE_DIR}/wmfs.1.txt)

if(DEFINED WMFS_MAN_PATH)
   set(WMFS_MAN_PATH ${WMFS_MAN_PATH} CACHE PATH "wmfs manpage directory")
else()
   set(WMFS_MAN_PATH ${PREFIX}/share/man CACHE PATH "wmfs manpage directory")
endif()

# {{{ Manpages
if(GENERATE_MANPAGES)
    foreach(txtfile ${WMFS_MAN_SRCS})
        string(REGEX REPLACE ".txt\$" ".xml" xmlfile ${txtfile})
        string(REPLACE ${SOURCE_DIR}
                       ${BUILD_DIR} xmlfile ${xmlfile})
        string(REGEX REPLACE ".xml\$" ".gz" gzfile ${xmlfile})
        string(REGEX REPLACE ".gz\$" "" manfile ${gzfile})

        add_custom_command(
            COMMAND ${CAT_EXECUTABLE} ${txtfile}
                    | ${ASCIIDOC_EXECUTABLE} -d manpage -b docbook -o ${xmlfile} -
            WORKING_DIRECTORY ${BUILD_DIR}
            OUTPUT  ${xmlfile}
            DEPENDS ${txtfile}
            VERBATIM)
        add_custom_command(
            COMMAND ${XMLTO_EXECUTABLE} man ${xmlfile}
            OUTPUT  ${manfile}
            DEPENDS ${xmlfile})
        add_custom_command(
            COMMAND ${CAT_EXECUTABLE} ${manfile}
                    | ${GZIP_EXECUTABLE}
                    > ${gzfile}
            OUTPUT  ${gzfile}
            DEPENDS ${manfile}
            VERBATIM)

        set(MAN_FILES ${MAN_FILES} ${gzfile})

        # figure out in what section to install to from filename
        string(REGEX REPLACE "^.*\\.([0-9])\\.gz\$" "\\1" section ${gzfile})
        set(WMFS_MAN${section}_FILES ${WMFS_MAN${section}_FILES} ${gzfile})
    endforeach()

    add_custom_target(man ALL DEPENDS ${MAN_FILES})
endif()
# }}}

# {{ Doxygen
if(DOXYGEN_EXECUTABLE)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} wmfs.doxygen
    WORKING_DIRECTORY ${BUILD_DIR})
endif()
# }}

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
  wmfs.1.txt.in
  wmfs.doxygen.in)

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

set(PROJECT_DATA_PATH share/${PROJECT_NAME})
set(PROJECT_TODO ${SOURCE_DIR}/TODO)
set(PROJECT_README ${SOURCE_DIR}/README)
set(PROJECT_CHANGELOG ${SOURCE_DIR}/CHANGELOG)
set(PROJECT_DEFAULT_CONF ${SOURCE_DIR}/wmfsrc)

# installs
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
if(WITH_LOGOS)
  set(PROJECT_LOGOS ${PROJECT_DATA_PATH}/logos)
  set(PROJECT_LOGOS_DIR ${SOURCE_DIR}/logos)
  install(DIRECTORY ${PROJECT_LOGOS_DIR} DESTINATION ${PROJECT_DATA_PATH})
  message("Install With logos")
endif()
if(GENERATE_MANPAGES)
      install(FILES ${WMFS_MAN1_FILES} DESTINATION ${WMFS_MAN_PATH}/man1)
endif()
install(FILES ${PROJECT_TODO} ${PROJECT_README} ${PROJECT_CHANGELOG}
  ${PROJECT_DEFAULT_CONF} DESTINATION ${PROJECT_DATA_PATH})
install(FILES ${PROJECT_DEFAULT_CONF} DESTINATION ${WMFS_SYSCONFDIR})
