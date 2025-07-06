#ifndef LIGHTING_H
#define LIGHTING_H

#include "math3d.h" // Needed for vec3_t

// Structure to define a point light source
typedef struct {
    vec3_t position; // Position of the light source in world space
    // Add other light properties here if needed, e.g., color, intensity
    // vec3_t color;
    float  intensity; // float intensity;
} light_source_t;

// Function to calculate a basic diffuse light intensity for a surface
// This will return an intensity value (0.0 to 1.0) based on the surface normal, light direction, and ambient light.
// You'll need to pass the normal of the surface being lit.
float calculate_diffuse_intensity(const vec3_t* surface_normal, const vec3_t* light_direction_from_surface);
// Add other lighting calculation functions as needed (e.g., specular, ambient)

#endif // LIGHTING_H