#pragma once

#include "meshes.h"

struct PARTICLES_BUFFER
{
    GLuint      va;
    GLuint      vb;
    GLuint      fa;
    GLuint      fb;
    GLsizei     count;
    float       point_size;
};

void draw_mesh(GLuint mesh, GLuint shader, const float3 color, const float *transform, GLsizei count);
void draw_cube(GLuint shader, const float3 color, const float *transform);
void draw_sphere(GLuint shader, const float3 color, const float *transform);
void draw_transparent_sphere(GLuint shader, const float4 color, const float *transform);
void draw_instanced_sphere(GLuint shader, int instance_count);
void draw_quad(GLuint shader, const float3 color, const float *transform);
void draw_textured_quad(GLuint shader, GLuint texture0, GLuint texture1, GLuint sampler, const float4 color, const float *transform);
void draw_cylinder(GLuint shader, const float3 color, const float *transform);
void draw_fullscreen_quad(GLuint shader, GLuint sampler);
void draw_particles(GLuint shader, float3 color, float dv, const float *transform, struct PARTICLES_BUFFER *buffer);
void update_particles(GLuint shader, float tv, struct PARTICLES_BUFFER *buffer);
void draw_ui(GLuint shader, GLuint texture, GLuint sampler, float3 color, float2 position, float2 size, int slot, int nslot);
