#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <math.h>
#include <string.h> // For sprintf

// Include your custom headers
#include "canvas.h"
#include "math3d.h" // Your math3d.h is crucial here
#include "renderer.h"
#include "lighting.h" // Already present in your version, good!
#include "animation.h"

// Define a common constant for converting degrees to radians
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI / 180.0f) // Use PI from math3d.h
#endif

// --- Model Utility Functions ---

// Function to generate a simple cube model for testing
model_t* generate_cube(float size) {
    model_t* cube = (model_t*)malloc(sizeof(model_t));
    if (!cube) {
        perror("Failed to allocate model_t for cube");
        return NULL;
    }

    cube->num_vertices = 8;
    cube->vertices = (vec3_t*)malloc(cube->num_vertices * sizeof(vec3_t));
    if (!cube->vertices) {
        perror("Failed to allocate vertices for cube");
        free(cube);
        return NULL;
    }

    float half_size = size / 2.0f;
    // Define vertices of a cube centered at origin
    cube->vertices[0] = vec3_create(-half_size, -half_size, -half_size); // 0: FBL (Front Bottom Left)
    cube->vertices[1] = vec3_create( half_size, -half_size, -half_size); // 1: FBR
    cube->vertices[2] = vec3_create( half_size,  half_size, -half_size); // 2: FTR
    cube->vertices[3] = vec3_create(-half_size,  half_size, -half_size); // 3: FTL
    cube->vertices[4] = vec3_create(-half_size, -half_size,  half_size); // 4: BBL
    cube->vertices[5] = vec3_create( half_size, -half_size,  half_size); // 5: BBR
    cube->vertices[6] = vec3_create( half_size,  half_size,  half_size); // 6: BTR
    cube->vertices[7] = vec3_create(-half_size,  half_size,  half_size); // 7: BTL

    cube->num_edges = 12; // (Not strictly used by face renderer, but keeping structure)
    cube->edges = (edge_t*)malloc(cube->num_edges * sizeof(edge_t));
    if (!cube->edges) {
        perror("Failed to allocate edges for cube");
        free(cube->vertices);
        free(cube);
        return NULL;
    }

    // Cube edges (12 edges connecting the vertices)
    cube->edges[0] = (edge_t){0, 1}; cube->edges[1] = (edge_t){1, 2};
    cube->edges[2] = (edge_t){2, 3}; cube->edges[3] = (edge_t){3, 0};
    cube->edges[4] = (edge_t){4, 5}; cube->edges[5] = (edge_t){5, 6};
    cube->edges[6] = (edge_t){6, 7}; cube->edges[7] = (edge_t){7, 4};
    cube->edges[8] = (edge_t){0, 4}; cube->edges[9] = (edge_t){1, 5};
    cube->edges[10] = (edge_t){2, 6}; cube->edges[11] = (edge_t){3, 7};

    cube->num_faces = 12; // 6 cube faces * 2 triangles/face
    cube->faces = (face_t*)malloc(cube->num_faces * sizeof(face_t));
    if (!cube->faces) {
        perror("Failed to allocate faces for cube");
        free(cube->edges);
        free(cube->vertices);
        free(cube);
        return NULL;
    }

    // Front face (Z-)
    cube->faces[0] = (face_t){0, 1, 2};
    cube->faces[1] = (face_t){0, 2, 3};
    // Back face (Z+)
    cube->faces[2] = (face_t){4, 6, 5}; // Note vertex order for normal
    cube->faces[3] = (face_t){4, 7, 6};
    // Left face (X-)
    cube->faces[4] = (face_t){0, 3, 7};
    cube->faces[5] = (face_t){0, 7, 4};
    // Right face (X+)
    cube->faces[6] = (face_t){1, 5, 6};
    cube->faces[7] = (face_t){1, 6, 2};
    // Bottom face (Y-)
    cube->faces[8] = (face_t){0, 4, 5};
    cube->faces[9] = (face_t){0, 5, 1};
    // Top face (Y+)
    cube->faces[10] = (face_t){3, 2, 6};
    cube->faces[11] = (face_t){3, 6, 7};
    
    // Edges are no longer strictly necessary for face-based rendering
    // Reset num_edges and edges to NULL if you don't intend to render them as wireframe explicitly.
    // However, if your render_wireframe function still iterates edges, keep them.
    // Based on the new renderer.c, it iterates faces, so these can be zeroed out.
    // cube->num_edges = 0;
    // cube->edges = NULL;

    return cube;
}

