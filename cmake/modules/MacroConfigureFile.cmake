macro(ConfigureFile file)
	string(REGEX REPLACE ".in\$" "" outfile ${file})
	message(STATUS "<<< Configuring ${outfile} >>>")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file}
		${CMAKE_CURRENT_BINARY_DIR}/${outfile}
		ESCAPE_QUOTE
		@ONLY)
endmacro(ConfigureFile)
