# qmk_emulator
An (eventual) emulator for qmk/qmk_firmware. 

At first this will only help you visualize the Ergodox Infinity visualizers. It runs a simple OpenGL program which draws a representation of the Infinity Ergodox, and shows the visualizer output on the LCD and LEDs.

The goal is to be able to run keymaps on your local PC, so that you are able to debug and and develop them more easilly. If the keyboard supports it, it will connect to the actual keyboard, so you can read the input from that, but all actual logic will happen inside the emulator.
