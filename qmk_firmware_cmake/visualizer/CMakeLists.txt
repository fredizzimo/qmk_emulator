set(VISUALIZER_DIR "${QMK_FIRMWARE}/quantum/visualizer")

add_library(visualizer STATIC
    ${VISUALIZER_DIR}/visualizer.c
    ${VISUALIZER_DIR}/lcd_backlight.c
    ${VISUALIZER_DIR}/lcd_keyframes.c
    ${VISUALIZER_DIR}/lcd_backlight_keyframes.c
    ${VISUALIZER_DIR}/led_keyframes.c
    ${VISUALIZER_DIR}/resources/lcd_logo.c
    ${VISUALIZER_DIR}/led_keyframes.c
	${VISUALIZER_DIR}/visualizer_keyframes.c
)
target_include_directories(visualizer PUBLIC ${VISUALIZER_DIR})
target_link_libraries(visualizer ugfx tmk_core_common)
target_compile_definitions(visualizer PUBLIC _USE_MATH_DEFINES VISUALIZER_ENABLE LCD_BACKLIGHT_ENABLE LCD_ENABLE BACKLIGHT_ENABLE)
