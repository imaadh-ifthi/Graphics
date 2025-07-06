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
    
    return cube;
}

// Function to generate a simple sphere model for testing
// Generates a sphere using latitude and longitude lines.
// NOTE: This implementation generates faces and edges for wireframe rendering.
// For solid rendering, you'd only need vertices and faces.
model_t* generate_sphere(float radius, int segments_h, int segments_v) {
    if (segments_h < 3 || segments_v < 2) {
        fprintf(stderr, "Error: Sphere segments must be at least 3 horizontal and 2 vertical.\n");
        return NULL;
    }

    model_t* sphere = (model_t*)malloc(sizeof(model_t));
    if (!sphere) {
        perror("Failed to allocate model_t for sphere");
        return NULL;
    }

    // Number of vertices: (segments_v - 1) * segments_h (for middle rings) + 2 (for poles)
    sphere->num_vertices = (segments_v - 1) * segments_h + 2;
    sphere->vertices = (vec3_t*)malloc(sphere->num_vertices * sizeof(vec3_t));
    if (!sphere->vertices) {
        perror("Failed to allocate vertices for sphere");
        free(sphere);
        return NULL;
    }

    int v_idx = 0;

    // Top pole
    sphere->vertices[v_idx++] = vec3_create(0.0f, 0.0f, radius); // Z-up sphere

    // Middle rings (latitude bands)
    for (int i = 1; i < segments_v; ++i) { // i for vertical segments (latitude lines)
        float phi = PI * i / segments_v; // phi from 0 to PI
        float sin_phi = sinf(phi);
        float cos_phi = cosf(phi);

        for (int j = 0; j < segments_h; ++j) { // j for horizontal segments (longitude lines)
            float theta = TWO_PI * j / segments_h; // theta from 0 to TWO_PI
            float sin_theta = sinf(theta);
            float cos_theta = cosf(theta);

            sphere->vertices[v_idx++] = vec3_create(radius * sin_phi * cos_theta,
                                                   radius * sin_phi * sin_theta,
                                                   radius * cos_phi);
        }
    }

    // Bottom pole
    sphere->vertices[v_idx++] = vec3_create(0.0f, 0.0f, -radius);

    // Faces (Triangles)
    sphere->num_faces = segments_h * 2 + (segments_v - 2) * segments_h * 2; // Top cap + bottom cap + middle quads
    sphere->faces = (face_t*)malloc(sphere->num_faces * sizeof(face_t));
    if (!sphere->faces) {
        perror("Failed to allocate faces for sphere");
        free(sphere->vertices);
        free(sphere);
        return NULL;
    }

    int f_idx = 0;
    int ring_start_idx;
    int next_ring_start_idx;

    // Top cap (triangles connecting to the top pole)
    int top_pole_idx = 0;
    ring_start_idx = 1; // First ring starts at index 1
    for (int j = 0; j < segments_h; ++j) {
        int v0 = ring_start_idx + j;
        int v1 = ring_start_idx + ((j + 1) % segments_h); // Wrap around for last vertex
        sphere->faces[f_idx++] = (face_t){top_pole_idx, v1, v0}; // Note order for consistent normal
    }

    // Middle sections (quads forming two triangles each)
    for (int i = 0; i < segments_v - 2; ++i) { // segments_v-2 middle rings
        ring_start_idx = 1 + i * segments_h;
        next_ring_start_idx = 1 + (i + 1) * segments_h;
        for (int j = 0; j < segments_h; ++j) {
            int v0 = ring_start_idx + j;
            int v1 = ring_start_idx + ((j + 1) % segments_h);
            int v2 = next_ring_start_idx + ((j + 1) % segments_h);
            int v3 = next_ring_start_idx + j;

            // First triangle of the quad
            sphere->faces[f_idx++] = (face_t){v0, v1, v2};
            // Second triangle of the quad
            sphere->faces[f_idx++] = (face_t){v0, v2, v3};
        }
    }

    // Bottom cap (triangles connecting to the bottom pole)
    int bottom_pole_idx = sphere->num_vertices - 1;
    ring_start_idx = 1 + (segments_v - 2) * segments_h; // Last ring before bottom pole
    for (int j = 0; j < segments_h; ++j) {
        int v0 = ring_start_idx + j;
        int v1 = ring_start_idx + ((j + 1) % segments_h);
        sphere->faces[f_idx++] = (face_t){bottom_pole_idx, v0, v1}; // Note order for consistent normal
    }
    
    // Edges (for wireframe rendering - simple generation, not optimized)
    sphere->num_edges = 0; // Initialize
    sphere->edges = (edge_t*)malloc(((segments_h * segments_v) * 2) * sizeof(edge_t)); // Allocate generously
    if (!sphere->edges) {
        perror("Failed to allocate edges for sphere");
        free(sphere->faces);
        free(sphere->vertices);
        free(sphere);
        return NULL;
    }
    int e_idx = 0;

    // Horizontal edges
    for (int i = 0; i < segments_v; ++i) { // Iterating through all horizontal "lines" of vertices
        if (i == 0 || i == segments_v) continue; // Skip poles in this general loop
        ring_start_idx = 1 + (i - 1) * segments_h;
        for (int j = 0; j < segments_h; ++j) {
            sphere->edges[e_idx++] = (edge_t){ring_start_idx + j, ring_start_idx + ((j + 1) % segments_h)};
        }
    }

    // Vertical edges (edges connecting rings, or poles to rings)
    // Edges from top pole to first ring
    for (int j = 0; j < segments_h; ++j) {
        sphere->edges[e_idx++] = (edge_t){top_pole_idx, 1 + j};
    }
    // Edges between middle rings
    for (int i = 0; i < segments_v - 2; ++i) {
        ring_start_idx = 1 + i * segments_h;
        next_ring_start_idx = 1 + (i + 1) * segments_h;
        for (int j = 0; j < segments_h; ++j) {
            sphere->edges[e_idx++] = (edge_t){ring_start_idx + j, next_ring_start_idx + j};
        }
    }
    // Edges from last ring to bottom pole
    ring_start_idx = 1 + (segments_v - 2) * segments_h;
    for (int j = 0; j < segments_h; ++j) {
        sphere->edges[e_idx++] = (edge_t){ring_start_idx + j, bottom_pole_idx};
    }

    sphere->num_edges = e_idx; // Set actual number of edges used

    return sphere;
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
        if (model->faces) {
            free(model->faces);
            model->faces = NULL;
        }
        free(model);
        model = NULL;
    }
}


