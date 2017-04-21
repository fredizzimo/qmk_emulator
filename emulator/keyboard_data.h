/*
Copyright 2016 Fred Sundvik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EMULATOR_KEYBOARD_DATA_H_
#define EMULATOR_KEYBOARD_DATA_H_

point screen_dimensions = { 1024, 768 };

typedef struct {
    point pos;
    float size;
    float rot;
} keyinfo_t;

static const point keyboard_pos = {10, 10};

static const color_t background_color = RGB2COLOR(0x8B, 0x45, 0x13);

static color_t main_keyboard_area_color = RGB2COLOR(196, 195, 192);
static const color_t keycap_outer_color = RGB2COLOR(0, 0, 0);
static const color_t keycap_inner_color = RGB2COLOR(3, 3, 3);

static const float lcd_base_intensity = 150.0f / 255.0f;
static const color_t lcd_lit_color = RGB2COLOR(87, 92, 78);
static const color_t lcd_pixel_area_color = RGB2COLOR(82, 87, 74);
static const float lcd_alpha = 0.85f;
static const color_t lcd_black_color = RGB2COLOR(0, 0, 0);

static const GLfloat keyboard_vertex_data[] = {
    25, 0,
    210, 0,
    242, 7,
    334, 50,
    366, 57,
    593, 57,
    619, 83,
    619, 333,
    633, 356,
    772, 421,
    784, 454,
    678, 678,
    646, 690,
    325, 543,
    25, 543,
    0, 517,
    0, 25,
};

static const float keycap_size = 73.66f; // 7.25 * 2.54 * 4
static const float keycap_inner_size = 50.8f;

static const keyinfo_t keys[] = {
     {{71.12f, 160.85f}, 1.5f, 0.0f},
     {{166.37f, 160.85f}, 1.0f, 0.0f},
     {{242.57f, 148.15f}, 1.0f, 0.0f},
     {{318.77f, 141.8f}, 1.0f, 0.0f},
     {{394.97f, 148.16f}, 1.0f, 0.0f},
     {{471.17f, 153.24f}, 1.0f, 0.0f},
     {{547.37f, 153.24f}, 1.0f, 0.0f},

     {{71.12f, 237.05f}, 1.5f, 0.0f},
     {{166.37f, 237.05f}, 1.0f, 0.0f},
     {{242.57f, 224.35f}, 1.0f, 0.0f},
     {{318.77f, 218.0f}, 1.0f, 0.0f},
     {{394.97f, 224.36f}, 1.0f, 0.0f},
     {{471.17f, 229.44f}, 1.0f, 0.0f},
     {{547.37f, 248.49f}, 1.5f, 270.0f},

     {{71.12f, 313.25f}, 1.5f, 0.0f},
     {{166.37f, 313.25f}, 1.0f, 0.0f},
     {{242.57f, 300.55f}, 1.0f, 0.0f},
     {{318.77f, 294.2f}, 1.0f, 0.0f},
     {{394.97f, 300.56f}, 1.0f, 0.0f},
     {{471.17f, 305.44f}, 1.0f, 0.0f},
     {{0, 0}, 0, 0},

     {{71.12f, 389.45f}, 1.5f, 0.0f},
     {{166.37f, 389.45f}, 1.0f, 0.0f},
     {{242.57f, 376.65f}, 1.0f, 0.0f},
     {{318.77f, 370.4f}, 1.0f, 0.0f},
     {{394.97f, 376.76f}, 1.0f, 0.0f},
     {{471.17f, 381.64f}, 1.0f, 0.0f},
     {{547.37f, 362.79f}, 1.5f, 270.0f},

     {{90.17f, 465.65f}, 1.0f, 0.0f},
     {{166.37f, 465.65f}, 1.0f, 0.0f},
     {{242.57f, 452.85f}, 1.0f, 0.0f},
     {{318.77f, 446.6f}, 1.0f, 0.0f},
     {{394.97f, 452.96f}, 1.0f, 0.0f},
     {{629.09f, 433.73f}, 1.0f, 335.0f},
     {{698.15f, 465.94f}, 1.0f, 335.0f},

     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{665.94f, 535.0f}, 1.0f, 335.0f},

     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{0, 0}, 0, 0},
     {{512.72f, 505.11f}, 2.0f, 245.0f},
     {{580.78f, 537.32f}, 2.0f, 245.0f},
     {{633.73f, 604.06f}, 1.0f, 335.0f},
};

static const point lcd_pos = {32, 19};
static const point lcd_size = {173, 102};
static const int lcd_transparent_border_width = 4;
static const int lcd_lit_area_height = 67;
static const int lcd_black_border_width = 2;
static const int lcd_black_border_to_black_box_dist = 1;
static const point lcd_pixel_area_size = {128, 32};

static const point led_size = {7, 7};

static const GLfloat lcd_uv_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
};

static const int led_radius = 256;

#endif /* EMULATOR_KEYBOARD_DATA_H_ */
