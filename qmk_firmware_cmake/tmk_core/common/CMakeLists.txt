set(TMK_CORE_COMMON_DIR "${QMK_FIRMWARE}/tmk_core/common")

add_library(tmk_core_common INTERFACE)
target_include_directories(tmk_core_common INTERFACE ${TMK_CORE_COMMON_DIR})