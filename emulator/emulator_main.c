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

#include "gfx.h"
#include "config.h"
#include "visualizer.h"
#include <math.h>
#include <stdio.h>
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include <glad/glad.h>
#pragma GCC diagnostic warning "-Wstrict-prototypes"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "keyboard_data.h"
#include "shader.h"
#include "emulator_driver.h"

//#define DISPLAY_FPS

static GDisplay* lcd;
static GDisplay* led;
static GDisplay* debug_display;
static font_t font;
static GLFWwindow* window;

typedef struct {
    GLuint program_id;
    GLuint view_projection_location;
    GLuint keyboard_position_location;
    GLuint element_color_location;
    GLuint texture_sampler_location;
    GLuint intensity_location;
    GLuint pos_location;
}program_t;

typedef struct {
    GLuint program_id;
    GLuint vcoord_location;
    GLuint fbo_texture_location;
}postprocess_t;

static program_t keyboard_program;
static program_t lcd_program;
static program_t led_program;
static program_t debug_program;
static program_t* current_program;

static postprocess_t postprocess_program;

static GLuint keyboard_vertex_buffer;
static GLuint key_inner_vertex_buffer;
static GLuint key_inner_vertex_buffer_size;
static GLuint key_outer_vertex_buffer;
static GLuint key_outer_vertex_buffer_size;
static GLuint lcd_vertex_buffer;
static GLuint lcd_uv_buffer;
static GLuint led_vertex_buffer;
static GLuint led_vertex_buffer_size;
static GLuint debug_vertex_buffer;

static GLuint lcd_texture;
static GLuint debug_texture;

static GLuint fbo_texture;
static GLuint fbo;
static GLuint fbo_vertices;

static color_t lcd_base_color;



static const int num_keys = sizeof(keys) / sizeof(keyinfo_t);
void error_callback(int error, const char* description)
{
    (void)error;
    fputs(description, stderr);
}

GDisplay* get_lcd_display(void) {
    return lcd;
}

GDisplay* get_led_display(void) {
    return led;
}

bool is_serial_link_master(void) {
    return true;
}

uint8_t get_mods(void) {
    return 0;
}

bool has_oneshot_mods_timed_out(void) {
    return false;
}

uint8_t get_oneshot_mods(void) {
    return 0;
}

static void create_keyboard_vertex_buffer(void) {
    glGenBuffers(1, &keyboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, keyboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(keyboard_vertex_data), keyboard_vertex_data, GL_STATIC_DRAW);
}

static GLfloat* create_quad(GLfloat* out, point* points) {
    *out++ = points[0].x;
    *out++ = points[0].y;
    *out++ = points[1].x;
    *out++ = points[1].y;
    *out++ = points[3].x;
    *out++ = points[3].y;
    *out++ = points[1].x;
    *out++ = points[1].y;
    *out++ = points[2].x;
    *out++ = points[2].y;
    *out++ = points[3].x;
    *out++ = points[3].y;
    return out;
}

static GLfloat* create_quad_from_box(GLfloat* out, GLfloat left, GLfloat top, GLfloat width, GLfloat height) {
    point p[] = {
            {left, top},
            {left + width, top},
            {left + width, top + height},
            {left, top + height}
    };
    return create_quad(out, p);
}

static void create_key_vertex_buffers(void) {
    GLfloat outer_vertex_data[2 * num_keys * 6];
    GLfloat* outer_vertex = outer_vertex_data;
    GLfloat inner_vertex_data[2 * num_keys * 6];
    GLfloat* inner_vertex = inner_vertex_data;
    const float keycap_border = keycap_size - keycap_inner_size;

    for (int i=0;i<num_keys;i++) {
        if (keys[i].size == 0)
            continue;
        MatrixFloat2D mat;
        gmiscMatrixFloat2DApplyRotation(&mat, NULL, keys[i].rot);
        gmiscMatrixFloat2DApplyTranslation(&mat, &mat, keys[i].pos.x, keys[i].pos.y);

        float width = keycap_size  * keys[i].size;
        float height = keycap_size;
        float half_width = width / 2.0f;
        float half_height = height / 2.0f;
        point points[] = {
            {-half_width, -half_height},
            {half_width, -half_height},
            {half_width, half_height},
            {-half_width, half_height},
        };
        gmiscMatrixFloat2DApplyToPoints(points, points, &mat, 4);
        outer_vertex = create_quad(outer_vertex, points);

        width -= keycap_border;
        height -= keycap_border;
        half_width = width / 2.0f;
        half_height = height / 2.0f;
        point points2[] = {
            {-half_width, -half_height},
            {half_width, -half_height},
            {half_width, half_height},
            {-half_width, half_height}
        };
        gmiscMatrixFloat2DApplyToPoints(points2, points2, &mat, 4);
        inner_vertex = create_quad(inner_vertex, points2);
    }
    key_outer_vertex_buffer_size = outer_vertex - outer_vertex_data;
    glGenBuffers(1, &key_outer_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, key_outer_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, key_outer_vertex_buffer_size * sizeof(GLfloat), outer_vertex_data, GL_STATIC_DRAW);

    key_inner_vertex_buffer_size = inner_vertex - inner_vertex_data;
    glGenBuffers(1, &key_inner_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, key_inner_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, key_inner_vertex_buffer_size * sizeof(GLfloat), inner_vertex_data, GL_STATIC_DRAW);
}

