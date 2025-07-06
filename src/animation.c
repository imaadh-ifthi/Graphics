// src/animation.c
#include "../include/animation.h"
#include "../include/math3d.h"
#include "../include/renderer.h" // Needed for render_wireframe
#include "../include/canvas.h"   // Needed for canvas operations
#include "../include/lighting.h" // Needed for lighting

#include <stdio.h> // For sprintf, printf
#include <math.h>  // For fmodf

void animation_init(animation_state_t* state, float duration, int num_frames) {
    if (state) {
        state->duration = duration;
        state->num_frames = num_frames;
        state->current_time = 0.0f; // Start at time 0
        state->spin_multiplier = 1.0f; // Default spin speed
        state->orbit_speed_multiplier = 1.0f; // Default orbit speed

        // Initialize default/example Bezier control points.
        // These will likely need to be customized for each animated object.
        state->orbit_p0 = vec3_create(0.0f, 0.0f, 0.0f);
        state->orbit_p1 = vec3_create(0.0f, 0.0f, 0.0f);
        state->orbit_p2 = vec3_create(0.0f, 0.0f, 0.0f);
        state->orbit_p3 = vec3_create(0.0f, 0.0f, 0.0f);
        state->orbit_p4 = vec3_create(0.0f, 0.0f, 0.0f);
        state->orbit_p5 = vec3_create(0.0f, 0.0f, 0.0f);

        // Initial position can be set here if not part of orbit
        state->initial_position = vec3_create(0.0f, 0.0f, 0.0f);
    }
}

// Function to update the animation state and calculate model matrix for a given object
// This is a generic example. You'll likely need separate functions or more parameters
// if you have multiple objects with different animation paths.
mat4_t animation_update_model_matrix(animation_state_t* state, int frame) {
    mat4_t model_matrix = mat4_identity(); // Start with identity

    if (!state) {
        return model_matrix; // Return identity if state is null
    }

    // Calculate normalized time for the animation cycle
    state->current_time = (float)frame / (state->num_frames - 1);
    // Ensure current_time wraps around for continuous animation
    float animation_t_offset = fmodf(state->current_time * state->orbit_speed_multiplier, 1.0f);

    // Example: Using Bezier for a specific object's orbit
    // This logic is directly from your main2.c for planet 1 (or sun rotation for example)
    // You will need to pass appropriate control points to this function or the state.
    
    // For now, let's make a generic example that just rotates around Y axis for a "sun" like object
    float angle_rad = animation_t_offset * TWO_PI * state->spin_multiplier;
    model_matrix = mat4_rotate_xyz(0.0f, angle_rad, 0.0f); // Sun's self-rotation

    // Example: If this function is for a planet on an orbit
    // You would pass the control points as part of the state or function arguments.
    // This is just a placeholder example, demonstrating how the animation_t_offset
    // would be used.
    // if (animation_t_offset < 0.5f) {
    //     current_position = vec3_bezier_cubic(animation_t_offset * 2.0f, state->orbit_p0, state->orbit_p1, state->orbit_p2, state->orbit_p3);
    // } else {
    //     current_position = vec3_bezier_cubic((animation_t_offset - 0.5f) * 2.0f, state->orbit_p3, state->orbit_p4, state->orbit_p5, state->orbit_p0);
    // }
    // mat4_t translation_matrix = mat4_translate(current_position);
    // model_matrix = mat4_multiply(&translation_matrix, &model_matrix); // Apply translation after rotation, or based on your model's pivot

    return model_matrix;
}


