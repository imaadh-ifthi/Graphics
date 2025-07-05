#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <math.h>
#include <string.h> // For sprintf

// Include your custom headers
#include "canvas.h"
#include "math3d.h" // Your math3d.h is crucial here
#include "renderer.h"

// Define a common constant for converting degrees to radians
// (Already defined in math3d.h, but no harm in keeping this safeguard)
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI / 180.0f) // Use PI from math3d.h
#endif

// --- Model Utility Functions ---

// Function to generate a simple cube model for testing
// model_t* generate_cube(void) {
//     model_t* cube = (model_t*)malloc(sizeof(model_t));
//     if (!cube) {
//         perror("Failed to allocate model_t for cube");
//         return NULL;
//     }

//     cube->num_vertices = 8;
//     cube->vertices = (vec3_t*)malloc(cube->num_vertices * sizeof(vec3_t));
//     if (!cube->vertices) {
//         perror("Failed to allocate vertices for cube");
//         free(cube);
//         return NULL;
//     }

//     // Define vertices of a unit cube centered at origin
//     cube->vertices[0] = vec3_create(-0.5f, -0.5f, -0.5f); // 0: FBL (Front Bottom Left)
//     cube->vertices[1] = vec3_create( 0.5f, -0.5f, -0.5f); // 1: FBR (Front Bottom Right)
//     cube->vertices[2] = vec3_create( 0.5f,  0.5f, -0.5f); // 2: FTR (Front Top Right)
//     cube->vertices[3] = vec3_create(-0.5f,  0.5f, -0.5f); // 3: FTL (Front Top Left)
//     cube->vertices[4] = vec3_create(-0.5f, -0.5f,  0.5f); // 4: BBL (Back Bottom Left)
//     cube->vertices[5] = vec3_create( 0.5f, -0.5f,  0.5f); // 5: BBR (Back Bottom Right)
//     cube->vertices[6] = vec3_create( 0.5f,  0.5f,  0.5f); // 6: BTR (Back Top Right)
//     cube->vertices[7] = vec3_create(-0.5f,  0.5f,  0.5f); // 7: BTL (Back Top Left)

//     cube->num_edges = 12;
//     cube->edges = (edge_t*)malloc(cube->num_edges * sizeof(edge_t));
//     if (!cube->edges) {
//         perror("Failed to allocate edges for cube");
//         free(cube->vertices);
//         free(cube);
//         return NULL;
//     }

//     // Define edges (pairs of vertex indices)
//     // Front face
//     cube->edges[0] = (edge_t){0, 1};
//     cube->edges[1] = (edge_t){1, 2};
//     cube->edges[2] = (edge_t){2, 3};
//     cube->edges[3] = (edge_t){3, 0};
//     // Back face
//     cube->edges[4] = (edge_t){4, 5};
//     cube->edges[5] = (edge_t){5, 6};
//     cube->edges[6] = (edge_t){6, 7};
//     cube->edges[7] = (edge_t){7, 4};
//     // Connecting edges
//     cube->edges[8] = (edge_t){0, 4};
//     cube->edges[9] = (edge_t){1, 5};
//     cube->edges[10] = (edge_t){2, 6};
//     cube->edges[11] = (edge_t){3, 7};

//     printf("Generated cube model with %d vertices and %d edges.\n", cube->num_vertices, cube->num_edges);
//     return cube;
// }

