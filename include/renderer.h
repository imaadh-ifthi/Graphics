#ifndef RENDERER_H
#define RENDERER_H

#include "canvas.h"
#include "math3d.h"

// A simple struct to represent an edge connecting two vertices by their indices
typedef struct {
    int v1_index;
    int v2_index;
} edge_t;

// A simple struct to hold our 3D model data
typedef struct {
    vec3_t* vertices;     // Dynamic array of vertices
    edge_t* edges;        // Dynamic array of edges
    int num_vertices;
    int num_edges;
} model_t;


// Renders a 3D model as a wireframe onto the canvas.
// This is the main function for Stage 3.
void render_wireframe(
    canvas_t* canvas, 
    const model_t* model, 
    mat4_t model_matrix, 
    mat4_t view_matrix, 
    mat4_t projection_matrix
);

#endif // RENDERER_H
