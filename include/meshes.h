#pragma once

#include <gl3/gl3platform.h>

#include "utils.h"
#include "wavefront.h"

#define MAX_MESHES 20

#define CUBE_VERTICES_NUM       24
#define CUBE_INDICES_NUM        36
#define QUAD_VERTICES_NUM       4
#define QUAD_INDICES_NUM        6

enum VERTEX_FORMAT
{
    VF_V3,
    VF_V3N3,
    VF_V3N3T2
};
enum INDEX_FORMAT
{
    IF_IU16,
    IF_IU32
};

typedef struct v3t2n3_t
{
    float3 position;
    float2 texcoord;
    float3 normal;
} v3t2n3_t;

typedef struct v3t2_t
{
    float x, y, z;
    float u, v;
} v3t2_t;

/*struct MESH
{
    GLuint      va;
    GLuint      vb;
    GLuint      ib;
    GLsizei     count;
};*/

extern const v3t2_t fullscreen_quad_vertices[6];

GLuint new_mesh(void *vertices, int vertices_num, void *indices, int indices_num);
GLuint new_cube();
GLuint new_sphere(int rings, int sectors);
GLuint new_quad();
GLuint new_cylinder(float radius, float height, int sides);

/*struct MESH* new_mesh2(const char *name, void *vertices, int vertices_num, void *indices, int indices_num);
struct MESH* get_mesh(const char *name);
void cleanup_meshes();*/
