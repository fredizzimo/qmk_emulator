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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include <glad/glad.h>
#pragma GCC diagnostic warning "-Wstrict-prototypes"

#include "shader.h"

static void print_compile_status(const char* file, GLuint id) {
    GLint result = GL_FALSE;
    int info_log_length;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
    if ( info_log_length > 0 ){
        char* error_message = malloc(info_log_length + 1);
        glGetShaderInfoLog(id, info_log_length, NULL, error_message);
        printf("%s %s\n", file, error_message);
        free(error_message);
    }
}

static void print_link_status(GLuint id) {
    GLint result = GL_FALSE;
    int info_log_length;
    glGetProgramiv(id, GL_LINK_STATUS, &result);
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
    if ( info_log_length > 0 ){
        char* error_message = malloc(info_log_length + 1);
        glGetShaderInfoLog(id, info_log_length, NULL, error_message);
        printf("%s\n", error_message);
        free(error_message);
    }
}

static GLuint load_shader(const char* path) {
    FILE* f = fopen( path, "rb");
    fseek( f, 0, SEEK_END );
    int filesize = ftell( f );
    rewind( f );
    char* buffer = (char*)malloc(filesize+1);
    fread(buffer, 1, filesize, f);
    buffer[filesize] = '\0';
    fclose(f);

    GLenum type = 0;
    if (strstr(path, "vertexshader")) {
       type = GL_VERTEX_SHADER;
    }
    else if(strstr(path, "fragmentshader")) {
        type = GL_FRAGMENT_SHADER;
    }
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, (const char**)&buffer, NULL);
    glCompileShader(id);

    print_compile_status(path, id);

    free(buffer);
    // Check Vertex Shader

    return id;
}

GLuint load_program(const char* vertex_shader, const char* fragment_shader) {
    GLuint vertex_shader_id = load_shader(vertex_shader);
    GLuint fragment_shader_id = load_shader(fragment_shader);
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    print_link_status(program_id);
    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}
