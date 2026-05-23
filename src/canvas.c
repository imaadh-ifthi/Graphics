#include "../include/canvas.h"
#include <stdio.h>    // For standard I/O Operations
#include <stdlib.h>   // For memory allocation (malloc, free)
#include <math.h>     // For math functions (fabsf, roundf, fmaxf, fminf)
#include <string.h>   // For memset, if used for initialization

// Helper function to clamp float values to [min_value, max_value]
static float clamp_float(float value, float min_value, float max_value) {
    if (value > max_value) return max_value;
    if (value < min_value) return min_value;
    return value;
}

// Function to create a new canvas
canvas_t* canvas_create(int width, int height) {
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Canvas dimensions must be positive.\n");
        return NULL;
    }

    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    if (!canvas) {
        perror("Error allocating canvas structure");
        return NULL;
    }
    canvas->width = width;
    canvas->height = height;

    // Allocate memory for pixel rows (intensities)
    canvas->pixels = (float**)malloc(height * sizeof(float*));
    if (!canvas->pixels) {
        perror("Error allocating pixel rows");
        free(canvas);
        return NULL;
    }

    // Allocate memory for z-buffer rows (depths) - NEW
    canvas->z_buffer = (float**)malloc(height * sizeof(float*));
    if (!canvas->z_buffer) {
        perror("Error allocating z_buffer rows");
        for (int i = 0; i < height; ++i) {
            if (canvas->pixels[i]) free(canvas->pixels[i]);
        }
        free(canvas->pixels);
        free(canvas);
        return NULL;
    }

    // Allocate memory for each row's pixels and z-buffer values
    for (int i = 0; i < height; ++i) {
        canvas->pixels[i] = (float*)malloc(width * sizeof(float));
        canvas->z_buffer[i] = (float*)malloc(width * sizeof(float)); // NEW
        if (!canvas->pixels[i] || !canvas->z_buffer[i]) { // NEW: Check both allocations
            perror("Error allocating pixel/z_buffer column");
            // Clean up already allocated rows
            for (int j = 0; j <= i; ++j) {
                if (canvas->pixels[j]) free(canvas->pixels[j]);
                if (canvas->z_buffer[j]) free(canvas->z_buffer[j]); // NEW
            }
            free(canvas->pixels);
            free(canvas->z_buffer); // NEW
            free(canvas);
            return NULL;
        }
    }

    // Initialize canvas and z-buffer (optional, but good practice)
    canvas_clear(canvas, 0.0f); // Clear pixels to black
    canvas_clear_depth(canvas, 1.0f); // Clear z-buffer to 'far' (1.0) - NEW

    return canvas;
}

// Function to destroy a canvas
void canvas_destroy(canvas_t* canvas) {
    if (canvas) {
        if (canvas->pixels) {
            for (int i = 0; i < canvas->height; ++i) {
                if (canvas->pixels[i]) {
                    free(canvas->pixels[i]);
                }
            }
            free(canvas->pixels);
        }
        if (canvas->z_buffer) { // NEW: Free z-buffer memory
            for (int i = 0; i < canvas->height; ++i) {
                if (canvas->z_buffer[i]) {
                    free(canvas->z_buffer[i]);
                }
            }
            free(canvas->z_buffer);
        }
        free(canvas);
    }
}

// Function to clear canvas pixels
void canvas_clear(canvas_t* canvas, float intensity) {
    if (!canvas || !canvas->pixels) {
        fprintf(stderr, "Error: Canvas not initialized for clearing pixels.\n");
        return;
    }
    // Clamp intensity to [0.0, 1.0]
    intensity = clamp_float(intensity, 0.0f, 1.0f);

    for (int y = 0; y < canvas->height; ++y) {
        for (int x = 0; x < canvas->width; ++x) {
            canvas->pixels[y][x] = intensity;
        }
    }
}

// Function to clear the depth buffer - NEW
void canvas_clear_depth(canvas_t* canvas, float depth) {
    if (!canvas || !canvas->z_buffer) {
        fprintf(stderr, "Error: Canvas not initialized for clearing depth buffer.\n");
        return;
    }
    // Clamp depth to [0.0, 1.0]
    depth = clamp_float(depth, 0.0f, 1.0f);

    for (int y = 0; y < canvas->height; ++y) {
        for (int x = 0; x < canvas->width; ++x) {
            canvas->z_buffer[y][x] = depth;
        }
    }
}

// Function to set a pixel with depth testing - NEW/MODIFIED
void canvas_put_pixel_f(canvas_t* canvas, float x, float y, float intensity, float depth) {
    // Round to nearest integer coordinates
    int px = (int)roundf(x);
    int py = (int)roundf(y);

    // Check bounds
    if (px < 0 || px >= canvas->width || py < 0 || py >= canvas->height) {
        return; // Pixel is out of bounds
    }

    // Clamp intensity and depth
    intensity = clamp_float(intensity, 0.0f, 1.0f);
    depth = clamp_float(depth, 0.0f, 1.0f);

    // Depth test: Only draw if the new pixel is closer than the one already there
    // Remember: Smaller Z means closer in NDC (Normalized Device Coordinates)
    if (depth < canvas->z_buffer[py][px]) {
        canvas->pixels[py][px] = intensity;
        canvas->z_buffer[py][px] = depth; // Update depth buffer
    }
}

// Function to draw a line using Bresenham-like algorithm for floating points
// This now takes an additional `depth` parameter
void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness, float intensity, float depth) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float steps = fmaxf(fabsf(dx), fabsf(dy)); // Determine number of steps

    if (steps == 0.0f) { // Points are identical, draw a single pixel/dot
        canvas_put_pixel_f(canvas, x0, y0, intensity, depth); // Use the new function
        return;
    }

    float x_increment = dx / steps;
    float y_increment = dy / steps;

    // Line thickness handling
    float half_thick = thickness / 2.0f;

    for (float i = 0; i <= steps; ++i) {
        float current_x = x0 + i * x_increment;
        float current_y = y0 + i * y_increment;

        if (thickness <= 1.0f) {
            canvas_put_pixel_f(canvas, current_x, current_y, intensity, depth); // Use the new function
        } else {
            // Draw a square of pixels for thicker lines
            for (float ty = -half_thick; ty <= half_thick; ty += 1.0f) {
                for (float tx = -half_thick; tx <= half_thick; tx += 1.0f) {
                    canvas_put_pixel_f(canvas, current_x + tx, current_y + ty, intensity, depth); // Use the new function
                }
            }
        }
    }
}


// Function to save canvas to PGM
int canvas_save_to_pgm(const canvas_t* canvas, const char* filename) {
    if (!canvas || !canvas->pixels) {
        fprintf(stderr, "Error: Canvas not initialized or no pixel data available to save.\n");
        return -1;
    }

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Error opening file for PGM export");
        return -1;
    }

    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", canvas->width, canvas->height);
    fprintf(fp, "255\n"); // Max intensity value for grayscale

    for (int y = 0; y < canvas->height; ++y) {
        for (int x = 0; x < canvas->width; ++x) {
            // Scale float intensity (0.0-1.0) to int (0-255)
            int gray_value = (int)roundf(canvas->pixels[y][x] * 255.0f);
            fprintf(fp, "%d%c", gray_value, (x == canvas->width - 1) ? '\n' : ' ');
        }
    }

    fclose(fp);
    return 0;
}