void run_animation(
    canvas_t* my_canvas,
    model_t* sun_model,
    model_t* planet_model_1,
    model_t* planet_model_2,
    light_source_t* main_light, // Use 'main_light' to avoid conflict with 'light' in render_wireframe
    int num_frames,
    int canvas_width,
    int canvas_height
) {
    // Define camera (view matrix) and projection matrix (from main2.c)
    // You might want to make these animated as well, but for now, keep them fixed.
    vec3_t camera_pos = vec3_create(0.0f, 0.0f, 5.0f); // Camera position
    
    vec3_t target_pos_val = vec3_create(0.0f, 0.0f, 0.0f);
    vec3_t up_vector_val = vec3_create(0.0f, 1.0f, 0.0f);
    mat4_t view_matrix = mat4_look_at(&camera_pos, &target_pos_val, &up_vector_val);

    float fov_rad = 60.0f * DEG_TO_RAD;
    float aspect_ratio = (float)canvas_width / canvas_height;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    mat4_t projection_matrix = mat4_perspective(fov_rad, aspect_ratio, near_plane, far_plane);

    // Initialize animation states for each object
    animation_state_t sun_anim_state;
    animation_init(&sun_anim_state, 1.0f, num_frames); // Duration 1.0 for full cycle in num_frames
    sun_anim_state.spin_multiplier = 0.5f; // Sun spins slower

    animation_state_t planet1_anim_state;
    animation_init(&planet1_anim_state, 1.0f, num_frames);
    planet1_anim_state.spin_multiplier = 3.0f; // Planet 1 spins faster
    planet1_anim_state.orbit_speed_multiplier = 1.0f; // Completes orbit in one full animation cycle

    // Control points for Planet 1's orbit (from main2.c)
    planet1_anim_state.orbit_p0 = vec3_create(2.0f, 0.0f, 0.0f);
    planet1_anim_state.orbit_p1 = vec3_create(0.0f, 0.0f, -4.0f);
    planet1_anim_state.orbit_p2 = vec3_create(-2.0f, 0.0f, 0.0f);
    planet1_anim_state.orbit_p3 = vec3_create(0.0f, 0.0f, 4.0f);


    animation_state_t planet2_anim_state;
    animation_init(&planet2_anim_state, 1.0f, num_frames);
    planet2_anim_state.spin_multiplier = 5.0f; // Planet 2 spins even faster
    planet2_anim_state.orbit_speed_multiplier = 0.5f; // Planet 2 completes half orbit in one full animation cycle (slower overall orbit)

    // Control points for Planet 2's orbit (from main2.c)
    planet2_anim_state.orbit_p0 = vec3_create(4.0f, 0.0f, 0.0f);
    planet2_anim_state.orbit_p1 = vec3_create(0.0f, 0.0f, -8.0f);
    planet2_anim_state.orbit_p2 = vec3_create(-4.0f, 0.0f, 0.0f);
    planet2_anim_state.orbit_p3 = vec3_create(0.0f, 0.0f, 8.0f);
    planet2_anim_state.orbit_p4 = vec3_create(4.0f, 0.0f, 0.0f); // Duplicate for circular
    planet2_anim_state.orbit_p5 = vec3_create(0.0f, 0.0f, -8.0f); // Duplicate for circular


    printf("animation.c: Starting animation rendering for %d frames.\n", num_frames);

    for (int frame = 0; frame < num_frames; ++frame) {
        canvas_clear(my_canvas, 0.0f); // Clear canvas for each frame
        canvas_clear_depth(my_canvas, 1.0f); // Clear depth buffer for each frame

        // --- Sun Animation ---
        mat4_t sun_model_matrix = mat4_identity();
        float sun_angle_rad = (float)frame / (num_frames - 1) * TWO_PI * sun_anim_state.spin_multiplier;
        mat4_t sun_rotation = mat4_rotate_xyz(0.0f, sun_angle_rad, 0.0f);
        sun_model_matrix = mat4_multiply(&sun_model_matrix, &sun_rotation);
        // Render Sun
        render_wireframe(my_canvas, sun_model, sun_model_matrix, view_matrix, projection_matrix, main_light, camera_pos);


        // --- Planet 1 Animation ---
        mat4_t planet1_model_matrix = mat4_identity();
        float planet1_animation_t_offset = fmodf(((float)frame / (num_frames - 1)) * planet1_anim_state.orbit_speed_multiplier, 1.0f);
        
        vec3_t planet1_position;
        // Use the first half of the animation for the first Bezier segment, second half for the second.
        if (planet1_animation_t_offset < 0.5f) {
            float segment_t = planet1_animation_t_offset * 2.0f; // Scale t to range from 0 to 1 for this segment
            planet1_position = vec3_bezier_cubic(segment_t, planet1_anim_state.orbit_p0, planet1_anim_state.orbit_p1, planet1_anim_state.orbit_p2, planet1_anim_state.orbit_p3);
        } else { // Second half of the orbit
            float segment_t = (planet1_animation_t_offset - 0.5f) * 2.0f;
            planet1_position = vec3_bezier_cubic(segment_t, planet1_anim_state.orbit_p3, planet1_anim_state.orbit_p0, planet1_anim_state.orbit_p1, planet1_anim_state.orbit_p2); // Adjusted for circular path
        }
        
        mat4_t planet1_translation = mat4_translate(planet1_position);
        planet1_model_matrix = mat4_multiply(&planet1_model_matrix, &planet1_translation);

        float planet1_spin_angle = planet1_animation_t_offset * TWO_PI * planet1_anim_state.spin_multiplier;
        mat4_t planet1_local_rotation = mat4_rotate_xyz(0.0f, planet1_spin_angle, 0.0f);
        planet1_model_matrix = mat4_multiply(&planet1_model_matrix, &planet1_local_rotation);
        
        render_wireframe(my_canvas, planet_model_1, planet1_model_matrix, view_matrix, projection_matrix, main_light, camera_pos);


        // --- Planet 2 Animation ---
        mat4_t planet2_model_matrix = mat4_identity();
        float planet2_animation_t_offset = fmodf(((float)frame / (num_frames - 1)) * planet2_anim_state.orbit_speed_multiplier, 1.0f);

        vec3_t planet2_position;
        // The original main2.c had 6 control points and a specific path logic.
        // Replicating that logic here for planet 2.
        if (planet2_animation_t_offset < 0.5f) {
            float segment_t = planet2_animation_t_offset * 2.0f;
            planet2_position = vec3_bezier_cubic(segment_t, planet2_anim_state.orbit_p0, planet2_anim_state.orbit_p1, planet2_anim_state.orbit_p2, planet2_anim_state.orbit_p3);
        } else { // Second half of the orbit
            float segment_t = (planet2_animation_t_offset - 0.5f) * 2.0f;
            planet2_position = vec3_bezier_cubic(segment_t, planet2_anim_state.orbit_p3, planet2_anim_state.orbit_p4, planet2_anim_state.orbit_p5, planet2_anim_state.orbit_p0);
        }
        
        mat4_t planet2_translation = mat4_translate(planet2_position);
        planet2_model_matrix = mat4_multiply(&planet2_model_matrix, &planet2_translation);

        float planet2_spin_angle = planet2_animation_t_offset * TWO_PI * planet2_anim_state.spin_multiplier;
        mat4_t planet2_local_rotation = mat4_rotate_xyz(0.0f, planet2_spin_angle, 0.0f);
        planet2_model_matrix = mat4_multiply(&planet2_model_matrix, &planet2_local_rotation);
        
        render_wireframe(my_canvas, planet_model_2, planet2_model_matrix, view_matrix, projection_matrix, main_light, camera_pos);

        // Save frame to PGM
        char filename[256];
        sprintf(filename, "frames/frame_%04d.pgm", frame);
        if (canvas_save_to_pgm(my_canvas, filename) != 0) {
            fprintf(stderr, "animation.c: Failed to save frame %d to %s.\n", frame, filename);
            break;
        }
        printf("animation.c: Frame %d saved to %s.\n", frame, filename);
    }
    printf("animation.c: Animation rendering complete.\n");
}