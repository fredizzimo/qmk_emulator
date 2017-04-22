add_library(emulator_gdisp 
	emulator_lcd/emulator_lcd.c
	emulator_led/emulator_led.c
)

target_include_directories(emulator_gdisp PRIVATE private)
target_include_directories(emulator_gdisp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(emulator_gdisp ugfx)