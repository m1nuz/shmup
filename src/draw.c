#include "draw.h"

extern int      screen_width;
extern int      screen_height;

extern GLuint   cube_mesh;
extern GLuint   sphere_mesh;
extern GLuint   quad_mesh;
extern GLuint   cylinder_mesh;
extern GLuint   fullscreen_va;
extern GLuint   sphere_mesh_instanced;

extern GLuint   ui_va;
extern GLuint   ui_vb;

extern GLuint   color_map;
extern GLuint   depth_map;
extern GLuint   blur_map;
extern GLuint   glow_map;
extern GLuint   ui_map;

void
draw_mesh(GLuint mesh, GLuint shader, const float3 color, const float *transform, GLsizei count)
{
    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], 1);

    glBindVertexArray(mesh);

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
}

void
draw_cube(GLuint shader, const float3 color, const float *transform)
{
    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], 1);

    glBindVertexArray(cube_mesh);

    glDrawElements(GL_TRIANGLES, /*CUBE_INDICES_NUM*/144, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);
}

void
draw_sphere(GLuint shader, const float3 color, const float *transform)
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], 1);

    glBindVertexArray(sphere_mesh);

    glDrawElements(GL_TRIANGLES, 16 * 16 * 5 + 128, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);
}

void
draw_transparent_sphere(GLuint shader, const float4 color, const float *transform)
{
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    //glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], color[3]);

    glBindVertexArray(sphere_mesh);

    glDrawElements(GL_TRIANGLES, 16 * 16 * 5 + 128, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}
void
draw_instanced_sphere(GLuint shader, int instance_count)
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glBindVertexArray(sphere_mesh_instanced);

    glDrawElementsInstanced(GL_TRIANGLES, 8 * 8 * 5 + 16, GL_UNSIGNED_SHORT, 0, instance_count);

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);
}

void
draw_quad(GLuint shader, const float3 color, const float *transform)
{
    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], 1);

    glBindVertexArray(quad_mesh);

    glDrawElements(GL_TRIANGLES, QUAD_INDICES_NUM, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
}

void
draw_textured_quad(GLuint shader, GLuint texture0, GLuint texture1, GLuint sampler, const float4 color, const float *transform)
{
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ONE);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");
    int loc_tex0 = glGetUniformLocation(shader, "color0_map");
    int loc_tex1 = glGetUniformLocation(shader, "color1_map");

    glUniform1i(loc_tex0, 0);
    glUniform1i(loc_tex1, 1);

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], color[4]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glBindSampler(0, sampler);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glBindSampler(1, sampler);

    glBindVertexArray(quad_mesh);

    glDrawElements(GL_TRIANGLES, QUAD_INDICES_NUM, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void
draw_cylinder(GLuint shader, const float3 color, const float *transform)
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform4f(loc_c, color[0], color[1], color[2], 1);

    glBindVertexArray(cylinder_mesh);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);
}

void
draw_fullscreen_quad(GLuint shader, GLuint sampler)
{
    int loc_cmp = glGetUniformLocation(shader, "color_map");
    int loc_gmp = glGetUniformLocation(shader, "glow_map");
    int loc_ump = glGetUniformLocation(shader, "ui_map");

    glUniform1i(loc_cmp, 0);
    glUniform1i(loc_gmp, 1);
    glUniform1i(loc_ump, 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_map);
    glBindSampler(0, sampler);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, glow_map);
    glBindSampler(1, sampler);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ui_map);
    glBindSampler(2, sampler);

    glBindVertexArray(fullscreen_va);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
draw_particles(GLuint shader, float3 color, float dv, const float *transform, struct PARTICLES_BUFFER *buffer)
{
    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_ps = glGetUniformLocation(shader, "point_size");
    int loc_c = glGetUniformLocation(shader, "color");
    int loc_d = glGetUniformLocation(shader, "dv");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);
    glUniform1f(loc_ps, buffer->point_size);
    glUniform1f(loc_d, dv);
    glUniform4f(loc_c, color[0], color[1], color[2], 1);

    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBindVertexArray(buffer->va);

    glDrawArrays(GL_POINTS, 0, buffer->count);

    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void
update_particles(GLuint shader, float tv, struct PARTICLES_BUFFER *buffer)
{
    glEnable(GL_RASTERIZER_DISCARD);

    int loc_tv = glGetUniformLocation(shader, "tv");
    int loc_gv = glGetUniformLocation(shader, "gravity");

    glUniform1f(loc_tv, tv);
    glUniform3f(loc_gv, 0.f, 0.f, 0.f);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer->fb);

    glBeginTransformFeedback(GL_POINTS);
    glBindVertexArray(buffer->fa);

    glDrawArrays(GL_POINTS, 0, buffer->count);

    glBindVertexArray(0);
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);
}

void
draw_ui(GLuint shader, GLuint texture, GLuint sampler, float3 color, float2 position, float2 size, int slot, int nslot)
{
    float aspect = (float)screen_width / screen_height;

    v3t2_t vertices[6];

    vertices[0].x = position[0] - size[0],  vertices[0].y = position[1] + size[1] * aspect, vertices[0].z = 0;
    vertices[1].x = position[0] + size[0],  vertices[1].y = position[1] + size[1] * aspect, vertices[1].z = 0;
    vertices[2].x = position[0] - size[0],  vertices[2].y = position[1] - size[1] * aspect, vertices[2].z = 0;
    vertices[3].x = position[0] + size[0],  vertices[3].y = position[1] + size[1] * aspect, vertices[3].z = 0;
    vertices[4].x = position[0] + size[0],  vertices[4].y = position[1] - size[1] * aspect, vertices[4].z = 0;
    vertices[5].x = position[0] - size[0],  vertices[5].y = position[1] - size[1] * aspect, vertices[5].z = 0;

    vertices[0].u = (float)slot * (1.f / nslot),        vertices[0].v = 0;
    vertices[1].u = (float)(slot + 1) * (1.f / nslot),  vertices[1].v = 0;
    vertices[2].u = (float)slot * (1.f / nslot),        vertices[2].v = 1;
    vertices[3].u = (float)(slot + 1) * (1.f / nslot),  vertices[3].v = 0;
    vertices[4].u = (float)(slot + 1) * (1.f / nslot),  vertices[4].v = 1;
    vertices[5].u = (float)slot * (1.f / nslot),        vertices[5].v = 1;


    glBindBuffer(GL_ARRAY_BUFFER, ui_vb);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int loc_tex = glGetUniformLocation(shader, "color_map");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniform1i(loc_tex, 0);
    glUniform4f(loc_c, color[0], color[1], color[2], 1.f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    glBindVertexArray(ui_va);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
