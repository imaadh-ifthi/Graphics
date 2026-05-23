#include "../include/math3d.h"
#include <math.h>
#include <stdbool.h>
#include <string.h> // For memcpy/memset if used for identity matrix
#include <stdlib.h> // For malloc/free
#include <stdio.h> // For printf, if needed for debugging


// Create a vec3 from Cartesian coordinates
vec3_t vec3_create(float x, float y, float z) {
    vec3_t v = {x, y, z, 0, 0, 0};
    vec3_update_spherical(&v);
    return v;
}

// Update spherical coords from Cartesian
void vec3_update_spherical(vec3_t* v) {
    v->r = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    v->theta = atan2f(v->y, v->x);
    v->phi = (v->r == 0.0f) ? 0.0f : acosf(v->z / v->r);
}

vec3_t vec3_from_spherical(float r, float theta, float phi) {
    vec3_t v;
    v.r = r;
    v.theta = theta;
    v.phi = phi;
    vec3_update_cartesian(&v); // Assuming you have vec3_update_cartesian that calculates x,y,z from r,theta,phi
    return v;
}

float vec3_length(const vec3_t* v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

// Update Cartesian coords from spherical
void vec3_update_cartesian(vec3_t* v) {
    float sin_phi = sinf(v->phi);
    v->x = v->r * sin_phi * cosf(v->theta);
    v->y = v->r * sin_phi * sinf(v->theta);
    v->z = v->r * cosf(v->phi);
}

// Normalize a vector (returns new normalized vector)
vec3_t vec3_normalize(const vec3_t* v) {
    float length = vec3_length(v);
    if (length == 0.0f) {
        return vec3_create(0.0f, 0.0f, 0.0f); // Return zero vector to avoid division by zero
    }
    return vec3_create(v->x / length, v->y / length, v->z / length);
}

vec3_t vec3_normalize_fast(const vec3_t* v) {
    float inv_len = 1.0f / sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    return vec3_create(v->x * inv_len, v->y * inv_len, v->z * inv_len);
}

float vec3_dot(const vec3_t* a, const vec3_t* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Vector subtraction (a - b) (returns new vector)
vec3_t vec3_subtract(const vec3_t* a, const vec3_t* b) {
    return vec3_create(a->x - b->x, a->y - b->y, a->z - b->z);
}

vec3_t vec3_add(const vec3_t* a, const vec3_t* b) {
    return vec3_create(a->x + b->x, a->y + b->y, a->z + b->z);
}

vec3_t vec3_scale(const vec3_t* v, float s) {
    return vec3_create(v->x * s, v->y * s, v->z * s);
}

// Cross product of two vectors (returns new vector)
vec3_t vec3_cross(const vec3_t* a, const vec3_t* b) {
    return vec3_create(a->y * b->z - a->z * b->y,
                       a->z * b->x - a->x * b->z,
                       a->x * b->y - a->y * b->x);
}

// Negate a vector (returns new vector with opposite direction)
vec3_t vec3_negate(const vec3_t* v) {
    return vec3_create(-v->x, -v->y, -v->z);
}

// src/math3d.c (within the vector functions section)

// ... (other vector functions)

// Linear interpolation between two vectors
vec3_t vec3_lerp(const vec3_t* a, const vec3_t* b, float t) {
    // 1. Calculate (b - a) and store it in a temporary variable
    vec3_t b_minus_a = vec3_subtract(b, a);

    // 2. Scale the result by t.
    //    Ensure vec3_scale uses the signature: vec3_scale(const vec3_t* v, float s)
    vec3_t scaled_diff = vec3_scale(&b_minus_a, t); // Pass address of b_minus_a, then float t

    // 3. Add 'a' to the scaled difference
    return vec3_add(a, &scaled_diff);
}

// ... (other vector functions)


// Bezier curve (updated to use vec3_t by value for points)
vec3_t vec3_bezier_cubic(float t, vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    vec3_t p;
    p = vec3_scale(&p0, uuu);
    vec3_t scaled_p1 = vec3_scale(&p1, 3 * uu * t);
    p = vec3_add(&p, &scaled_p1);

    vec3_t scaled_p2 = vec3_scale(&p2, 3 * u * tt);
    p = vec3_add(&p, &scaled_p2);

    vec3_t scaled_p3 = vec3_scale(&p3, ttt); // Assuming ttt is a float scalar to scale p3
    p = vec3_add(&p, &scaled_p3);
    return p;
}

vec3_t vec3_slerp(const vec3_t* a, const vec3_t* b, float t) {
    vec3_t result;

    // Calculate angle between them.
    float dot = (a->x * b->x + a->y * b->y + a->z * b->z);
    dot = fmaxf(-1.0f, fminf(1.0f, dot)); // Clamp to valid range for acosf
    float angle = acosf(dot);

    if (fabsf(angle) < 0.0001f) { // If vectors are very close, just return 'a' or 'b'
        return *a;
    }

    float sin_angle = sinf(angle);
    float s0 = sinf((1.0f - t) * angle) / sin_angle;
    float s1 = sinf(t * angle) / sin_angle;

    result.x = (a->x * s0) + (b->x * s1);
    result.y = (a->y * s0) + (b->y * s1);
    result.z = (a->z * s0) + (b->z * s1);

    // After SLERP, update spherical coordinates if your struct uses them
    vec3_update_spherical(&result);
    return result;
}

// Identity matrix
mat4_t mat4_identity() {
    mat4_t m;
    memset(m.m, 0, sizeof(m.m)); // Initialize all to 0
    m.m[0] = 1.0f; // Diagonal elements to 1
    m.m[5] = 1.0f;
    m.m[10] = 1.0f;
    m.m[15] = 1.0f;
    return m;
}

// Translation matrix (takes vec3_t by value)
mat4_t mat4_translate(vec3_t t) {
    mat4_t m = mat4_identity();
    m.m[12] = t.x; // Column 3, Row 0 (X translation)
    m.m[13] = t.y; // Column 3, Row 1 (Y translation)
    m.m[14] = t.z; // Column 3, Row 2 (Z translation)
    return m;
}

// Rotation around X axis
mat4_t mat4_rotate_x(float angle_rad) {
    mat4_t m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    m.m[5] = c; m.m[6] = s;
    m.m[9] = -s; m.m[10] = c;
    return m;
}

// Rotation around Y axis
mat4_t mat4_rotate_y(float angle_rad) {
    mat4_t m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    m.m[0] = c; m.m[2] = -s;
    m.m[8] = s; m.m[10] = c;
    return m;
}

// Rotation around Z axis
mat4_t mat4_rotate_z(float angle_rad) {
    mat4_t m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    m.m[0] = c; m.m[1] = s;
    m.m[4] = -s; m.m[5] = c;
    return m;
}

// Combined XYZ rotation (order X->Y->Z)
mat4_t mat4_rotate_xyz(float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    mat4_t rot_x = mat4_rotate_x(angle_x_rad);
    mat4_t rot_y = mat4_rotate_y(angle_y_rad);
    mat4_t rot_z = mat4_rotate_z(angle_z_rad);

    mat4_t temp = mat4_multiply(&rot_y, &rot_x);
    return mat4_multiply(&rot_z, &temp);
}


// Scaling matrix (takes vec3_t by value)
mat4_t mat4_scale(vec3_t s) {
    mat4_t m = mat4_identity();
    m.m[0] = s.x;
    m.m[5] = s.y;
    m.m[10] = s.z;
    return m;
}

// Matrix multiplication (takes const pointers)
mat4_t mat4_multiply(const mat4_t* a, const mat4_t* b) {
    mat4_t result;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a->m[k * 4 + row] * b->m[col * 4 + k];
            }
            result.m[col * 4 + row] = sum;
        }
    }
    return result;
}

