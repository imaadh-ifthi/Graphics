#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h> // For fminf/fmaxf

// --- Internal Helper Structs and Functions ---

typedef struct { vec3_t p1_screen, p2_screen; float avg_z, intensity; } line_to_draw_t;
typedef struct { float x, y, z, w; } vec4_t;

int compare_lines_by_depth(const void* a, const void* b) {
    const line_to_draw_t* la = (const line_to_draw_t*)a;
    const line_to_draw_t* lb = (const line_to_draw_t*)b;
    if (la->avg_z < lb->avg_z) return 1;
    if (la->avg_z > lb->avg_z) return -1;
    return 0;
}

// Correctly transforms a 3D point using a 4x4 matrix.
static vec3_t transform_point(const mat4_t* m, const vec3_t* v) {
    vec3_t result;
    result.x = v->x * m->m[0] + v->y * m->m[4] + v->z * m->m[8]  + m->m[12];
    result.y = v->x * m->m[1] + v->y * m->m[5] + v->z * m->m[9]  + m->m[13];
    result.z = v->x * m->m[2] + v->y * m->m[6] + v->z * m->m[10] + m->m[14];
    return result;
}

// Correctly transforms a 3D direction vector using a 4x4 matrix (ignores translation).
static vec3_t transform_direction(const mat4_t* m, const vec3_t* v) {
    vec3_t result;
    result.x = v->x * m->m[0] + v->y * m->m[4] + v->z * m->m[8];
    result.y = v->x * m->m[1] + v->y * m->m[5] + v->z * m->m[9];
    result.z = v->x * m->m[2] + v->y * m->m[6] + v->z * m->m[10];
    return result;
}

// --- Main Rendering Function ---

void render_wireframe(canvas_t* canvas, const model_t* model, mat4_t model_matrix, mat4_t view_matrix, mat4_t projection_matrix, const light_source_t* light, vec3_t camera_pos_world) {
    if (!canvas || !model || !light) return;

    mat4_t mv_matrix = mat4_multiply(&view_matrix, &model_matrix);
    mat4_t mvp_matrix = mat4_multiply(&projection_matrix, &mv_matrix);
    
    line_to_draw_t lines_to_draw[model->num_faces * 3];
    int current_line_count = 0;
    //const float AMBIENT_LIGHT = 0.1f;

    for (int i = 0; i < model->num_faces; ++i) {
        face_t face = model->faces[i];
        vec3_t v0_model = model->vertices[face.v1_index];
        vec3_t v1_model = model->vertices[face.v2_index];
        vec3_t v2_model = model->vertices[face.v3_index];

        // --- Back-face culling ---
        vec3_t v0_world = transform_point(&model_matrix, &v0_model);
        
        vec3_t edge1 = vec3_subtract(&v1_model, &v0_model);
        vec3_t edge2 = vec3_subtract(&v2_model, &v0_model);

        // FIX: Store the cross product result before normalizing to fix the lvalue error.
        vec3_t cross_product = vec3_cross(&edge1, &edge2);
        vec3_t face_normal_model = vec3_normalize_fast(&cross_product);

        vec3_t face_normal_world = transform_direction(&model_matrix, &face_normal_model);
        
        vec3_t vector_to_camera = vec3_subtract(&camera_pos_world, &v0_world);
        vec3_t view_dir_world = vec3_normalize_fast(&vector_to_camera);
        
        if (vec3_dot(&face_normal_world, &view_dir_world) <= 0.0f) {
            continue;
        }

        // --- Vertex Transformation to Clip Space (Manual) ---
        vec4_t clip_v[3];
        const vec3_t* model_v[] = {&v0_model, &v1_model, &v2_model};
        for(int j = 0; j < 3; ++j) {
            clip_v[j].x = model_v[j]->x * mvp_matrix.m[0] + model_v[j]->y * mvp_matrix.m[4] + model_v[j]->z * mvp_matrix.m[8]  + mvp_matrix.m[12];
            clip_v[j].y = model_v[j]->x * mvp_matrix.m[1] + model_v[j]->y * mvp_matrix.m[5] + model_v[j]->z * mvp_matrix.m[9]  + mvp_matrix.m[13];
            clip_v[j].z = model_v[j]->x * mvp_matrix.m[2] + model_v[j]->y * mvp_matrix.m[6] + model_v[j]->z * mvp_matrix.m[10] + mvp_matrix.m[14];
            clip_v[j].w = model_v[j]->x * mvp_matrix.m[3] + model_v[j]->y * mvp_matrix.m[7] + model_v[j]->z * mvp_matrix.m[11] + mvp_matrix.m[15];
        }

        // --- Clipping & Viewport Transformation ---
        vec3_t screen_v[3];
        bool all_verts_valid = true;
        for(int j = 0; j < 3; ++j) {
            if (clip_v[j].w < 0.1f) { all_verts_valid = false; break; }
            float inv_w = 1.0f / clip_v[j].w;
            vec3_t ndc_v = vec3_create(clip_v[j].x * inv_w, clip_v[j].y * inv_w, clip_v[j].z * inv_w);            screen_v[j].x = (ndc_v.x + 1.0f) * 0.5f * canvas->width;
            screen_v[j].y = (1.0f - (ndc_v.y + 1.0f) * 0.5f) * canvas->height;
            screen_v[j].z = (ndc_v.z + 1.0f) * 0.5f;
        }
        if (!all_verts_valid) continue;
        
        // --- Lighting Calculation ---
        vec3_t vector_to_light = vec3_subtract(&light->position, &v0_world);
        vec3_t light_dir_from_face = vec3_normalize_fast(&vector_to_light);
        float diffuse = calculate_diffuse_intensity(&face_normal_world, &light_dir_from_face);
        float line_intensity = fminf(1.0f, AMBIENT_LIGHT + diffuse);

        // --- Store edges for drawing ---
        if (current_line_count + 3 > model->num_faces * 3) break;
        lines_to_draw[current_line_count++] = (line_to_draw_t){screen_v[0], screen_v[1], (screen_v[0].z + screen_v[1].z) * 0.5f, line_intensity};
        lines_to_draw[current_line_count++] = (line_to_draw_t){screen_v[1], screen_v[2], (screen_v[1].z + screen_v[2].z) * 0.5f, line_intensity};
        lines_to_draw[current_line_count++] = (line_to_draw_t){screen_v[2], screen_v[0], (screen_v[2].z + screen_v[0].z) * 0.5f, line_intensity};
    }

    qsort(lines_to_draw, current_line_count, sizeof(line_to_draw_t), compare_lines_by_depth);

    for (int i = 0; i < current_line_count; ++i) {
        line_to_draw_t* line = &lines_to_draw[i];
        draw_line_f(canvas, line->p1_screen.x, line->p1_screen.y, line->p2_screen.x, line->p2_screen.y, 1.0f, line->intensity, line->avg_z);
    }
}