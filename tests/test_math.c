#include <stdio.h>
#include "../include/math3d.h"
#include "../include/canvas.h"
#include <stdlib.h> 
#include <string.h>

// Cube edges (12 edges connecting the vertices)
static int cube_edges[12][2] = {
    // Back face
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    // Front face
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    // Connecting edges
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

static vec3_t cube_vertices[8];

void print_vertex(const vec3_t* v) {
    printf("(%.2f, %.2f, %.2f) | r=%.2f, θ=%.2f, φ=%.2f\n",
        v->x, v->y, v->z, v->r, v->theta, v->phi);
}

void render_cube_wireframe(canvas_t* canvas, const mat4_t* model_transform, float line_intensity) {
    if (!canvas || !model_transform) return;

    // 1. Define View Matrix (Camera position and orientation)
    // Move the camera back along the Z-axis to see the cubes
    mat4_t view_matrix = mat4_translate(vec3_create(0.0f, 0.0f, -5.0f)); // Adjust as needed for visibility

    // 2. Define Projection Matrix (using mat4_frustum_asymmetric via mat4_perspective)
    float fov_y = PI / 4.0f; // 45 degrees vertical FOV
    float aspect_ratio = (float)canvas->width / canvas->height;
    float near_plane = 0.1f; // Near clipping plane
    float far_plane = 100.0f; // Far clipping plane
    // Use mat4_frustum_asymmetric or mat4_perspective directly
    // Calculate frustum parameters from FOV, aspect ratio, and near/far planes
    float tan_half_fov_y = tanf(fov_y / 2.0f);
    float top = near_plane * tan_half_fov_y;
    float bottom = -top;
    float right = top * aspect_ratio;
    float left = -right;

    mat4_t projection_matrix = mat4_frustum_asymmetric(left, right, bottom, top, near_plane, far_plane);
    // 3. Combine matrices: Projection * View * Model
    // Order: Model -> View -> Projection
    mat4_t view_model_matrix = mat4_multiply(&view_matrix, model_transform);
    mat4_t mvp_matrix = mat4_multiply(&projection_matrix, &view_model_matrix);

    // Transform and project all vertices
    vec3_t transformed_vertices_clip[8]; // Stores points after MVP, before perspective divide
    float w_components[8]; // Store the w-component for perspective divide

    for (int i = 0; i < 8; i++) {
        // Manually calculate homogeneous coordinates and transformed w
        float x_hom = cube_vertices[i].x;
        float y_hom = cube_vertices[i].y;
        float z_hom = cube_vertices[i].z;
        float w_hom = 1.0f; // Assume w=1 for input point

        // Calculate transformed homogeneous coordinates
        float transformed_x = mvp_matrix.m[0] * x_hom + mvp_matrix.m[4] * y_hom + mvp_matrix.m[8] * z_hom + mvp_matrix.m[12] * w_hom;
        float transformed_y = mvp_matrix.m[1] * x_hom + mvp_matrix.m[5] * y_hom + mvp_matrix.m[9] * z_hom + mvp_matrix.m[13] * w_hom;
        float transformed_z = mvp_matrix.m[2] * x_hom + mvp_matrix.m[6] * y_hom + mvp_matrix.m[10] * z_hom + mvp_matrix.m[14] * w_hom;
        float transformed_w = mvp_matrix.m[3] * x_hom + mvp_matrix.m[7] * y_hom + mvp_matrix.m[11] * z_hom + mvp_matrix.m[15] * w_hom;

        transformed_vertices_clip[i] = vec3_create(transformed_x, transformed_y, transformed_z);
        w_components[i] = transformed_w;
    }

    // Map to canvas coordinates and draw edges
    float half_width = canvas->width / 2.0f;
    float half_height = canvas->height / 2.0f;

    for (int i = 0; i < 12; i++) {
        int v1_idx = cube_edges[i][0];
        int v2_idx = cube_edges[i][1];

        // Perform perspective divide to get NDC
        vec3_t ndc_v1, ndc_v2;
        if (w_components[v1_idx] != 0.0f) {
            ndc_v1 = vec3_create(transformed_vertices_clip[v1_idx].x / w_components[v1_idx],
                                 transformed_vertices_clip[v1_idx].y / w_components[v1_idx],
                                 transformed_vertices_clip[v1_idx].z / w_components[v1_idx]);
        } else { /* Handle division by zero if necessary, e.g., set to infinity or clamp */ ndc_v1 = vec3_create(0,0,0); }
        if (w_components[v2_idx] != 0.0f) {
            ndc_v2 = vec3_create(transformed_vertices_clip[v2_idx].x / w_components[v2_idx],
                                 transformed_vertices_clip[v2_idx].y / w_components[v2_idx],
                                 transformed_vertices_clip[v2_idx].z / w_components[v2_idx]);
        } else { /* Handle division by zero */ ndc_v2 = vec3_create(0,0,0); }


        // Convert Normalized Device Coordinates (NDC, typically -1 to 1) to screen coordinates (0 to width/height)
        // Note: Y-axis inversion is common for screen coordinates (origin at top-left)
        float x0_screen = (ndc_v1.x * half_width) + half_width;
        float y0_screen = (-ndc_v1.y * half_height) + half_height;
        float x1_screen = (ndc_v2.x * half_width) + half_width;
        float y1_screen = (-ndc_v2.y * half_height) + half_height;

        // Draw the edge using draw_line_f
        float line_intensity = 1.0f;
        float line_thickness = 5.0f;
        float line_depth = 0.0f; // Added a placeholder for depth. Adjust this value as needed for your scene's depth.

        draw_line_f(canvas, x0_screen, y0_screen, x1_screen, y1_screen, line_thickness, line_intensity, line_depth);
    }
}

int main() {
    // Create a canvas for the visual demo
    int canvas_width = 800;
    int canvas_height = 600;
    canvas_t* canvas = canvas_create(canvas_width, canvas_height);
    if (!canvas) {
        fprintf(stderr, "Failed to create canvas\n");
        return EXIT_FAILURE;
    }

    // Initialize cube_vertices properly using vec3_create (as discussed before)
    cube_vertices[0] = vec3_create(-0.5f, -0.5f, -0.5f);
    cube_vertices[1] = vec3_create( 0.5f, -0.5f, -0.5f);
    cube_vertices[2] = vec3_create( 0.5f,  0.5f, -0.5f);
    cube_vertices[3] = vec3_create(-0.5f,  0.5f, -0.5f);
    cube_vertices[4] = vec3_create(-0.5f, -0.5f,  0.5f);
    cube_vertices[5] = vec3_create( 0.5f, -0.5f,  0.5f);
    cube_vertices[6] = vec3_create( 0.5f,  0.5f,  0.5f);
    cube_vertices[7] = vec3_create(-0.5f,  0.5f,  0.5f);


    printf("Task 2: Demo\n");

    // Test Vector Operations (Keep these tests from your original main if you want them printed)
    printf("\n=== Vector Operations Tests ===\n");
    // Example: vec3_create and print
    vec3_t v1 = vec3_create(1.0f, 2.0f, 3.0f);
    printf("v1 (Cartesian): "); print_vertex(&v1);

    // Example: vec3_from_spherical
    vec3_t v_spherical = vec3_from_spherical(5.0f, PI/4, PI/3);
    printf("v_spherical: "); print_vertex(&v_spherical);

    // Example: vec3_normalize_fast
    vec3_t v1_normalized = vec3_normalize_fast(&v1);
    printf("v1_normalized (fast): "); print_vertex(&v1_normalized);

    // Example: vec3_slerp
    vec3_t temp_dir1 = vec3_create(1.0f, 0.0f, 0.0f);
    vec3_t dir1 = vec3_normalize_fast(&temp_dir1);
    vec3_t temp_dir2 = vec3_create(0.0f, 1.0f, 0.0f);
    vec3_t dir2 = vec3_normalize_fast(&temp_dir2);
    vec3_t slerp_result = vec3_slerp(&dir1, &dir2, 0.5f);
    printf("SLERP (0.5) from (1,0,0) to (0,1,0): "); print_vertex(&slerp_result);

    // Test Matrix Operations (Keep these tests from your original main if you want them printed)
    printf("\n=== Matrix Operations Tests ===\n");
    mat4_t scale_mat = mat4_scale(vec3_create(2.0f, 0.5f, 1.0f));
    printf("Scale Matrix:\n");
    for (int i=0; i<16; ++i) { printf("%8.2f%s", scale_mat.m[i], (i+1)%4==0?"\n":""); }

    mat4_t rot_xyz_mat = mat4_rotate_xyz(PI/6, PI/4, PI/3);
    printf("Rotate XYZ Matrix:\n");
    for (int i=0; i<16; ++i) { printf("%8.2f%s", rot_xyz_mat.m[i], (i+1)%4==0?"\n":""); }

    //Animation Loop
    int num_frames = 60; // Number of frames for the animation
    float rotation_speed = TWO_PI / num_frames; // Rotate a full circle over num_frames

    // Buffer for unique filenames
    char filename[256];

    mat4_t base_translate = mat4_translate(vec3_create(0.0f, 0.0f, 0.0f)); // Cube initially at origin, camera will be moved back in render_cube_wireframe

    printf("Visual Demo\n");

    for (int frame = 0; frame < num_frames; ++frame) {
        // Clear the canvas for the new frame to black (0.0f intensity)
        canvas_clear(canvas, 0.0f);

        // Calculate the current rotation angles based on the frame number
        float current_angle_y = frame * rotation_speed;
        float current_angle_x = frame * rotation_speed * 0.5f; // Slower rotation on X for more interesting motion

        // Create a model transformation matrix for the rotating cube
        // Rotate around X and Y axes
        mat4_t rotation_matrix = mat4_rotate_xyz(current_angle_x, current_angle_y, 0.0f);

        // Combine base translation and rotation for the model transform
        // Order: Scale (if any) -> Rotate -> Translate
        // Here, it's Rotate then Translate (base_translate is the final position of the model)
        mat4_t model_transform = mat4_multiply(&base_translate, &rotation_matrix);

        // Render the single cube with its dynamic model transform
        render_cube_wireframe(canvas, &model_transform, 1.0f); // Render at full intensity

        // Save each frame to a unique PGM file
        sprintf(filename, "frames/cube_frame_%03d.pgm", frame);
        if (canvas_save_to_pgm(canvas, filename) == 0) {
            printf("Saved frame %d to %s\n", frame, filename);
        } else {
            fprintf(stderr, "Failed to save frame %d.\n", frame);
        }
    }

    // Animation frames saved to frames dir.

    // Clean up canvas
    canvas_destroy(canvas);

    printf("\nTask 2: Demo Complete\n");
    return EXIT_SUCCESS;
}