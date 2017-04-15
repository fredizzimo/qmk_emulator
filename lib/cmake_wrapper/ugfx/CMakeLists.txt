set(UGFX_DIR "${QMK_FIRMWARE}/lib/ugfx")

add_library(ugfx "${UGFX_DIR}/src/gfx_mk.c")
target_include_directories(ugfx PUBLIC "${UGFX_DIR}")
target_compile_definitions(ugfx PUBLIC GFX_USE_OS_WIN32=TRUE)