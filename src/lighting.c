#include "../include/lighting.h"
#include <math.h> // For fmaxf

// Implements a basic diffuse lighting model (Lambertian reflectance)
// surface_normal: Normalized normal vector of the surface point.
// light_direction_from_surface: Normalized vector pointing from the surface point towards the light source.
float calculate_diffuse_intensity(const vec3_t* surface_normal, const vec3_t* light_direction_from_surface) {
    // Basic Lambertian diffuse reflection: intensity = max(0, N . L)
    // N is the surface normal, L is the light direction.
    float dot_product = vec3_dot(surface_normal, light_direction_from_surface);
    return fmaxf(0.0f, dot_product); // Clamp to 0 to prevent negative intensity
}

// You can add more complex lighting functions here later, e.g.:
/*
float calculate_ambient_intensity(float ambient_strength) {
    return ambient_strength;
}

float calculate_specular_intensity(const vec3_t* surface_normal, const vec3_t* light_direction_from_surface, const vec3_t* view_direction, float shininess) {
    // Basic Phong specular reflection
    vec3_t reflect_direction = vec3_normalize_fast(&vec3_subtract(&vec3_scale(2.0f * vec3_dot(surface_normal, light_direction_from_surface), surface_normal), light_direction_from_surface));
    float spec_dot = vec3_dot(view_direction, &reflect_direction);
    if (spec_dot < 0.0f) return 0.0f;
    return powf(spec_dot, shininess);
}
*/