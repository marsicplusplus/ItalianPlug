cmake_minimum_required( VERSION 3.1 )

project( ItalianPlug )

set( GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_INSTALL OFF CACHE BOOL "GLFW lib only" FORCE)

SET (ASSIMP_BUILD_TESTS OFF)

add_subdirectory( external/glfw )
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory( external/assimp )
add_subdirectory( external/spdlog )

include_directories( 
	external/glfw/include 
	external/glm
	external/glad/include
	external/imgui
	external/imgui/backends
	external/spdlog/include
	)

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

set( SRC
	 external/glad/src/glad.c
	 external/imgui/imgui_draw.cpp
	 external/imgui/imgui_tables.cpp
	 external/imgui/imgui_widgets.cpp
	 external/imgui/imgui.cpp
	 external/imgui/backends/imgui_impl_glfw.cpp
	 external/imgui/backends/imgui_impl_opengl3.cpp
	 src/main.cpp
	 src/renderer.cpp
	 src/mesh.cpp
	 src/shader.cpp
	 src/utils.cpp
	 src/input_handler.cpp
	 src/camera.cpp
	 src/mesh_map.cpp
	)

add_executable( ItalianPlug WIN32 ${SRC} )
target_link_libraries( ItalianPlug glfw assimp )
target_compile_options( ItalianPlug PRIVATE -g)
set_property(TARGET ItalianPlug PROPERTY CXX_STANDARD 17)
if( !MSVC )
	ADD_CUSTOM_TARGET( shaders ALL  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders )
else()
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/shaders" _dstDir)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders" _srcDir)
    execute_process(COMMAND cmd.exe /c mklink /J "${_dstDir}" "${_srcDir}")
endif()

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

