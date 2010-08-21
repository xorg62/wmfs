# -*- mode: cmake -*-

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parse)

SET(WMFS_SOURCES
	parse/parse.c
	barwin.c
	client.c
	config.c
	draw.c
	event.c
	ewmh.c
	frame.c
	getinfo.c
	infobar.c
	init.c
	launcher.c
	layout.c
	menu.c
	mouse.c
	screen.c
	status.c
	systray.c
	tag.c
	util.c
	viwmfs.c
	wmfs.c
)

add_executable(${PROJECT_NAME} ${WMFS_SOURCES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_LINK_LIBRARIES})
INSTALL ( TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib${LIB_SUFFIX} )