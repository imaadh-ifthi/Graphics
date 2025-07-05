// math3d.c
#include "math3d.h"
#include <math.h>
#include <stdbool.h>

// Create a vec3 from Cartesian coordinates
vec3_t vec3_create(float x, float y, float z) {
    vec3_t v = {x, y, z, 0, 0, 0};
    vec3_update_spherical(&v);
    return v;
}

// Create a vec3 from spherical coordinates
vec3_t vec3_from_spherical(float r, float theta, float phi) {
    vec3_t v = {0, 0, 0, r, theta, phi};
    vec3_update_cartesian(&v);
    return v;
}

// Update spherical coords from Cartesian
void vec3_update_spherical(vec3_t* v) {
    v->r = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    v->theta = atan2f(v->y, v->x);
    v->phi = (v->r == 0.0f) ? 0.0f : acosf(v->z / v->r);
}

// Update Cartesian coords from spherical
void vec3_update_cartesian(vec3_t* v) {
    float sin_phi = sinf(v->phi);
    v->x = v->r * sin_phi * cosf(v->theta);
    v->y = v->r * sin_phi * sinf(v->theta);
    v->z = v->r * cosf(v->phi);
}

// Fast inverse square root (approximation)
float fast_inverse_sqrt(float x) {
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    return x * (1.5f - xhalf * x * x);
}

// Normalize using fast inverse sqrt
vec3_t vec3_normalize_fast(const vec3_t* v) {
    float inv_len = fast_inverse_sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    return vec3_create(v->x * inv_len, v->y * inv_len, v->z * inv_len);
}

// Spherical linear interpolation (SLERP)
vec3_t vec3_slerp(const vec3_t* a, const vec3_t* b, float t) {
    float dot = a->x * b->x + a->y * b->y + a->z * b->z;
    dot = fmaxf(-1.0f, fminf(dot, 1.0f)); // Clamp to avoid acos domain error

    float theta = acosf(dot) * t;
    vec3_t relative = vec3_create(b->x - a->x * dot, b->y - a->y * dot, b->z - a->z * dot);
    relative = vec3_normalize_fast(&relative);

    return vec3_create(
        a->x * cosf(theta) + relative.x * sinf(theta),
        a->y * cosf(theta) + relative.y * sinf(theta),
        a->z * cosf(theta) + relative.z * sinf(theta)
    );
}

// === Matrix functions ===

// Identity matrix
mat4_t mat4_identity(void) {
    mat4_t m = {0};
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
    return m;
}

// Translation matrix
mat4_t mat4_translate(float tx, float ty, float tz) {
    mat4_t m = mat4_identity();
    m.m[12] = tx;
    m.m[13] = ty;
    m.m[14] = tz;
    return m;
}

// Scaling matrix
mat4_t mat4_scale(float sx, float sy, float sz) {
    mat4_t m = {0};
    m.m[0] = sx;
    m.m[5] = sy;
    m.m[10] = sz;
    m.m[15] = 1.0f;
    return m;
}

// Rotation matrix XYZ
mat4_t mat4_rotate_xyz(float rx, float ry, float rz) {
    float cx = cosf(rx), sx = sinf(rx);
    float cy = cosf(ry), sy = sinf(ry);
    float cz = cosf(rz), sz = sinf(rz);

    mat4_t m = {0};
    m.m[0] = cy * cz;
    m.m[1] = sx * sy * cz - cx * sz;
    m.m[2] = cx * sy * cz + sx * sz;
    m.m[4] = cy * sz;
    m.m[5] = sx * sy * sz + cx * cz;
    m.m[6] = cx * sy * sz - sx * cz;
    m.m[8] = -sy;
    m.m[9] = sx * cy;
    m.m[10] = cx * cy;
    m.m[15] = 1.0f;
    return m;
}

// Frustum projection matrix
mat4_t mat4_frustum_asymmetric(float left, float right, float bottom, float top, float near, float far) {
    mat4_t m = {0};
    m.m[0] = (2.0f * near) / (right - left);
    m.m[5] = (2.0f * near) / (top - bottom);
    m.m[8] = (right + left) / (right - left);
    m.m[9] = (top + bottom) / (top - bottom);
    m.m[10] = -(far + near) / (far - near);
    m.m[11] = -1.0f;
    m.m[14] = -(2.0f * far * near) / (far - near);
    return m;
}

// Matrix multiplication
// Matrix multiplication: result = A * B; modified
mat4_t mat4_multiply(const mat4_t* a, const mat4_t* b) {
    mat4_t result = {0}; // Initialize all to zero
    for (int col = 0; col < 4; ++col) { // Iterate columns of result (and B)
        for (int row = 0; row < 4; ++row) { // Iterate rows of result (and A)
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) { // Dot product
                // a->m[k * 4 + row] is A[row][k]
                // b->m[col * 4 + k] is B[k][col]
                sum += a->m[k * 4 + row] * b->m[col * 4 + k];
            }
            result.m[col * 4 + row] = sum;
        }
    }
    return result;
}

// Transform point with matrix
// In src/math3d.c
// Transform point with matrix (now handles homogeneous coordinates)
vec3_t mat4_transform_point(const mat4_t* m, const vec3_t* point) {
    vec3_t result;
    // Calculate the homogeneous coordinates
    float x_hom = m->m[0] * point->x + m->m[4] * point->y + m->m[8] * point->z + m->m[12];
    float y_hom = m->m[1] * point->x + m->m[5] * point->y + m->m[9] * point->z + m->m[13];
    float z_hom = m->m[2] * point->x + m->m[6] * point->y + m->m[10] * point->z + m->m[14];
    float w_hom = m->m[3] * point->x + m->m[7] * point->y + m->m[11] * point->z + m->m[15]; // Get W component!

    // Perform perspective division if w_hom is not zero
    if (w_hom != 0.0f) {
        result.x = x_hom / w_hom;
        result.y = y_hom / w_hom;
        result.z = z_hom / w_hom; // Keep z for depth buffer or sorting later
    } else {
        // Handle case where w is zero (point at infinity, or error)
        // For now, set to zero or a large value to avoid NaNs/Infs
        result.x = 0.0f;
        result.y = 0.0f;
        result.z = 0.0f;
    }

    // Do NOT call vec3_update_spherical here. This is a 3D point in transformed Cartesian space.
    // The spherical coordinates are irrelevant for the rendering pipeline here.
    // Remove: vec3_update_spherical(&result);
    return result;
}
