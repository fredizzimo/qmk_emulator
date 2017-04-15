project(emulator)
get_filename_component(QMK_FIRMWARE "qmk_firmware" ABSOLUTE)
include_directories("${QMK_FIRMWARE}/keyboards/ergodox/infinity")
add_subdirectory(lib/cmake_wrapper/ugfx)


add_executable(emulator emulator/emulator_main.c)
target_link_libraries(emulator ugfx)