// Function to generate a simple soccer ball (icosahedron-based) model for testing
model_t* generate_soccer_ball(void) {
    model_t* ball = (model_t*)malloc(sizeof(model_t));
    if (!ball) {
        perror("Failed to allocate model_t for soccer ball");
        return NULL;
    }

    // Vertices of a regular Icosahedron (20 faces, 12 vertices, 30 edges)
    // Scaled to a reasonable size, e.g., radius 0.5
    // These are derived from a golden ratio (phi) construction
    float phi = (1.0f + sqrtf(5.0f)) / 2.0f; // Golden ratio
    float r = 0.5f; // Radius, adjust as needed for visibility
    float a = r;
    float b = r / phi;

    ball->num_vertices = 12;
    ball->vertices = (vec3_t*)malloc(ball->num_vertices * sizeof(vec3_t));
    if (!ball->vertices) {
        perror("Failed to allocate vertices for soccer ball");
        free(ball);
        return NULL;
    }

    // 12 vertices of an icosahedron
    ball->vertices[0] = vec3_create(0.0f, a, b);
    ball->vertices[1] = vec3_create(0.0f, a, -b);
    ball->vertices[2] = vec3_create(0.0f, -a, b);
    ball->vertices[3] = vec3_create(0.0f, -a, -b);
    ball->vertices[4] = vec3_create(a, b, 0.0f);
    ball->vertices[5] = vec3_create(a, -b, 0.0f);
    ball->vertices[6] = vec3_create(-a, b, 0.0f);
    ball->vertices[7] = vec3_create(-a, -b, 0.0f);
    ball->vertices[8] = vec3_create(b, 0.0f, a);
    ball->vertices[9] = vec3_create(b, 0.0f, -a);
    ball->vertices[10] = vec3_create(-b, 0.0f, a);
    ball->vertices[11] = vec3_create(-b, 0.0f, -a);

    ball->num_edges = 30; // 30 edges for a regular icosahedron
    ball->edges = (edge_t*)malloc(ball->num_edges * sizeof(edge_t));
    if (!ball->edges) {
        perror("Failed to allocate edges for soccer ball");
        free(ball->vertices);
        free(ball);
        return NULL;
    }

    // 30 edges connecting the vertices to form the icosahedron wireframe
    ball->edges[0] = (edge_t){0, 4};
    ball->edges[1] = (edge_t){0, 6};
    ball->edges[2] = (edge_t){0, 8};
    ball->edges[3] = (edge_t){0, 10};
    ball->edges[4] = (edge_t){1, 4};
    ball->edges[5] = (edge_t){1, 6};
    ball->edges[6] = (edge_t){1, 9};
    ball->edges[7] = (edge_t){1, 11};
    ball->edges[8] = (edge_t){2, 5};
    ball->edges[9] = (edge_t){2, 7};
    ball->edges[10] = (edge_t){2, 8};
    ball->edges[11] = (edge_t){2, 10};
    ball->edges[12] = (edge_t){3, 5};
    ball->edges[13] = (edge_t){3, 7};
    ball->edges[14] = (edge_t){3, 9};
    ball->edges[15] = (edge_t){3, 11};
    ball->edges[16] = (edge_t){4, 5};
    ball->edges[17] = (edge_t){4, 9};
    ball->edges[18] = (edge_t){5, 8};
    ball->edges[19] = (edge_t){6, 7};
    ball->edges[20] = (edge_t){6, 11};
    ball->edges[21] = (edge_t){7, 10};
    ball->edges[22] = (edge_t){8, 9};
    ball->edges[23] = (edge_t){8, 10};
    ball->edges[24] = (edge_t){9, 11};
    ball->edges[25] = (edge_t){10, 11};
    // Additional edges to complete the icosahedron, ensuring all vertices have 5 connections
    ball->edges[26] = (edge_t){0, 1}; // Connects top points
    ball->edges[27] = (edge_t){0, 2}; // Connects top to bottom pentagon
    ball->edges[28] = (edge_t){1, 3}; // Connects top to bottom pentagon
    ball->edges[29] = (edge_t){2, 3}; // Connects bottom points

    printf("Generated soccer ball model with %d vertices and %d edges.\n", ball->num_vertices, ball->num_edges);
    return ball;
}

// Function to destroy a model and free its allocated memory
void destroy_model(model_t* model) {
    if (model) {
        if (model->vertices) {
            free(model->vertices);
            model->vertices = NULL;
        }
        if (model->edges) {
            free(model->edges);
            model->edges = NULL;
        }
        free(model);
        model = NULL;
    }
}


// --- Main Demo Function ---

