#pragma once

#include <gl3/gl3platform.h>

struct FEEDBACK_VARYINGS
{
    const char  **varyings;
    int         count;
    GLenum      buffer_mode;
};

extern const char color_vertex_shader[];
extern const char color_fragment_shader[];
extern const char color_instanced_vertex_shader[];
extern const char color_instanced_fragment_shader[];
extern const char texturing_fragment_shader[];
extern const char postprocess_vertex_shader[];
extern const char postprocess_fragment_shader[];
extern const char hblur_fragment_shader[];
extern const char vblur_fragment_shader[];
extern const char ui_fragment_shader[];
extern const char particles_draw_vertex_shader[];
extern const char particles_draw_fragment_shader[];
extern const char particles_update_vertex_shader[];
extern const char particles_udpate_fragment_shader[];

GLuint new_shader(const char *_vs_text, const char *_fs_text, struct FEEDBACK_VARYINGS *_varyings);