static void create_lcd_vertex_buffer(void) {
    GLfloat vertex_data[2 * 6 * 6];
    GLfloat* vertex = vertex_data;

    // The whole transparent area
    vertex = create_quad_from_box(vertex, lcd_pos.x, lcd_pos.y, lcd_size.x, lcd_size.y);
    // The LCD lit area
    vertex = create_quad_from_box(vertex,
        lcd_pos.x + lcd_transparent_border_width,
        lcd_pos.y + lcd_transparent_border_width,
        lcd_size.x - 2 * lcd_transparent_border_width,
        lcd_lit_area_height
    );
    // The black box at the bottom
    vertex = create_quad_from_box(vertex,
        lcd_pos.x + lcd_transparent_border_width,
        lcd_pos.y + lcd_transparent_border_width + lcd_lit_area_height,
        lcd_size.x - 2 * lcd_transparent_border_width,
        lcd_size.y - 2 * lcd_transparent_border_width - lcd_lit_area_height
    );
    // The black border
    vertex = create_quad_from_box(vertex,
        lcd_pos.x + lcd_transparent_border_width,
        lcd_pos.y + lcd_transparent_border_width,
        lcd_size.x - 2 * lcd_transparent_border_width,
        lcd_lit_area_height - lcd_black_border_to_black_box_dist
    );
    // The LCD lit area again, but inside the black border only
    int inner_lit_area_height = lcd_lit_area_height - 2 * lcd_black_border_width - lcd_black_border_to_black_box_dist;
    vertex = create_quad_from_box(vertex,
        lcd_pos.x + lcd_transparent_border_width + lcd_black_border_width,
        lcd_pos.y + lcd_transparent_border_width + lcd_black_border_width,
        lcd_size.x - 2 * lcd_transparent_border_width - 2 * lcd_black_border_width,
        inner_lit_area_height
    );
    // The main LCD pixel area
    vertex = create_quad_from_box(vertex,
        lcd_pos.x + lcd_size.x / 2 - lcd_pixel_area_size.x / 2,
        lcd_pos.y + lcd_transparent_border_width + lcd_black_border_width +
            inner_lit_area_height / 2  - lcd_pixel_area_size.y / 2,
        lcd_pixel_area_size.x,
        lcd_pixel_area_size.y
    );

    glGenBuffers(1, &lcd_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, lcd_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glGenBuffers(1, &lcd_uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, lcd_uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lcd_uv_data), lcd_uv_data, GL_STATIC_DRAW);
}