// Perspective projection matrix
mat4_t mat4_perspective(float fov_y, float aspect_ratio, float near_plane, float far_plane) {
    mat4_t m;
    memset(m.m, 0, sizeof(m.m));

    float tan_half_fov_y = tanf(fov_y / 2.0f);
    float f = 1.0f / tan_half_fov_y;

    m.m[0] = f / aspect_ratio;
    m.m[5] = f;
    m.m[10] = (far_plane + near_plane) / (near_plane - far_plane);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
    m.m[15] = 0.0f; // Last element should be 0 for perspective projection
    return m;
}

mat4_t mat4_frustum_asymmetric(float left, float right, float bottom, float top, float near_plane, float far_plane) {
    mat4_t m = mat4_identity(); // Start with an identity matrix
    float width = right - left;
    float height = top - bottom;
    float depth = far_plane - near_plane;

    if (width == 0.0f || height == 0.0f || depth == 0.0f) {
        // Handle error: dimensions cannot be zero
        // You might want to return an identity matrix or a zero matrix,
        // or print an error and exit. For now, returning identity.
        return m;
    }

    m.m[0] = (2.0f * near_plane) / width;
    m.m[5] = (2.0f * near_plane) / height;
    m.m[8] = (right + left) / width;
    m.m[9] = (top + bottom) / height;
    m.m[10] = -(far_plane + near_plane) / depth;
    m.m[11] = -1.0f;
    m.m[14] = -(2.0f * far_plane * near_plane) / depth;
    m.m[15] = 0.0f; // This should usually be 0 for projection matrices
    return m;
}

