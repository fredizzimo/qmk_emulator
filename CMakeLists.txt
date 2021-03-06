project(emulator)
cmake_minimum_required(VERSION 3.2)
get_filename_component(QMK_FIRMWARE "qmk_firmware" ABSOLUTE)
include_directories("${QMK_FIRMWARE}/keyboards/ergodox/infinity")
add_definitions(-DEMULATOR)
set (CMAKE_C_STANDARD 99)

# There's a debug in the CMAKE, which makes the debuging not work
# So fix that
if (${MSVC_IDE})
	if ("${CMAKE_VS_PLATFORM_TOOLSET}" MATCHES "clang")
		add_compile_options("-g2;-gdwarf-2")
	endif()
endif()

add_subdirectory(lib/cmake_wrapper/ugfx)
add_subdirectory(lib/glad)
add_subdirectory(qmk_firmware_cmake/tmk_core/common)
add_subdirectory(qmk_firmware_cmake/visualizer)
add_subdirectory(qmk_firmware_cmake/serial_link)
add_subdirectory(drivers/gdisp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

 add_subdirectory(lib/glfw)

add_executable(emulator 
    emulator/emulator_main.c
    emulator/shader.c
	emulator/visualizer.c
	emulator/animations.c
)

target_link_libraries(emulator visualizer serial_link emulator_gdisp ugfx glad glfw)
target_include_directories(emulator PRIVATE .)
if (${MSVC_IDE})
	set_target_properties(emulator PROPERTIES LINK_FLAGS "/FORCE")
endif()