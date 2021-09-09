cmake_minimum_required( VERSION 3.1 )

project( ItalianPlug )

set( GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_INSTALL OFF CACHE BOOL "GLFW lib only" FORCE)

add_subdirectory( external/glfw )
include_directories( 
	external/glfw/include 
	external/glm
	external/glad/include
	)

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

set( SRC
	 external/glad/src/glad.c
     src/main.cpp
     )

 add_executable( ItalianPlug WIN32 ${SRC} )
 target_link_libraries( ItalianPlug glfw )

if( MSVC ) 
	add_custom_command ( 
		TARGET ItalianPlug
		PRE_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/get_sources.bat
	)
else()
	add_custom_command ( 
		TARGET ItalianPlug
		PRE_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/get_sources.sh
	)
endif()

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'GLFW-CMake-starter' as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GLFW-CMake-starter )
    endif()
endif()