static void create_lcd_texture(void) {
    gdispGClear(lcd, White);
    glGenTextures(1, &lcd_texture);
    glBindTexture(GL_TEXTURE_2D, lcd_texture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 128, 32, 0, GL_BGRA, GL_UNSIGNED_BYTE, getEmulatorPixmap(lcd));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void create_led_vertex_buffer(void) {
    GLfloat vertex_data[num_keys * 6 * 2];
    GLfloat* vertex = vertex_data;
    for (int i=0;i<num_keys;i++) {
        if (keys[i].size == 0)
            continue;

        MatrixFloat2D rot;
        gmiscMatrixFloat2DApplyRotation(&rot, NULL, keys[i].rot);
        int mid_x = keys[i].pos.x;
        int mid_y = keys[i].pos.y;
        point points[] = {
            {0, -20}
        };
        gmiscMatrixFloat2DApplyToPoints(points, points, &rot, 1);
        int led_x = points[0].x + mid_x;
        int led_y = points[0].y + mid_y;

        point pos = {
            led_x - led_radius,
            led_y - led_radius
        };

        vertex = create_quad_from_box(vertex, pos.x, pos.y, led_radius * 2, led_radius * 2);
    }
    led_vertex_buffer_size = vertex - vertex_data;
    glGenBuffers(1, &led_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, led_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, led_vertex_buffer_size * sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);
}

static void create_debug_texture(void) {
    gdispGClear(debug_display, background_color);
    glGenTextures(1, &debug_texture);
    glBindTexture(GL_TEXTURE_2D, debug_texture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, screen_dimensions.x, screen_dimensions.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, getEmulatorPixmap(debug_display));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void create_debug_vertex_buffer(void) {
    GLfloat vertex_data[6 * 2];
    GLfloat* vertex = vertex_data;
    vertex = create_quad_from_box(vertex, 0, 0, screen_dimensions.x, screen_dimensions.y);
    glGenBuffers(1, &debug_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, debug_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
}

static void create_frame_buffer(void) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &fbo_texture);
    glBindTexture(GL_TEXTURE_2D, fbo_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, screen_dimensions.x, screen_dimensions.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_EXT_framebuffer_object, fbo);
    glFramebufferTexture2DEXT(GL_EXT_framebuffer_object, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo_texture, 0);
    glBindFramebufferEXT(GL_EXT_framebuffer_object, 0);

    GLfloat fbo_vertices_data[] = {
      -1, -1,
       1, -1,
      -1,  1,
       1,  1,
    };
    glGenBuffers(1, &fbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, fbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices_data), fbo_vertices_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void load_shaders(void) {
    keyboard_program.program_id = load_program("keyboard.vertexshader", "keyboard.fragmentshader");
    lcd_program.program_id = load_program("lcd.vertexshader", "lcd.fragmentshader");
    led_program.program_id = load_program("led.vertexshader", "led.fragmentshader");
    debug_program.program_id = load_program("debug.vertexshader", "debug.fragmentshader");
    postprocess_program.program_id = load_program("postprocess.vertexshader", "postprocess.fragmentshader");

    GLuint program_id = keyboard_program.program_id;
    keyboard_program.view_projection_location = glGetUniformLocation(program_id, "view_projection");
    keyboard_program.keyboard_position_location = glGetUniformLocation(program_id, "keyboard_location");
    keyboard_program.element_color_location = glGetUniformLocation(program_id, "element_color");

    program_id = lcd_program.program_id;
    lcd_program.view_projection_location = glGetUniformLocation(program_id, "view_projection");
    lcd_program.keyboard_position_location = glGetUniformLocation(program_id, "keyboard_location");
    lcd_program.texture_sampler_location = glGetUniformLocation(program_id, "texture_sampler");
    lcd_program.element_color_location = glGetUniformLocation(program_id, "element_color");

    program_id = led_program.program_id;
    led_program.view_projection_location = glGetUniformLocation(program_id, "view_projection");
    led_program.keyboard_position_location = glGetUniformLocation(program_id, "keyboard_location");
    led_program.element_color_location = -1;
    led_program.intensity_location = glGetUniformLocation(program_id, "intensity");

    program_id = debug_program.program_id;
    debug_program.view_projection_location = glGetUniformLocation(program_id, "view_projection");
    debug_program.keyboard_position_location = -1;
    debug_program.texture_sampler_location = glGetUniformLocation(program_id, "texture_sampler");
    debug_program.element_color_location = glGetUniformLocation(program_id, "element_color");

    program_id = postprocess_program.program_id;
    postprocess_program.vcoord_location = glGetAttribLocation(program_id, "v_coord");
    postprocess_program.fbo_texture_location = glGetUniformLocation(program_id, "fbo_texture");

}

int main(void) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_SRGB_CAPABLE, TRUE);
    window = glfwCreateWindow(screen_dimensions.x, screen_dimensions.y,
            "Ergodox Emulator", NULL, NULL);
    if(!window) {
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    if(!gladLoadGL()) {
        exit(EXIT_FAILURE);
    };
    printf("OpenGL Version %d.%d loaded\n", GLVersion.major, GLVersion.minor);
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    load_shaders();

    create_keyboard_vertex_buffer();
    create_key_vertex_buffers();
    create_lcd_vertex_buffer();
    create_led_vertex_buffer();
    create_debug_vertex_buffer();

    create_frame_buffer();

    gfxInit();
    lcd = gdispGetDisplay(0);
    led = gdispGetDisplay(1);
    debug_display = gdispPixmapCreate(screen_dimensions.x, screen_dimensions.y);

    create_lcd_texture();
    create_debug_texture();
    glfwMakeContextCurrent(NULL);

    font = gdispOpenFont("DejaVuSansBold12");

    gdispSetDisplay(lcd);

    // Initialize and clear the display
    visualizer_init();

    uint8_t default_layer_state = 0;
    uint8_t layer_state = 0;
    uint8_t mods = 0;
    uint8_t leds = 0;
    ergodox_right_led_1_on();

    while (!glfwWindowShouldClose(window)) {
        visualizer_update(default_layer_state, layer_state, mods, leds);
        gfxSleepMilliseconds(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

void lcd_backlight_hal_init(void) {
}

void lcd_backlight_hal_color(uint16_t r, uint16_t g, uint16_t b) {
    lcd_base_color = RGB2COLOR(
            r * 255 / 65536,
            g * 255 / 65536,
            b * 255 / 65536);
}

static void setup_view_projection(void) {
    const GLfloat left = 0;
    const GLfloat right = screen_dimensions.x;
    const GLfloat top = 0;
    const GLfloat bottom = screen_dimensions.y;
    const GLfloat far_val = 10.0f;
    const GLfloat near_val = -10.0f;
    const GLfloat a = 2.0f / (right-left);
    const GLfloat b = -(right + left) / (right - left);
    const GLfloat c = 2.0f / (top - bottom);
    const GLfloat d = -(top + bottom) / (top - bottom);
    const GLfloat e = -2.0f / (far_val - near_val);
    const GLfloat f = -(far_val + near_val) / (far_val - near_val);

    const GLfloat view_projection[4*4] = {
        a,    0.0f, 0.0f, 0.0f,
        0.0f, c,    0.0f, 0.0f,
        0.0f, 0.0f, e,    0.0f,
        b,    d,    f,    1.0f
    };
    glUniformMatrix4fv(current_program->view_projection_location, 1, GL_FALSE, view_projection);
}

static void setup_keyboard_location(int keyboard_x, int keyboard_y) {
    const GLfloat matrix[4*4] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        keyboard_x, keyboard_y, 0.0f, 1.0f
    };
    glUniformMatrix4fv(current_program->keyboard_position_location, 1, GL_FALSE, matrix);
}

static void use_program(program_t* program) {
    glUseProgram(program->program_id);
    current_program = program;
    setup_view_projection();
    setup_keyboard_location(keyboard_pos.x, keyboard_pos.y);
    glDisable(GL_DEPTH_TEST);
}

static void draw_main_keyboard_area(void) {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, keyboard_vertex_buffer);
    glVertexAttribPointer(
       0,                  // attribute
       2,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );
    float r = RED_OF(main_keyboard_area_color) / 255.0f;
    float g = GREEN_OF(main_keyboard_area_color) / 255.0f;
    float b = BLUE_OF(main_keyboard_area_color) / 255.0f;
    glUniform3f(current_program->element_color_location, r, g, b);
    glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(keyboard_vertex_data) / sizeof(GLfloat) / 2);
    glDisableVertexAttribArray(0);
}

static void draw_triangles_with_offset(GLuint vertex_buffer, GLuint vertex_buffer_size, GLuint offset, color_t color) {
    offset = offset * sizeof(GLfloat) * 2;
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(
       0,                  // attribute
       2,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)(intptr_t)offset           // array buffer offset
    );
    float r = RED_OF(color) / 255.0f;
    float g = GREEN_OF(color) / 255.0f;
    float b = BLUE_OF(color) / 255.0f;
    glUniform3f(current_program->element_color_location, r, g, b);
    glDrawArrays(GL_TRIANGLES, 0, vertex_buffer_size);
    glDisableVertexAttribArray(0);
}