int main(){
    printf("--- main2.c: Starting Task 3 Demo ---\n"); // Debug print

    int width = 600;
    int height = 600;

    printf("main2.c: Creating canvas...\n"); // Debug print
    canvas_t* my_canvas = canvas_create(width, height);
    if(!my_canvas){
        fprintf(stderr, "main2.c: Failed to create canvas in main.\n"); // Debug print
        return 1;
    }
    printf("main2.c: Canvas created.\n"); // Debug print

    // Generate the 3D model (changed to generate_soccer_ball())
    model_t* my_model = generate_soccer_ball(); // <-- Now exclusively calls generate_soccer_ball()
    if (!my_model) {
        fprintf(stderr, "main2.c: Failed to generate 3D model.\n"); // Debug print
        canvas_destroy(my_canvas);
        return 1;
    }
    printf("main2.c: Model generated.\n"); // Debug print

    // --- Set up Camera and Projection ---
    // View Matrix: Position the camera. Your mat4_translate returns a mat4_t by value.
    mat4_t view_matrix = mat4_translate(0.0f, 0.0f, -3.0f); // Correct for your math3d.h

    // Projection Matrix: Define the viewing frustum.
    float aspect_ratio = (float)width / height;
    float fov_y_rad = 60.0f * DEG_TO_RAD; // 60 degrees Field of View in Y direction
    float near_plane = 0.1f;             // Near clipping plane
    float far_plane = 100.0f;            // Far clipping plane

    // Calculate frustum parameters
    float top_val = tanf(fov_y_rad / 2.0f) * near_plane;
    float bottom_val = -top_val;
    float right_val = top_val * aspect_ratio;
    float left_val = -right_val;

    // Your mat4_frustum_asymmetric returns a mat4_t by value.
    mat4_t projection_matrix = mat4_frustum_asymmetric(left_val, right_val, bottom_val, top_val, near_plane, far_plane);

    // --- Animation Loop ---
    int num_frames = 60; // For a smooth rotation (e.g., 2 seconds at 30fps)
    float rotation_speed_degrees_per_frame = 360.0f / num_frames; // Full rotation in one loop

    printf("main2.c: Starting animation rendering for %d frames...\n", num_frames); // Debug print
    for (int frame = 0; frame < num_frames; ++frame) {
        printf("main2.c: Rendering frame %d...\n", frame); // Debug print per frame
        canvas_clear(my_canvas, 0.0f); // Clear canvas to black for each frame

        float angle_rad = frame * rotation_speed_degrees_per_frame * DEG_TO_RAD;

        // Model Matrix: Translate, Scale, Rotate the object.
        // Start with identity. Your mat4_identity returns a mat4_t by value.
        mat4_t model_matrix = mat4_identity();

        // Create the rotation matrix. Your mat4_rotate_xyz returns a mat4_t by value.
        mat4_t rotation_matrix = mat4_rotate_xyz(0.0f, angle_rad, 0.0f);

        // Multiply the current model_matrix by the rotation_matrix.
        // Your mat4_multiply takes pointers to const mat4_t and returns a mat4_t by value.
        model_matrix = mat4_multiply(&model_matrix, &rotation_matrix);

        // Render the wireframe for the current frame
        render_wireframe(my_canvas, my_model, model_matrix, view_matrix, projection_matrix);

        // Save frame to PGM
        char filename[256];
        sprintf(filename, "frames/frame_%04d.pgm", frame); // Ensure this path matches your 'frames' directory location
        if (canvas_save_to_pgm(my_canvas, filename) != 0) {
            fprintf(stderr, "main2.c: Failed to save frame %d to %s.\n", frame, filename); // Debug print
            break;
        }
        printf("main2.c: Frame %d saved to %s.\n", frame, filename); // Debug print
    }

    printf("main2.c: Animation rendering finished. Cleaning up...\n"); // Debug print
    // Clean up allocated memory for the model and canvas
    destroy_model(my_model);
    canvas_destroy(my_canvas);

    printf("--- main2.c: Demo finished. ---\n"); // Debug print

    return 0;
}