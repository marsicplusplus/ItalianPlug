cmake_minimum_required( VERSION 3.1 )

project( ItalianPlug )

set( GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only" FORCE)
set( GLFW_INSTALL OFF CACHE BOOL "GLFW lib only" FORCE)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OFF_IMPORTER TRUE)
set(ASSIMP_BUILD_PLY_IMPORTER TRUE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
# Needed for compilation?
set(ASSIMP_BUILD_ASSBIN_EXPORTER TRUE)
set(ASSIMP_BUILD_ASSXML_EXPORTER TRUE)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)
add_subdirectory( external/glfw )
add_subdirectory( external/assimp )

include_directories( 
    external/glfw/include 
    external/glm
    external/glad/include
    external/imgui
    external/imgui/backends
    external/libigl/include
    external/eigen
    external/quickhull
    external/quickhull/Structs
    )

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

set( SRC
     external/glad/src/glad.c
     external/imgui/imgui_draw.cpp
     external/imgui/imgui_tables.cpp
     external/imgui/imgui_widgets.cpp
     external/imgui/imgui.cpp
     external/imgui/backends/imgui_impl_glfw.cpp
     external/imgui/backends/imgui_impl_opengl3.cpp
     external/quickhull/QuickHull.cpp
     src/renderer.cpp
     src/mesh.cpp
     src/mesh_base.cpp
	 src/convex_hull.cpp
     src/shader.cpp
     src/utils.cpp
     src/normalization.cpp
     src/descriptors.cpp
     src/input_handler.cpp
     src/mesh_repair.cpp
     src/shader_map.cpp
     src/camera.cpp
     src/mesh_map.cpp
     src/unit_cube.cpp
)

if (UNIX) 
	set (LIBS 
		dl
		pthread
		glfw
		assimp)
else()
	set (LIBS 
		glfw
		assimp)
endif()

set(CXX_OPTIONS -g )
add_executable( ItalianPlug WIN32 ${SRC} src/italianplug.cpp)
target_link_libraries( ItalianPlug ${LIBS})
target_compile_options( ItalianPlug PRIVATE ${CXX_OPTIONS})
set_property(TARGET ItalianPlug PROPERTY CXX_STANDARD 17)

add_executable( FeatureExtractor WIN32 ${SRC} src/features_extractor.cpp)
target_link_libraries( FeatureExtractor ${LIBS})
target_compile_options( FeatureExtractor PRIVATE ${CXX_OPTIONS})
set_property(TARGET FeatureExtractor PROPERTY CXX_STANDARD 17)

add_executable( StatsExtractor WIN32 ${SRC} src/stats_extractor.cpp)
target_link_libraries( StatsExtractor ${LIBS})
target_compile_options( StatsExtractor PRIVATE ${CXX_OPTIONS})
set_property(TARGET StatsExtractor PROPERTY CXX_STANDARD 17)

add_executable( Normalizer WIN32 ${SRC} src/normalizer.cpp)
target_link_libraries( Normalizer ${LIBS})
target_compile_options( Normalizer PRIVATE ${CXX_OPTIONS})
set_property(TARGET Normalizer PROPERTY CXX_STANDARD 17)

if( UNIX )
    add_custom_command(
        TARGET ItalianPlug
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/Scripts ${CMAKE_BINARY_DIR}/Scripts)
    add_custom_command(
        TARGET ItalianPlug
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)
else()
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/shaders" _dstDir)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders" _srcDir)
    execute_process(COMMAND cmd.exe /c mklink /J "${_dstDir}" "${_srcDir}")
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/Scripts" _dstDir)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Scripts" _srcDir)
    execute_process(COMMAND cmd.exe /c mklink /J "${_dstDir}" "${_srcDir}")
endif()

if( WIN32 ) 
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

