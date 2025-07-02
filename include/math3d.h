#ifndef MATH3D_H
#define MATH3D_H

#include <math.h>

// Constants
#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define HALF_PI 1.57079632679f
#define DEG_TO_RAD (PI / 180.0f)
#define RAD_TO_DEG (180.0f / PI)

// 3D vector with Cartesian and spherical coordinates
typedef struct {
    float x, y, z;       // Cartesian coordinates
    float r, theta, phi; // Spherical coordinates (r: radius, theta: azimuth, phi: polar)
} vec3_t;

// 4x4 matrix structure (column-major layout)
typedef struct {
    float m[16]; // m[column * 4 + row]
} mat4_t;

// === Vector functions ===
vec3_t vec3_create(float x, float y, float z);
vec3_t vec3_from_spherical(float r, float theta, float phi);
void vec3_update_spherical(vec3_t* v);
void vec3_update_cartesian(vec3_t* v);
vec3_t vec3_normalize_fast(const vec3_t* v);
vec3_t vec3_slerp(const vec3_t* a, const vec3_t* b, float t);

// === Matrix functions ===
mat4_t mat4_identity(void);
mat4_t mat4_translate(float tx, float ty, float tz);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate_xyz(float rx, float ry, float rz);
mat4_t mat4_frustum_asymmetric(float left, float right, float bottom, float top, float near, float far);
mat4_t mat4_multiply(const mat4_t* a, const mat4_t* b);
vec3_t mat4_transform_point(const mat4_t* m, const vec3_t* point);

#endif // MATH3D_H