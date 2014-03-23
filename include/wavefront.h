#pragma once

#include <gl3/gl3platform.h>

void* load_wavefront(const char *_filepath, float **_vertices, int *_vertices_num, unsigned short **_indices, int *_indices_num);
