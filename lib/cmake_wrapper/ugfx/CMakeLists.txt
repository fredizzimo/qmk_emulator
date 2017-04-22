set(UGFX_DIR "${QMK_FIRMWARE}/lib/ugfx")

add_library(ugfx 
    "${UGFX_DIR}/src/gfx_mk.c"
	"${UGFX_DIR}/src/gdisp/gdisp_pixmap.c"
)
set_source_files_properties(${UGFX_DIR}/src/gdisp/gdisp_pixmap.c PROPERTIES COMPILE_DEFINITIONS  GDISP_LLD_PIXELFORMAT=GDISP_PIXELFORMAT_RGB888)

target_include_directories(ugfx PUBLIC "${UGFX_DIR}")
if (${MSVC_IDE})
	target_compile_definitions(ugfx PUBLIC GFX_USE_OS_WIN32=TRUE)
else()
	target_compile_definitions(ugfx PUBLIC GFX_USE_OS_LINUX=TRUE)
endif()
target_link_libraries(ugfx emulator_gdisp)