// Function to free model memory
void free_model(model_t* model) {
    if (model) {
        if (model->vertices) {
            free(model->vertices);
            model->vertices = NULL;
        }
        if (model->edges) {
            free(model->edges);
            model->edges = NULL;
        }
        if (model->faces) { // NEW: Free faces
            free(model->faces);
            model->faces = NULL;
        }
        free(model);
        model = NULL;
    }
}


int main() {
    printf("main2.c: Starting animation rendering...\n"); // Debug print

    // Canvas dimensions
    const int WIDTH = 800;
    const int HEIGHT = 600;
    const int NUM_FRAMES = 360; // Total frames for one animation cycle

    // Create the canvas
    canvas_t* my_canvas = canvas_create(WIDTH, HEIGHT);
    if (!my_canvas) {
        return EXIT_FAILURE;
    }

    // Ensure 'frames' directory exists (create it manually or add system call)

    // --- Scene Setup ---

    // 1. Define Camera (View Matrix)
    // Move the camera back along the Z-axis to see the scene
    vec3_t camera_position = vec3_create(0.0f, 0.0f, -5.0f); // Camera is at (0,0,-5) looking towards origin
    mat4_t view_matrix = mat4_translate(vec3_negate(&camera_position)); // Translate scene by negative camera position

    // 2. Define Projection Matrix (Perspective Projection)
    float aspect_ratio = (float)WIDTH / HEIGHT;
    float fov_y = 60.0f * DEG_TO_RAD; // 60 degrees Field of View in Y
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    mat4_t projection_matrix = mat4_perspective(fov_y, aspect_ratio, near_plane, far_plane);

    light_source_t main_light = {
    .position = vec3_create(0.0f, 5.0f, -5.0f),
    .intensity = 1.0f // Set a default intensity (e.g., 1.0f for full brightness)
    };

    // 3. Define Models (Two Planets)
    model_t* planet_model_1 = generate_cube(0.4f); // First planet
    model_t* planet_model_2 = generate_cube(0.2f); // Second, smaller planet
    
    if (!planet_model_1 || !planet_model_2) {
        canvas_destroy(my_canvas);
        free_model(planet_model_1); 
        free_model(planet_model_2);
        return EXIT_FAILURE;
    }

    // 4. Define Planet 1 Orbital Path (Bézier Control Points)
    // Larger orbit
    vec3_t orbit1_p0 = vec3_create( 2.5f, 0.0f,  0.0f);
    vec3_t orbit1_p1 = vec3_create( 3.5f, 1.0f, -1.5f);
    vec3_t orbit1_p2 = vec3_create(-1.0f, 2.5f,  1.5f);
    vec3_t orbit1_p3 = vec3_create(-2.5f, 0.0f,  0.0f);
    vec3_t orbit1_p4 = vec3_create(-3.5f, -1.0f, 1.5f);
    vec3_t orbit1_p5 = vec3_create( 1.0f, -2.5f, -1.5f);

    // 5. Define Planet 2 Orbital Path (Bézier Control Points)
    // Smaller, slightly offset orbit
    vec3_t orbit2_p0 = vec3_create( 1.5f, 0.0f,  0.5f);
    vec3_t orbit2_p1 = vec3_create( 1.0f, 1.0f, -0.5f);
    vec3_t orbit2_p2 = vec3_create(-0.5f, 1.5f,  0.5f);
    vec3_t orbit2_p3 = vec3_create(-1.5f, 0.0f,  0.5f);
    vec3_t orbit2_p4 = vec3_create(-1.0f, -1.0f, -0.5f);
    vec3_t orbit2_p5 = vec3_create( 0.5f, -1.5f,  0.5f);

    // --- Animation Loop ---
    for (int frame = 0; frame < NUM_FRAMES; ++frame) {
        canvas_clear(my_canvas, 0.0f); // Clear canvas to black (0.0f intensity)
        canvas_clear_depth(my_canvas, 1.0f); // NEW: Clear depth buffer to 'far' (1.0) for each frame

        float animation_t = (float)frame / (NUM_FRAMES - 1); // 0.0 to 1.0 over the animation

        // --- Planet 1 Model ---
        mat4_t planet1_model_matrix = mat4_identity();

        vec3_t planet1_position;
        if (animation_t < 0.5f) { // First half of the orbit
            float segment_t = animation_t * 2.0f; // Scale t to 0.0-1.0 for this segment
            planet1_position = vec3_bezier_cubic(segment_t, orbit1_p0, orbit1_p1, orbit1_p2, orbit1_p3);
        } else { // Second half of the orbit
            float segment_t = (animation_t - 0.5f) * 2.0f; // Scale t to 0.0-1.0 for this segment
            planet1_position = vec3_bezier_cubic(segment_t, orbit1_p3, orbit1_p4, orbit1_p5, orbit1_p0); // Back to P0
        }
        
        mat4_t planet1_translation = mat4_translate(planet1_position);
        planet1_model_matrix = mat4_multiply(&planet1_model_matrix, &planet1_translation);

        float planet1_spin_angle = animation_t * TWO_PI * 5; // Spin 5 times faster
        mat4_t planet1_local_rotation = mat4_rotate_xyz(planet1_spin_angle, 0.0f, 0.0f);
        planet1_model_matrix = mat4_multiply(&planet1_model_matrix, &planet1_local_rotation);
        
        render_wireframe(my_canvas, planet_model_1, planet1_model_matrix, view_matrix, projection_matrix, &main_light);


        // --- Planet 2 Model ---
        mat4_t planet2_model_matrix = mat4_identity();
        
        // Offset the animation_t for the second planet to make it asynchronous
        float animation_t_offset = fmodf(animation_t + 0.25f, 1.0f); // Start 1/4 of the way through
        vec3_t planet2_position;
        if (animation_t_offset < 0.5f) { // First half of the orbit
            float segment_t = animation_t_offset * 2.0f;
            planet2_position = vec3_bezier_cubic(segment_t, orbit2_p0, orbit2_p1, orbit2_p2, orbit2_p3);
        } else { // Second half of the orbit
            float segment_t = (animation_t_offset - 0.5f) * 2.0f;
            planet2_position = vec3_bezier_cubic(segment_t, orbit2_p3, orbit2_p4, orbit2_p5, orbit2_p0);
        }
        
        mat4_t planet2_translation = mat4_translate(planet2_position);
        planet2_model_matrix = mat4_multiply(&planet2_model_matrix, &planet2_translation);

        float planet2_spin_angle = animation_t_offset * TWO_PI * 3; // Spin 3 times
        mat4_t planet2_local_rotation = mat4_rotate_xyz(0.0f, planet2_spin_angle, 0.0f);
        planet2_model_matrix = mat4_multiply(&planet2_model_matrix, &planet2_local_rotation);
        
        render_wireframe(my_canvas, planet_model_2, planet2_model_matrix, view_matrix, projection_matrix, &main_light);


        // Save frame to PGM
        char filename[256];
        sprintf(filename, "frames/frame_%04d.pgm", frame);
        if (canvas_save_to_pgm(my_canvas, filename) != 0) {
            fprintf(stderr, "main2.c: Failed to save frame %d to %s.\n", frame, filename);
            break;
        }
        printf("main2.c: Frame %d saved to %s.\n", frame, filename);
    }

    printf("main2.c: Animation rendering finished. Cleaning up...\n");
    // Clean up allocated memory
    free_model(planet_model_1);
    free_model(planet_model_2);
    canvas_destroy(my_canvas);

    return EXIT_SUCCESS;
}