int main() {
    printf("main2.c: Starting animation rendering...\n"); // Debug print

    // Canvas dimensions - HIGHEST RESOLUTION
    const int WIDTH = 1920; // Full HD width
    const int HEIGHT = 1080; // Full HD height
    const int NUM_FRAMES = 360; // Total frames for one animation cycle

    // Create the canvas
    canvas_t* my_canvas = canvas_create(WIDTH, HEIGHT);
    if (!my_canvas) {
        return EXIT_FAILURE;
    }

    // Ensure 'frames' directory exists (create it manually or add system call)

    // --- Scene Setup ---

    // 1. Define Camera (View Matrix) - Zoomed out
    vec3_t camera_position = vec3_create(0.0f, 0.0f, 12.0f); // Zoomed out by increasing Z
    vec3_t target_position = vec3_create(0.0f, 0.0f, 0.0f); 
    vec3_t up_vector = vec3_create(0.0f, 1.0f, 0.0f);     
    mat4_t view_matrix = mat4_look_at(&camera_position, &target_position, &up_vector);

    // 2. Define Projection Matrix (Perspective Projection)
    float aspect_ratio = (float)WIDTH / HEIGHT;
    float fov_y = 45.0f * DEG_TO_RAD; 
    float near_plane = 0.1f;
    float far_plane = 100.0f; 
    mat4_t projection_matrix = mat4_perspective(fov_y, aspect_ratio, near_plane, far_plane);

    light_source_t main_light = {
    .position = vec3_create(0.0f, 5.0f, -5.0f), 
    .intensity = 1.0f
    };

    // 3. Define Models: Sun (Cube) and Two Planets (Spheres)
    model_t* sun_model = generate_cube(0.5f); // Smaller cube
    model_t* planet_model_1 = generate_sphere(0.8f, 20, 16); // Larger sphere, more segments
    model_t* planet_model_2 = generate_sphere(0.6f, 16, 12); // Larger sphere (slightly smaller than planet 1), more segments
    
    if (!sun_model || !planet_model_1 || !planet_model_2) {
        canvas_destroy(my_canvas);
        free_model(sun_model); 
        free_model(planet_model_1); 
        free_model(planet_model_2);
        return EXIT_FAILURE;
    }

    // 4. Define Planet Orbital Paths (Bézier Control Points) - Adjusted for larger planets
    // Orbit 1: Larger orbit for planet 1, ensuring it clears the sun
    vec3_t orbit1_p0 = vec3_create( 3.5f, 0.0f,  0.0f);
    vec3_t orbit1_p1 = vec3_create( 4.5f, 1.0f, -2.0f);
    vec3_t orbit1_p2 = vec3_create(-1.5f, 3.5f,  2.0f);
    vec3_t orbit1_p3 = vec3_create(-3.5f, 0.0f,  0.0f);
    vec3_t orbit1_p4 = vec3_create(-4.5f, -1.0f, 2.0f);
    vec3_t orbit1_p5 = vec3_create( 1.5f, -3.5f, -2.0f);

    // Orbit 2: Smaller, slightly offset orbit for planet 2
    vec3_t orbit2_p0 = vec3_create( 2.5f, 0.0f,  1.0f);
    vec3_t orbit2_p1 = vec3_create( 2.0f, 1.0f, -1.0f);
    vec3_t orbit2_p2 = vec3_create(-1.0f, 2.5f,  1.0f);
    vec3_t orbit2_p3 = vec3_create(-2.5f, 0.0f,  1.0f);
    vec3_t orbit2_p4 = vec3_create(-2.0f, -1.0f, -1.0f);
    vec3_t orbit2_p5 = vec3_create( 1.0f, -2.5f,  1.0f);

    // --- Animation Loop ---
    for (int frame = 0; frame < NUM_FRAMES; ++frame) {
        canvas_clear(my_canvas, 0.0f); 
        canvas_clear_depth(my_canvas, 1.0f); 

        float animation_t = (float)frame / (NUM_FRAMES - 1); 

        // --- Sun Model (Rotating Cube) ---
        mat4_t sun_model_matrix = mat4_identity();
        float sun_spin_angle = animation_t * TWO_PI * 2; // Spin the sun twice per animation cycle
        mat4_t sun_local_rotation = mat4_rotate_xyz(0.0f, sun_spin_angle, 0.0f); // Spin around Y-axis
        sun_model_matrix = mat4_multiply(&sun_model_matrix, &sun_local_rotation);
        render_wireframe(my_canvas, sun_model, sun_model_matrix, view_matrix, projection_matrix, &main_light, camera_position);


        // --- Planet 1 Model (Orbiting Sphere) ---
        mat4_t planet1_model_matrix = mat4_identity();

        vec3_t planet1_position;
        if (animation_t < 0.5f) { 
            float segment_t = animation_t * 2.0f; 
            planet1_position = vec3_bezier_cubic(segment_t, orbit1_p0, orbit1_p1, orbit1_p2, orbit1_p3);
        } else { 
            float segment_t = (animation_t - 0.5f) * 2.0f; 
            planet1_position = vec3_bezier_cubic(segment_t, orbit1_p3, orbit1_p4, orbit1_p5, orbit1_p0); 
        }
        
        mat4_t planet1_translation = mat4_translate(planet1_position);
        planet1_model_matrix = mat4_multiply(&planet1_model_matrix, &planet1_translation);

        float planet1_spin_angle = animation_t * TWO_PI * 5; // Local spin for planet 1
        mat4_t planet1_local_rotation = mat4_rotate_xyz(planet1_spin_angle, planet1_spin_angle * 0.5f, 0.0f); // Spin on X and Y
        planet1_model_matrix = mat4_multiply(&planet1_model_matrix, &planet1_local_rotation);
        
        render_wireframe(my_canvas, planet_model_1, planet1_model_matrix, view_matrix, projection_matrix, &main_light, camera_position);

        // --- Planet 2 Model (Orbiting Sphere) ---
        mat4_t planet2_model_matrix = mat4_identity();
        
        float animation_t_offset = fmodf(animation_t + 0.25f, 1.0f); 
        vec3_t planet2_position;
        if (animation_t_offset < 0.5f) { 
            float segment_t = animation_t_offset * 2.0f;
            planet2_position = vec3_bezier_cubic(segment_t, orbit2_p0, orbit2_p1, orbit2_p2, orbit2_p3);
        } else { 
            float segment_t = (animation_t_offset - 0.5f) * 2.0f;
            planet2_position = vec3_bezier_cubic(segment_t, orbit2_p3, orbit2_p4, orbit2_p5, orbit2_p0);
        }
        
        mat4_t planet2_translation = mat4_translate(planet2_position);
        planet2_model_matrix = mat4_multiply(&planet2_model_matrix, &planet2_translation);

        float planet2_spin_angle = animation_t_offset * TWO_PI * 3; // Local spin for planet 2
        mat4_t planet2_local_rotation = mat4_rotate_xyz(0.0f, planet2_spin_angle, planet2_spin_angle * 0.7f); // Spin on Y and Z
        planet2_model_matrix = mat4_multiply(&planet2_model_matrix, &planet2_local_rotation);
        
        render_wireframe(my_canvas, planet_model_2, planet2_model_matrix, view_matrix, projection_matrix, &main_light, camera_position);

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
    free_model(sun_model);
    free_model(planet_model_1);
    free_model(planet_model_2);
    canvas_destroy(my_canvas);

    return EXIT_SUCCESS;
}