static void draw_lcd_texture(GLuint offset, color_t color) {
    use_program(&lcd_program);

    offset = offset * sizeof(GLfloat) * 2;
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, lcd_vertex_buffer);
    glVertexAttribPointer(
       0,                  // attribute
       2,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)(intptr_t)offset           // array buffer offset
    );
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, lcd_uv_buffer);
    glVertexAttribPointer(
        1,                                // attribute.
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lcd_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, lcd_pixel_area_size.x, lcd_pixel_area_size.y, GL_BGRA, GL_UNSIGNED_BYTE, getEmulatorPixmap(lcd));
    glUniform1i(current_program->texture_sampler_location, 0);
    float r = RED_OF(color) / 255.0f;
    float g = GREEN_OF(color) / 255.0f;
    float b = BLUE_OF(color) / 255.0f;
    glUniform3f(current_program->element_color_location, r, g, b);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void draw_triangles(GLuint vertex_buffer, GLuint vertex_buffer_size, color_t color) {
    draw_triangles_with_offset(vertex_buffer, vertex_buffer_size, 0, color);
}

static void draw_keycaps(void) {
    draw_triangles(key_outer_vertex_buffer, key_outer_vertex_buffer_size, keycap_outer_color);
    draw_triangles(key_inner_vertex_buffer, key_inner_vertex_buffer_size, keycap_inner_color);
}

