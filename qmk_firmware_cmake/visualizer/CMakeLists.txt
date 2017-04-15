set(VISUALIZER_DIR "${QMK_FIRMWARE}/quantum/visualizer")

add_library(visualizer ${VISUALIZER_DIR}/visualizer.c)
target_include_directories(visualizer PUBLIC ${VISUALIZER_DIR})
target_link_libraries(visualizer ugfx tmk_core_common)
