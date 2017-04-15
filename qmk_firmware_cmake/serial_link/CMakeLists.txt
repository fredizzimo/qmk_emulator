set(SERIAL_LINK_DIR "${QMK_FIRMWARE}/quantum/serial_link")

add_library(serial_link INTERFACE)
target_include_directories(serial_link INTERFACE ${SERIAL_LINK_DIR})