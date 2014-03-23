#pragma once

#include <stdlib.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#define EMPTY_MATRIX4  { 0.0, 0.0, 0.0, 0.0,\
                         0.0, 0.0, 0.0, 0.0,\
                         0.0, 0.0, 0.0, 0.0,\
                         0.0, 0.0, 0.0, 0.0 }

#define IDENTITY_MATRIX4 { 1.0, 0.0, 0.0, 0.0,\
                           0.0, 1.0, 0.0, 0.0,\
                           0.0, 0.0, 1.0, 0.0,\
                           0.0, 0.0, 0.0, 1.0 }

typedef float   float2[2];
typedef float   float3[3];
typedef float   float4[4];
typedef float4  float4x4[4];
typedef float   float16[16];

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define set_float3(v, x, y, z) (v)[0] = (x), (v)[1] = (y), (v)[2] = (z)

#define randf() (rand() / (float)RAND_MAX)
#define randf2(min_value, max_value) (min_value + (max_value - min_value) * randf())

#define length3(v) (sqrtf((v)[0] * (v)[0] + (v)[1] * (v)[1] + (v)[2] * (v)[2]))
#define normalize3(v) do {                           \
    float invlen = 1.f / length3(v);                \
    v[0] *= invlen, v[1] *= invlen, v[2] *= invlen; \
    } while(0)

#define mul3(a, b, c) c[0] = (a)[0] * (b)[0], c[1] = (a)[1] * (b)[1], c[2] = (a)[2] * (b)[2]
#define mix3(a, b, c, f) (c)[0] = (a)[0] * f + (b)[0] * (1 - f), \
    (c)[1] = (a)[1] * f + (b)[1] * (1 - f), \
    (c)[2] = (a)[2] * f + (b)[2] * (1 - f)

#define mul_aabb(a, b, c) mul3((a).min, (b).min, (c).min), mul3((a).max, (b).max, (c).min)

struct AABB
{
    float3 min;
    float3 max;
};

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} AXIS;

/* Multiply 4x4 matrix m1 by 4x4 matrix matrix2 and store the result in matrix1 */
void multiply4x4(float *matrix1, const float *matrix2);

/* Generate a perspective view matrix using a field of view angle fov,
 * window aspect ratio, near and far clipping planes */
void perspective(float *matrix, float fov, float aspect, float nearz, float farz);

/* Perform translation operations on a matrix */
void translate(float *matrix, float x, float y, float z);

/* Rotate a matrix by an angle on a X, Y, or Z axis specified by the AXIS enum */
void rotate(float *matrix, float angle, AXIS axis);

/* Scale a matrix by value x, y, z and store the result in matrix */
void scale(float *matrix, float x, float y, float z);

void transpose(float *a);

void make_aabb0(float px, float py, float pz, float sx, float sy, float sz, struct AABB *aabb);
void make_aabb(float *vertices, int count, struct AABB *aabb);
int overlaps(struct AABB *a, struct AABB *b);

void randv3(float3 v);
