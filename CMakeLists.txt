# -*- mode: cmake -*-

# Minimal version of CMake
cmake_minimum_required(VERSION 2.6.0)
cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# set some default options
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE_SHARED_LIBS ON)
set(CMAKE_C_FLAGS $ENV{CFLAGS})
set(CMAKE_CXX_FLAGS $ENV{CXXFLAGS})
set(CMAKE_LINK_FLAGS $ENV{LDFLAGS})

# include macros
include(MacroConfigureFile)

# uninstall
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY )
add_custom_target( uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" )

# Project name - wmfs
set(PROJECT_NAME wmfs)
set(VERSION "WMFS-201008")
project(${PROJECT_NAME} C)

find_package(PkgConfig REQUIRED)
# required packages
pkg_check_modules(FREETYPE REQUIRED freetype2)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XFT REQUIRED xft)

include_directories(
	${FREETYPE_INCLUDE_DIRS}
	${X11_INCLUDE_DIRS}
	${XFT_INCLUDE_DIRS}
)

set(CMAKE_LINK_LIBRARIES
	${FREETYPE_LIBRARIES}
	${X11_LIBRARIES}
	${XFT_LIBRARIES}
	-lpthread
)

# options
option(BUILD_DOC "Generate documentation using doxygen" OFF)
option(WITH_XINERAMA "Build with Xinerama support" ON)
option(WITH_XRANDR "Build with Xrandr support" ON)
option(WITH_IMLIB2 "Build with Imlib2 support" ON)

# optional finders
if(WITH_XINERAMA)
	add_definitions(-DHAVE_XINERAMA)
	pkg_check_modules(XINERAMA REQUIRED xinerama)
	include_directories(${XINERAMA_INCLUDE_DIRS})
	set(CMAKE_LINK_LIBRARIES ${XINERAMA_LIBRARIES} ${CMAKE_LINK_LIBRARIES})
else(WITH_XINERAMA)
	message(STATUS "Not building with Xinerama support")
endif(WITH_XINERAMA)

if(WITH_XRANDR)
	add_definitions(-DHAVE_XRANDR)
	pkg_check_modules(XRANDR REQUIRED xrandr)
	include_directories(${XRANDR_INCLUDE_DIRS})
	set(CMAKE_LINK_LIBRARIES ${XRANDR_LIBRARIES} ${CMAKE_LINK_LIBRARIES})
else(WITH_XRANDR)
	message(STATUS "Not building with Xrandr support")
endif(WITH_XRANDR)

if(WITH_IMLIB2)
	add_definitions(-DHAVE_IMLIB)
	pkg_check_modules(IMLIB2 REQUIRED imlib2)
	include_directories(${IMLIB2_INCLUDE_DIRS})
	set(CMAKE_LINK_LIBRARIES ${IMLIB2_LIBRARIES} ${CMAKE_LINK_LIBRARIES})
else(WITH_IMLIB2)
	message(STATUS "Not building with Imlib2 support")
endif(WITH_IMLIB2)

set(CMAKE_SYSCONFDIR ${CMAKE_INSTALL_PREFIX}/etc/ CACHE PATH "Config directory")
set(CMAKE_XDGCONFDIR ${CMAKE_SYSCONFDIR}/xdg/ CACHE PATH "XDG config directory")
set(CMAKE_DATADIR ${CMAKE_INSTALL_PREFIX}/share/ CACHE PATH "Data directory")
set(CMAKE_DOCDIR ${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${VERSION} CACHE PATH "Data directory")

# set default terminal for user
set(WMFS_TERM xterm)

# configure files
set(CONFIGURE_FILES
  src/config.h.in
  wmfs.doxygen.in
  wmfsrc.in)
foreach(file ${CONFIGURE_FILES})
    ConfigureFile(${file})
endforeach(file)

# add the source directory
add_subdirectory(src)

# documentation
if(BUILD_DOC)
	find_package(Doxygen REQUIRED)
	if(DOXYGEN_FOUND)
		add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/wmfs.doxygen
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	else(DOXYGEN_FOUND)
		MESSAGE(FATAL_ERROR "Doxygen executable not found")
	endif(DOXYGEN_FOUND)
endif(BUILD_DOC)

# Generating ChangeLog only for live version, for release we generate it to tarball
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/ChangeLog)
	find_program(GIT_EXECUTABLE git)
	if(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
		message(STATUS "<<< Generating ChangeLog... >>>")
		execute_process(
			COMMAND ${GIT_EXECUTABLE} log
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/ChangeLog)
	else(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
		message(STATUS "No ChangeLog present and git not found")
		message(STATUS "<<< Will not generate ChangeLog >>>")
	endif(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
endif(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/ChangeLog)

# manpage (compression handled by system not by us)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/wmfs.1 DESTINATION ${CMAKE_DATADIR}/man/man1)

# install docs
# changelog || can have 2 locations based on whether we generate it or not
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/ChangeLog)
	set(WMFS_DOCS ${CMAKE_CURRENT_BINARY_DIR}/ChangeLog ${WMFS_DOCS})
endif(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/ChangeLog)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog)
	set(WMFS_DOCS ${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog ${WMFS_DOCS})
endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog)
set(WMFS_DOCS
	${CMAKE_CURRENT_SOURCE_DIR}/TODO
	${CMAKE_CURRENT_SOURCE_DIR}/README
	${WMFS_DOCS}
)
install(FILES ${WMFS_DOCS} DESTINATION ${CMAKE_DOCDIR})

# config file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/wmfsrc DESTINATION ${CMAKE_SYSCONFDIR}/${PROJECT_NAME})

# xsession
install(FILES "wmfs.desktop" DESTINATION ${CMAKE_DATADIR}/xsessions)

# Status messages
message(STATUS "<<< ${PROJECT_NAME} ${VERSION} configuration >>>
		Build type      ${CMAKE_BUILD_TYPE}
		Install path    ${CMAKE_INSTALL_PREFIX}
		Compiler flags:
		C               ${CMAKE_C_FLAGS}
		C++             ${CMAKE_CXX_FLAGS}
		Linker flags:
		Executable      ${CMAKE_EXE_LINKER_FLAGS}
		Module          ${CMAKE_MODULE_LINKER_FLAGS}
		Shared          ${CMAKE_SHARED_LINKER_FLAGS}\n")
