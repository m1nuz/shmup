#pragma once

#include <gl3/gl3platform.h>
#include "targa.h"

GLuint new_texture2D(GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLuint new_sampler2D(GLint mag_filter, GLint min_filter, GLint wrap);
GLuint get_texture(const char *filename);