static color_t clip_color(float r, float g, float b) {
    float m = r > g ? r : g;
    m = m > b ? m : b;
    if (m > 255.0f) {
        float d = 255.0f / m;
        r*=d;
        g*=d;
        b*=d;
    }
    return RGB2COLOR((int)r, (int)g, (int)b);
}

static color_t multiply_color(color_t color, float multiplier) {
    float r = RED_OF(color) * multiplier;
    float g = GREEN_OF(color) * multiplier;
    float b = BLUE_OF(color) * multiplier;
    return clip_color(r, g, b);
}

static color_t add_color(color_t color1, color_t color2) {
    float r = RED_OF(color1) + RED_OF(color2);
    float g = GREEN_OF(color1) + GREEN_OF(color2);
    float b = BLUE_OF(color1) + BLUE_OF(color2);
    return clip_color(r, g, b);
}

static void convert_rgb_to_hsi(float r, float g, float b, float* h, float* s, float* i) {
    float min = fminf(fminf(r, g), b);
    float max = fmaxf(fmaxf(r, g), b);
    float diff = max - min;
    *i = r + g + b;
    if(diff == 0) {
        *h = 0.0;
        *s = 0.0;
    }
    else {
        if(max==r) {
            *h = fmodf(((g - b) / diff), 6.0f);
        }
        else if(max == g) {
            *h = (b - r) / diff + 2.0f;
        }
        else if(max==b) {
            *h = (r - g) / diff + 4.0f;
        }
        *h *= 60.0f;
        *s = 1.0f - (min / *i);
    }
}

static void convert_hsv_to_rgb(float h, float s, float v, float* r, float* g, float* b) {
    double      hh, p, q, t, ff;
    long        i;

    hh = h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch(i) {
    case 0:
        *r = v;
        *g = t;
        *b = p;
        break;
    case 1:
        *r = q;
        *g = v;
        *b = p;
        break;
    case 2:
        *r = p;
        *g = v;
        *b = t;
        break;

    case 3:
        *r = p;
        *g = q;
        *b = v;
        break;
    case 4:
        *r = t;
        *g = p;
        *b = v;
        break;
    case 5:
    default:
        *r = v;
        *g = p;
        *b = q;
        break;
    }
}