// Transform point with matrix (now correctly handles homogeneous coordinates)
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
        result.z = z_hom / w_hom;
    } else {
        // Handle the case where w_hom is zero (e.g., point at infinity or invalid)
        // For rendering, you might want to clip or handle this specially.
        // For now, setting to a large value or zero might indicate an error.
        result.x = x_hom;
        result.y = y_hom;
        result.z = z_hom;
    }
    // Spherical coordinates are not relevant for transformed points directly after matrix operations
    result.r = 0.0f;
    result.theta = 0.0f;
    result.phi = 0.0f;
    return result;
}

vec3_t mat4_multiply_vec3(const mat4_t* m, vec3_t v) {
    vec3_t result;
    // Calculate the homogeneous coordinates
    float x_hom = m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z + m->m[12];
    float y_hom = m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z + m->m[13];
    float z_hom = m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z + m->m[14];
    float w_hom = m->m[3] * v.x + m->m[7] * v.y + m->m[11] * v.z + m->m[15]; // Get W component!

    // Perform perspective division if w_hom is not zero
    if (w_hom != 0.0f) {
        result.x = x_hom / w_hom;
        result.y = y_hom / w_hom;
        result.z = z_hom / w_hom;
    } else {
        // Handle case where w_hom is zero (e.g., point at infinity or vector)
        // For a point transformation, if w_hom is 0, the point is at infinity.
        // Returning 0,0,0 or clamping can be alternatives depending on desired behavior.
        // For rendering pipeline, points with w=0 often get clipped.
        fprintf(stderr, "Warning: mat4_multiply_vec3 encountered a W component of 0. Returning 0 vector.\n");
        result.x = 0.0f;
        result.y = 0.0f;
        result.z = 0.0f;
    }
    return result;
}

// Create a view matrix (Look-At matrix)
mat4_t mat4_look_at(const vec3_t* camera_pos, const vec3_t* target_pos, const vec3_t* up_vector) {
    mat4_t result;

    // 1. Calculate the forward (Z) axis of the camera
    // It points from camera position to target position.
    // Store the result of vec3_subtract in a temporary lvalue (variable).
    vec3_t direction_to_target = vec3_subtract(target_pos, camera_pos);

    // Then, negate this direction and normalize it to get the camera's forward vector.
    // Camera's Z axis points "into" the screen, so it's the negation of the direction from camera to target.
    vec3_t temp_forward_unnormalized = vec3_negate(&direction_to_target);
    vec3_t forward = vec3_normalize_fast(&temp_forward_unnormalized);


    // 2. Calculate the right (X) axis of the camera
    // Cross product of Up and Forward to get Right
    vec3_t cross_product_result = vec3_cross(up_vector, &forward);
    vec3_t right = vec3_normalize_fast(&cross_product_result);
    // 3. Calculate the actual Up (Y) axis of the camera
    // Cross product of Forward and Right to ensure orthogonality
    vec3_t up = vec3_cross(&forward, &right); // Already normalized if forward and right are normalized

    // Construct the rotation part of the view matrix
    // This is the inverse rotation of the camera's orientation
    result.m[0] = right.x;    result.m[4] = up.x;    result.m[8] = forward.x;     result.m[12] = 0.0f;
    result.m[1] = right.y;    result.m[5] = up.y;    result.m[9] = forward.y;     result.m[13] = 0.0f;
    result.m[2] = right.z;    result.m[6] = up.z;    result.m[10] = forward.z;    result.m[14] = 0.0f;
    result.m[3] = 0.0f;       result.m[7] = 0.0f;       result.m[11] = 0.0f;       result.m[15] = 1.0f;

    // Add the translation part (inverse of camera's position)
    // The translation component is -dot(axis, camera_pos)
    result.m[12] = -vec3_dot(&right, camera_pos);
    result.m[13] = -vec3_dot(&up, camera_pos);
    result.m[14] = -vec3_dot(&forward, camera_pos);

    return result;
}