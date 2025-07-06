#ifndef RENDERER_H
#define RENDERER_H

#include "canvas.h"
#include "math3d.h"
#include "lighting.h" // NEW: Include lighting.h for light_source_t

// A simple struct to represent an edge connecting two vertices by their indices
typedef struct {
    int v1_index;
    int v2_index;
} edge_t;

// A simple struct to represent a face using three vertex indices
typedef struct {
    int v1_index;
    int v2_index;
    int v3_index;
} face_t;

// A simple struct to hold our 3D model data
typedef struct {
    vec3_t* vertices;     // Dynamic array of vertices
    edge_t* edges;        // Dynamic array of edges
    face_t* faces;        // Dynamic array of faces (new!)
    int num_vertices;
    int num_edges;
    int num_faces;        // Number of faces (new!)
} model_t;

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
    const light_source_t* light // NEW: Pass light source for basic lighting
);

#endif // RENDERER_H