#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>   // For malloc, free, and qsort
#include <stdbool.h>  // For boolean type

// --- Internal Helper Structs and Functions ---

// A temporary structure to hold a line that's ready to be drawn.
// We'll use this for depth sorting.
typedef struct {
    vec3_t p1_screen; // Projected start point (in screen space)
    vec3_t p2_screen; // Projected end point (in screen space)
    float avg_z;     // Average depth of the line, for sorting
    float intensity; // Intensity after lighting calculation
} line_to_draw_t;

// Comparison function for qsort. Sorts lines from back to front (larger Z to smaller Z).
// This ensures that closer lines are drawn last, correctly overwriting more distant ones
// if depth testing wasn't perfectly handling overlapping lines.
// However, with canvas_put_pixel_f, depth testing is handled at the pixel level.
// This sort is still useful for consistency and potential future optimizations (like early-z reject).
int compare_lines_by_depth(const void* a, const void* b) {
    const line_to_draw_t* line_a = (const line_to_draw_t*)a;
    const line_to_draw_t* line_b = (const line_to_draw_t*)b;
    if (line_a->avg_z < line_b->avg_z) return 1; // 'a' is farther, so sort it before 'b'
    if (line_a->avg_z > line_b->avg_z) return -1; // 'a' is closer, so sort it after 'b'
    return 0;
}

/**
 * @brief Renders a 3D model as a wireframe onto the canvas with lighting and depth testing.
 * @param canvas A pointer to the canvas to draw on.
 * @param model A pointer to the 3D model to render.
 * @param model_matrix The model transformation matrix.
 * @param view_matrix The view transformation matrix.
 * @param projection_matrix The projection matrix.
 * @param light A pointer to the light source for lighting calculations.
 */
