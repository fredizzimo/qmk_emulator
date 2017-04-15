project(emulator)
get_filename_component(QMK_FIRMWARE "qmk_firmware" ABSOLUTE)
include_directories("${QMK_FIRMWARE}/keyboards/ergodox/infinity")
add_subdirectory(lib/cmake_wrapper/ugfx)
add_subdirectory(lib/glad)
add_subdirectory(qmk_firmware_cmake/tmk_core/common)
add_subdirectory(qmk_firmware_cmake/visualizer)
add_subdirectory(qmk_firmware_cmake/serial_link)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

 add_subdirectory(lib/glfw)

add_executable(emulator 
    emulator/emulator_main.c
    emulator/shader.c
	${QMK_FIRMWARE}/keyboards/ergodox/infinity/visualizer.c
	${QMK_FIRMWARE}/keyboards/ergodox/infinity/animations.c
)

target_link_libraries(emulator visualizer serial_link ugfx glad glfw)
set_target_properties(emulator PROPERTIES LINK_FLAGS "/FORCE")