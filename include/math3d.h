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
float vec3_dot(const vec3_t* a, const vec3_t* b); // Add or confirm this for lighting
vec3_t vec3_cross(const vec3_t* a, const vec3_t* b); // Add or confirm this for normals
vec3_t vec3_sub(const vec3_t* a, const vec3_t* b); // For light direction vector
vec3_t vec3_add(const vec3_t* a, const vec3_t* b); // For bezier
vec3_t vec3_scale(const vec3_t* v, float s); // For bezier
vec3_t vec3_slerp(const vec3_t* a, const vec3_t* b, float t); 
vec3_t vec3_subtract(const vec3_t* a, const vec3_t* b);
vec3_t vec3_cross(const vec3_t* a, const vec3_t* b);
vec3_t vec3_negate(const vec3_t* v);


// Bezier curve function (ensure it takes vec3_t by value for points)
vec3_t vec3_bezier_cubic(float t, vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3);

// === Matrix functions ===
mat4_t mat4_identity();
mat4_t mat4_translate(vec3_t t); // Takes vec3_t by value
mat4_t mat4_rotate_x(float angle_rad);
mat4_t mat4_rotate_y(float angle_rad);
mat4_t mat4_rotate_z(float angle_rad);
mat4_t mat4_rotate_xyz(float angle_x_rad, float angle_y_rad, float angle_z_rad); // Ensure this is correct
mat4_t mat4_scale(vec3_t s); // Takes vec3_t by value
mat4_t mat4_multiply(const mat4_t* a, const mat4_t* b); // Takes const pointers
mat4_t mat4_perspective(float fov_rad, float aspect_ratio, float near_plane, float far_plane);
mat4_t mat4_frustum_asymmetric(float left, float right, float bottom, float top, float near_plane, float far_plane);
mat4_t mat4_transpose(const mat4_t* m);

mat4_t mat4_look_at(const vec3_t* camera_pos, const vec3_t* target_pos, const vec3_t* up_vector);
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far);

// Transform point with matrix (now correctly handles homogeneous coordinates and returns vec3_t)
vec3_t mat4_transform_point(const mat4_t* m, const vec3_t* point);
vec3_t mat4_multiply_vec3(const mat4_t* m, vec3_t v);


#endif // MATH3D_H