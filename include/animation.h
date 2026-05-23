// include/animation.h
#ifndef ANIMATION_H
#define ANIMATION_H

#include "math3d.h"
#include "renderer.h"
#include "canvas.h" // If animation functions directly interact with canvas for effects

// Structure to hold animation state for a single animated object
typedef struct {
    float current_time;    // Current time in the animation cycle (e.g., 0.0 to 1.0)
    float duration;        // Total duration of one animation cycle
    int num_frames;        // Total number of frames for the animation
    vec3_t initial_position; // Initial position (if applicable)
    vec3_t orbit_p0, orbit_p1, orbit_p2, orbit_p3; // Control points for Bezier orbits
    vec3_t orbit_p4, orbit_p5; // Additional control points for longer orbits or specific paths
    float spin_multiplier; // How fast the object spins locally
    float orbit_speed_multiplier; // How fast the object moves along its orbit
    // Add more animation-specific parameters as needed
} animation_state_t;

// Function to initialize an animation state
void animation_init(animation_state_t* state, float duration, int num_frames);

// Function to update the animation state for a given frame
// This function will calculate the model matrix for the animated object
mat4_t animation_update_model_matrix(animation_state_t* state, int frame);

// If you want to encapsulate the entire animation loop:
void run_animation(
    canvas_t* canvas,
    model_t* sun_model,
    model_t* planet_model_1,
    model_t* planet_model_2,
    light_source_t* light, // If light is animated
    int num_frames,
    int canvas_width,
    int canvas_height
);


#endif // ANIMATION_H