static void draw_lcd(void) {
    // The whole transparent area
    float r = RED_OF(lcd_base_color) / 255.0f;
    float g = GREEN_OF(lcd_base_color) / 255.0f;
    float b = BLUE_OF(lcd_base_color) / 255.0f;
    float h, s, i;
    convert_rgb_to_hsi(r, g, b, &h, &s, &i);

    // Scale with the current brightness, so we don't emulate brightness control here
    float lcd_brightness = lcd_get_backlight_brightness() / 255.0f;
    i /= lcd_brightness;

    // Let the intensity vary between the base intensity and 1.0
    i *= (1.0f - lcd_base_intensity);
    i = lcd_base_intensity + i;
    i = fminf(i, 1.0f);

    convert_hsv_to_rgb(h, s, i, &r, &g, &b);
    color_t color = RGB2COLOR((uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
    color_t color_with_alpha = multiply_color(color, 1.0f - lcd_alpha);

    draw_triangles(lcd_vertex_buffer, 6, color);
    // The LCD lit area
    draw_triangles_with_offset(lcd_vertex_buffer, 6, 6,
            add_color(color_with_alpha, lcd_lit_color));
    // The black box at the bottom
    draw_triangles_with_offset(lcd_vertex_buffer, 6, 12, lcd_black_color);
    // The black border
    draw_triangles_with_offset(lcd_vertex_buffer, 6, 18, lcd_black_color);
    // The LCD lit area again, but inside the black border only
    draw_triangles_with_offset(lcd_vertex_buffer, 6, 24,
            add_color(color_with_alpha, lcd_lit_color));
    draw_lcd_texture(30,
            add_color(color_with_alpha, lcd_pixel_area_color));
}

static void draw_leds(void) {
    use_program(&led_program);
    glEnable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, lcd_uv_buffer);
    glVertexAttribPointer(
        1,                                // attribute.
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    unsigned buffer_pos = 0;

    pixel_t* pixels = getEmulatorPixmap(led);

    for (int i=0;i<num_keys;i++) {
        if (keys[i].size == 0)
            continue;
        int luma = LUMA_OF(pixels[i]);
        glUniform1f(current_program->intensity_location, luma / 255.0f);
        draw_triangles_with_offset(led_vertex_buffer, 6, buffer_pos, RGB2COLOR(0, 0, 255));
        buffer_pos += 6;
    }
    glDisableVertexAttribArray(1);
    glDisable(GL_BLEND);
}

void draw_debug(void) {
    use_program(&debug_program);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, debug_vertex_buffer);
    glVertexAttribPointer(
       0,                  // attribute
       2,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)(intptr_t)0           // array buffer offset
    );
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, lcd_uv_buffer);
    glVertexAttribPointer(
        1,                                // attribute.
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, debug_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screen_dimensions.x, screen_dimensions.y, GL_BGRA, GL_UNSIGNED_BYTE, gdispPixmapGetBits(debug_display));
    glUniform1i(current_program->texture_sampler_location, 0);
    float r = RED_OF(background_color) / 255.0f;
    float g = GREEN_OF(background_color) / 255.0f;
    float b = BLUE_OF(background_color) / 255.0f;
    glUniform3f(current_program->element_color_location, r, g, b);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void draw_post_process(void) {
    glBindFramebufferEXT(GL_EXT_framebuffer_object, 0);
    glEnable(GL_EXT_framebuffer_sRGB);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glUseProgram(postprocess_program.program_id);
    glBindTexture(GL_TEXTURE_2D, fbo_texture);
    glUniform1i(postprocess_program.fbo_texture_location, /*GL_TEXTURE*/0);
    glEnableVertexAttribArray(postprocess_program.vcoord_location);

    glBindBuffer(GL_ARRAY_BUFFER, fbo_vertices);
    glVertexAttribPointer(
      postprocess_program.vcoord_location,  // attribute
      2,                  // number of elements per vertex, here (x,y)
      GL_FLOAT,           // the type of each element
      GL_FALSE,           // take our values as-is
      0,                  // no extra data between each position
      0                   // offset of first element
    );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(postprocess_program.vcoord_location);
}

void draw_emulator(void) {
    static double last = 0.0;
    double start_draw = glfwGetTime();

    if (!glfwGetCurrentContext()) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
    }
    glBindFramebufferEXT(GL_EXT_framebuffer_object, fbo);
    glDisable(GL_EXT_framebuffer_sRGB);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(
       RED_OF(background_color) / 255.0f,
       GREEN_OF(background_color) / 255.0f,
       BLUE_OF(background_color) / 255.0f,
       1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    use_program(&keyboard_program);

    draw_main_keyboard_area();
    double after_draw_keyboard = glfwGetTime();

    draw_keycaps();
    double after_draw_keycaps = glfwGetTime();

    draw_lcd();
    double after_draw_lcd = glfwGetTime();

    draw_leds();
    double after_draw_leds = glfwGetTime();

    double total_time = after_draw_leds - start_draw;
#ifdef DISPLAY_FPS
    gdispSetDisplay(debug_display);
    gdispClear(background_color);
    char buffer[256];
    sprintf(buffer, "Frame time %lf draw_only: %lf ", start_draw - last, total_time);
    gdispDrawString(10, 700, buffer, font, Black);
    sprintf(buffer, "Keyboard %lf, keycaps %lf, leds %lf, lcd %lf ",
            after_draw_keyboard - start_draw,
            after_draw_keycaps - after_draw_keyboard,
            after_draw_lcd - after_draw_keycaps,
            after_draw_leds - after_draw_lcd
            );
    gdispDrawString(10, 715, buffer, font, Black);
    gdispFlush();
    gdispSetDisplay(gdispGetDisplay(0));
    draw_debug();
#else
    (void) start_draw;
    (void) last;
    (void) total_time;
    (void) after_draw_keyboard;
    (void) after_draw_keycaps;
    (void) after_draw_lcd;
    (void) after_draw_leds;
#endif
    gdispSetDisplay(lcd);
    draw_post_process();
    glfwSwapBuffers(window);
    last = start_draw;
}