void render_wireframe(
    canvas_t* canvas,
    const model_t* model,
    mat4_t model_matrix,
    mat4_t view_matrix,
    mat4_t projection_matrix,
    const light_source_t* light // NEW parameter
) {
    if (!canvas || !model || !model->vertices || !model->faces) { // Check for faces too
        fprintf(stderr, "Error: Invalid canvas or model for rendering.\n");
        return;
    }

    // Combine matrices: Projection * View * Model
    mat4_t mvp_matrix = mat4_multiply(&projection_matrix, &view_matrix);
    mvp_matrix = mat4_multiply(&mvp_matrix, &model_matrix);

    // For lighting calculations, we need the inverse of the model-view matrix
    // to transform normals from model space to view space.
    // Assuming model_matrix is applied first, then view_matrix.
    mat4_t model_view_matrix = mat4_multiply(&view_matrix, &model_matrix);

    // Allocate space for projected points and lines to draw (for depth sorting)
    // We'll store lines for faces, not edges directly.
    line_to_draw_t* lines_to_draw = (line_to_draw_t*)malloc(model->num_faces * 3 * sizeof(line_to_draw_t)); // 3 edges per face
    if (!lines_to_draw) {
        perror("Failed to allocate lines_to_draw buffer");
        return;
    }
    int current_line_count = 0;

    // Iterate through each face to calculate lighting and project vertices
    for (int i = 0; i < model->num_faces; ++i) {
        face_t face = model->faces[i];
        vec3_t v0 = model->vertices[face.v1_index];
        vec3_t v1 = model->vertices[face.v2_index];
        vec3_t v2 = model->vertices[face.v3_index];

        // --- Back-face culling ---
        // Transform vertices to view space for normal calculation and culling
        vec3_t v0_view = mat4_multiply_vec3(&model_view_matrix, v0);
        vec3_t v1_view = mat4_multiply_vec3(&model_view_matrix, v1);
        vec3_t v2_view = mat4_multiply_vec3(&model_view_matrix, v2);

        // Calculate face normal in view space
        vec3_t edge1_view = vec3_subtract(&v1_view, &v0_view);
        vec3_t edge2_view = vec3_subtract(&v2_view, &v0_view);
        vec3_t temp_cross_product = vec3_cross(&edge1_view, &edge2_view); // Store the result of cross product
        vec3_t face_normal_view = vec3_normalize_fast(&temp_cross_product); // Now take the address of the lvalue

        // View direction (from surface to camera) is approximately -vertex_position_in_view_space
        // For orthographic camera or distant objects, it's often simpler to assume (0,0,1) for -Z axis camera
        // For perspective, view_direction = vec3_normalize_fast(&vec3_negate(&v0_view));
        // However, for back-face culling, just check Z component of normal or dot product with camera forward
        // In view space, camera is typically at origin looking down -Z. So normal.z > 0 means facing away.
        if (face_normal_view.z > 0.0f) { // Simple back-face culling: if normal points towards +Z (away from camera)
             continue; // Cull this face
        }

        // --- Lighting Calculation ---
        // Light direction from surface to light source, in view space
        vec3_t light_pos_view = mat4_multiply_vec3(&view_matrix, light->position); // Transform light position to view space
        vec3_t temp_light_dir = vec3_subtract(&light_pos_view, &v0_view); // Store the result of subtraction
        vec3_t light_dir_from_surface_view = vec3_normalize_fast(&temp_light_dir); // Now take the address of the lvalue

        float diffuse_intensity = calculate_diffuse_intensity(&face_normal_view, &light_dir_from_surface_view);
        float line_intensity = diffuse_intensity; // Use diffuse intensity for line color

        // Project vertices of the current face
        vec3_t proj_v0 = mat4_multiply_vec3(&mvp_matrix, v0);
        vec3_t proj_v1 = mat4_multiply_vec3(&mvp_matrix, v1);
        vec3_t proj_v2 = mat4_multiply_vec3(&mvp_matrix, v2);

        // Convert to screen coordinates
        // Remember NDC x, y from -1 to 1, Z from 0 to 1 (or -1 to 1)
        // Screen X = (NDC.x + 1) * 0.5 * width
        // Screen Y = (1 - NDC.y) * 0.5 * height (assuming Y-down screen coords)
        // Screen Z = (NDC.z + 1) * 0.5 (or just use NDC.z if it's already 0-1)

        vec3_t screen_v0 = vec3_create(
            (proj_v0.x + 1.0f) * 0.5f * canvas->width,
            (1.0f - (proj_v0.y + 1.0f) * 0.5f) * canvas->height, // Invert Y for screen coords
            proj_v0.z // Z is already in NDC [0,1] or [-1,1] - keep as is for depth test
        );
        vec3_t screen_v1 = vec3_create(
            (proj_v1.x + 1.0f) * 0.5f * canvas->width,
            (1.0f - (proj_v1.y + 1.0f) * 0.5f) * canvas->height,
            proj_v1.z
        );
        vec3_t screen_v2 = vec3_create(
            (proj_v2.x + 1.0f) * 0.5f * canvas->width,
            (1.0f - (proj_v2.y + 1.0f) * 0.5f) * canvas->height,
            proj_v2.z
        );

        // Store the three edges of the face for depth sorting
        // Edge 1 (v0-v1)
        lines_to_draw[current_line_count].p1_screen = screen_v0;
        lines_to_draw[current_line_count].p2_screen = screen_v1;
        lines_to_draw[current_line_count].avg_z = (proj_v0.z + proj_v1.z) / 2.0f;
        lines_to_draw[current_line_count].intensity = line_intensity;
        current_line_count++;

        // Edge 2 (v1-v2)
        lines_to_draw[current_line_count].p1_screen = screen_v1;
        lines_to_draw[current_line_count].p2_screen = screen_v2;
        lines_to_draw[current_line_count].avg_z = (proj_v1.z + proj_v2.z) / 2.0f;
        lines_to_draw[current_line_count].intensity = line_intensity;
        current_line_count++;

        // Edge 3 (v2-v0)
        lines_to_draw[current_line_count].p1_screen = screen_v2;
        lines_to_draw[current_line_count].p2_screen = screen_v0;
        lines_to_draw[current_line_count].avg_z = (proj_v2.z + proj_v0.z) / 2.0f;
        lines_to_draw[current_line_count].intensity = line_intensity;
        current_line_count++;
    }

    // Sort lines by their average Z-depth (back to front) for correct rendering order
    qsort(lines_to_draw, current_line_count, sizeof(line_to_draw_t), compare_lines_by_depth);

    // Render sorted lines
    for (int i = 0; i < current_line_count; ++i) {
        // Pass the line's calculated intensity and depth to draw_line_f
        // draw_line_f itself will use canvas_put_pixel_f which does the depth testing.
        draw_line_f(
            canvas,
            lines_to_draw[i].p1_screen.x, lines_to_draw[i].p1_screen.y,
            lines_to_draw[i].p2_screen.x, lines_to_draw[i].p2_screen.y,
            1.0f, // Line thickness (can be adjusted)
            lines_to_draw[i].intensity,
            lines_to_draw[i].avg_z // Pass average depth for the line
        );
    }

    free(lines_to_draw);
}