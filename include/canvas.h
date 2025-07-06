#ifndef CANVAS_H
#define CANVAS_H

#include <stdbool.h> // For bool type

// Define a simple structure for our canvas
typedef struct {
    int width;
    int height;
    float **pixels;  // Stores intensity values (0.0 to 1.0) for grayscale
    float **z_buffer; // NEW: Stores depth values (0.0 to 1.0)
} canvas_t;

// Function prototypes

/**
 * @brief Creates a new canvas with the specified dimensions.
 * @param width The width of the canvas.
 * @param height The height of the canvas.
 * @return A pointer to the newly created canvas, or NULL on failure.
 */
canvas_t* canvas_create(int width, int height);

/**
 * @brief Destroys a canvas and frees its allocated memory.
 * @param canvas A pointer to the canvas to destroy.
 */
void canvas_destroy(canvas_t* canvas);

/**
 * @brief Clears the canvas pixels to a specified intensity.
 * @param canvas A pointer to the canvas.
 * @param intensity The intensity value to clear the canvas with (0.0 to 1.0).
 */
void canvas_clear(canvas_t* canvas, float intensity);

/**
 * @brief Clears the depth buffer to a specified depth value (typically 1.0 for 'far').
 * @param canvas A pointer to the canvas.
 * @param depth The depth value to clear the z-buffer with (e.g., 1.0 for far plane).
 */
void canvas_clear_depth(canvas_t* canvas, float depth); // NEW: Function to clear z-buffer

/**
 * @brief Sets the intensity of a pixel at (x, y) with depth testing.
 * Only sets the pixel if the new depth is closer than the existing one.
 * @param canvas A pointer to the canvas.
 * @param x The X-coordinate of the pixel.
 * @param y The Y-coordinate of the pixel.
 * @param intensity The intensity value (0.0 to 1.0).
 * @param depth The depth value of the pixel (0.0 to 1.0).
 */
void canvas_put_pixel_f(canvas_t* canvas, float x, float y, float intensity, float depth); // NEW: For depth testing

/**
 * @brief Draws a line between two floating-point coordinates with a given thickness and intensity.
 * @param canvas A pointer to the canvas.
 * @param x0 The X-coordinate of the start point.
 * @param y0 The Y-coordinate of the start point.
 * @param x1 The X-coordinate of the end point.
 * @param y1 The Y-coordinate of the end point.
 * @param thickness The thickness of the line.
 * @param intensity The intensity value (0.0 to 1.0).
 * @param depth The average depth of the line, for depth testing (passed to canvas_put_pixel_f).
 */
void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness, float intensity, float depth); // MODIFIED: Added depth

/**
 * @brief Saves the canvas content to a PGM (Portable Graymap) file.
 * @param canvas A pointer to the canvas.
 * @param filename The name of the file to save to.
 * @return 0 on success, -1 on failure.
 */
int canvas_save_to_pgm(const canvas_t* canvas, const char* filename);


// No longer needed here as it's a private helper in renderer.c (or can be in math3d.c for generic clipping)
// bool clip_to_circular_viewport(int px, int py, int width, int height);

#endif // CANVAS_H