#
# CMake for wmfs David Demelier <markand@malikania.fr>
#

# General settings
cmake_minimum_required(VERSION 2.8)
project(wmfs)

set(CMAKE_C_FLAGS_RELEASE "-Wall")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")

# General option
option(WITH_XINERAMA "Build with X.Org xinerama support" ON)
option(WITH_XRANDR "Build with X.Org xrandr support" ON)
option(WITH_XFT "Build with X.Org xft support" ON)
option(WITH_IMLIB2 "Build with imlib2 graphic library" ON)

# WMFS Version and XDG directory
set(WMFS_VERSION "201106")
if (NOT XDG_CONFIG_DIR)
	set(XDG_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/etc/wmfs")
endif ()

# Man prefix
if (NOT MANPREFIX)
	set(MANPREFIX "${CMAKE_INSTALL_PREFIX}/share")
endif ()

# Libraries are optional
find_package(X11)
if (NOT X11_FOUND)
	message(FATAL_ERROR "You need x11 libraries to build wmfs")
else ()
	list(APPEND INCLUDES ${X11_INCLUDE_DIR})
	list(APPEND LIBRARIES ${X11_LIBRARIES})
endif ()

# pthread is needed
set(CMAKE_THREAD_PREFER_PTHREAD)
find_package(Threads)

if (NOT CMAKE_USE_PTHREADS_INIT)
	message(FATAL_ERROR "You need pthread libraries to build wmfs")
else ()
	list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif ()

# The following are optional X11 libraries
if (X11_Xinerama_FOUND AND WITH_XINERAMA)
	list(APPEND INCLUDES ${X11_Xinerama_INCLUDE_PATH})
	list(APPEND LIBRARIES ${X11_Xinerama_LIB})
	list(APPEND DEFINES "HAVE_XINERAMA")
else ()
	list(APPEND DISABLED "HAVE_XINERAMA")
endif ()

if (X11_Xrandr_FOUND AND WITH_XRANDR)
	list(APPEND INCLUDES ${X11_Xrandr_INCLUDE_PATH})
	list(APPEND LIBRARIES ${X11_Xrandr_LIB})
	list(APPEND DEFINES "HAVE_XRANDR")
else ()
	list(APPEND DISABLED "HAVE_XRANDR")
endif ()

if (X11_Xft_FOUND AND WITH_XFT)
	find_package(Freetype)
	if (FREETYPE_FOUND)
		list(APPEND INCLUDES ${FREETYPE_INCLUDE_DIRS}
		    ${X11_Xft_INCLUDE_PATH})
		list(APPEND LIBRARIES ${FREETYPE_LIBRARIES}
		    ${X11_Xft_LIB})
		list(APPEND DEFINES "HAVE_XFT")
	else ()
		list(APPEND DISABLED "HAVE_XFT")
	endif ()
endif ()

if (WITH_IMLIB2)
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(IMLIB2 imlib2)
		if (IMLIB2_FOUND)
			list(APPEND INCLUDES ${IMLIB2_INCLUDE_DIRS})
			list(APPEND LIBRARIES ${IMLIB2_LIBRARIES})
			list(APPEND DEFINES "HAVE_IMLIB")

			link_directories(${IMLIB2_LIBRARY_DIRS})
		else ()
			list(APPEND DISABLED "HAVE_IMLIB")
		endif ()
	else ()
		list(APPEND DISABLED "HAVE_IMLIB")
	endif ()
endif ()

# Enable the optional module to compilation
foreach (modname ${DEFINES})
	add_definitions(-D${modname})

	# Set a variable to print all enabled modules.
	# Remove the HAVE_ from module names
	string(SUBSTRING ${modname} 5 -1 upcase)
	string(TOLOWER ${upcase} module)

	message("INFO: ${module} enabled")
endforeach ()

# Show modules disabled
foreach (modname ${DISABLED})
	string(SUBSTRING ${modname} 5 -1 upcase)
	string(TOLOWER ${upcase} module)

	message("INFO: ${module} disabled")
endforeach ()

file(
	GLOB
	SOURCES
	src/*.c
     src/*.h
)

# Add definitions for the version and XDG
add_definitions(-DWMFS_VERSION=\"${WMFS_VERSION}\")
add_definitions(-DXDG_CONFIG_DIR=\"${XDG_CONFIG_DIR}\")

include_directories(${INCLUDES})
add_executable(wmfs ${SOURCES})
target_link_libraries(wmfs ${LIBRARIES})

# Install targets
install(TARGETS wmfs DESTINATION bin/)
install(FILES wmfsrc DESTINATION ${XDG_CONFIG_DIR}/)
install(FILES wmfs.1 DESTINATION ${MANPREFIX}/man1/)
