#include "../include/canvas.h" //includeing our header file
#include <stdio.h> //for standard IO Operations
#include <stdlib.h> //for memory allocation
#include <math.h> //for math functions
#include <renderer.h>

//function to clamp float values
float clamp_float(float value, float min_value, float max_value) {
    if (value > max_value) return max_value;
    if (value < min_value) return min_value;
    return value;
}

//canvas_create function defintion
canvas_t* canvas_create(int width, int height){
    if (width <= 0 || height <=0) { //handling error or negative values
        fprintf(stderr, "Error: Canvas dimensions must be positive.\n"); //print an err message to stderr stream- usually console
        return NULL;
    }

    //Allocate memory for the canvas_t structure itself
    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    if (!canvas){ //check if malloc failed (NULL is returned)
        perror("Error allocating canvas structure");
        return NULL;
    }
    canvas->width = width;
    canvas->height = height;

    //Allocate memory for the array of row pointers (canvas->pixels)
    //height number of float pointers required
    canvas->pixels = (float**)malloc(height * sizeof(float*));
    if(!canvas->pixels){
        perror("Error allocating pixel rows"); 
        free(canvas); //free allocated canvas struct
        return NULL;
    }

    //Allocate memory for each row and initialise
    for (int i = 0; i < height; i++){
        //for each row i, allocate width number of floats
        //calloc initializes memory to zero --> zero = black colour
        canvas->pixels[i] = (float*)calloc(width,sizeof(float));
        if (!canvas->pixels[i]) {
            perror("Error allocating pixel columns for a row");
            // if allocation for a row fails we need to free all previously allocated memory
            for (int j = 0; j < i; ++j){
                free(canvas->pixels[j]);
            }
            free(canvas->pixels); //free the array of row pointers
            free(canvas);
            return NULL;
        }
    }
    printf("Canvas created successfully (%dx%d).\n", width, height);
    return canvas;
}

//Defining function canvas_destroy
void canvas_destroy(canvas_t* canvas){
    if(!canvas) {
        return; //if canvas is already destroyed/NULL is returned, just return
    }

    if (canvas->pixels){
        //free the memory for reach row
        for (int i = 0; i < canvas ->height; ++i) {
            //free the memory for the ith row of floats
            free(canvas->pixels[i]);
        }
        //free the memory for the array of row pointers
        free(canvas->pixels);
        
    }
    free(canvas);
    printf("Canvas destroyed.\n");

}

//pixel function definition
// void set_pixel_f(canvas_t* canvas, float x, float y, float intensity) {
//     //error handling for uninitialised canavs
//     if(!canvas || !canvas->pixels) {
//         fprintf(stderr, "Error: Canvas not initialised in set_pixel_f\n");
//     return;
//     }

//     //clamp intensity to the valid range
//     intensity = clamp_float(intensity, 0.0f, 1.0f);

//     int x_floor = (int)floor(x);
//     int y_floor = (int)floor(y);

//     float fx = x - x_floor;
//     float fy = y - y_floor;

//     int p_coords[4][2] = {
//         {x_floor, y_floor},
//         {x_floor+1, y_floor},
//         {x_floor, y_floor+1},
//         {x_floor+1, y_floor+1}
//     };

//     float weights[4] = {
//         (1.0f-fx)*(1.0-fy),
//         fx * (1.0f -fx),
//         (1.0f - fx) * fy,
//         fx *fy
//     };

//     for (int i = 0; i < 4; ++i) {
//         int current_px = p_coords[i][0];
//         int current_py = p_coords[i][1];

//         if (current_px >= 0 &&  current_px < canvas->width && current_py >= 0 && current_py < canvas-> height) {
//             canvas->pixels[current_py][current_px] += intensity * weights[i];

//             canvas->pixels[current_py][current_px] = clamp_float(canvas->pixels[current_py][current_px],0.0f,1.0f);
//         }
//     }
// }

// In canvas.c, within set_pixel_f:

// Add this function prototype at the top of canvas.c or in canvas.h if it's external
// bool clip_to_circular_viewport(int px, int py, int width, int height); 
// (If clip_to_circular_viewport is in renderer.c, you'd need to expose it or move it.)

void set_pixel_f(canvas_t* canvas, float x, float y, float intensity) {
    if(!canvas || !canvas->pixels) {
        fprintf(stderr, "Error: Canvas not initialised in set_pixel_f\n");
    return;
    }

    // Clamp intensity to the valid range
    intensity = clamp_float(intensity, 0.0f, 1.0f);

    int x_floor = (int)floor(x);
    int y_floor = (int)floor(y);

    float fx = x - x_floor;
    float fy = y - y_floor;

    // The logic below for bilinear filtering is correct.
    // Now, we need to apply the circular clipping.
    // For each of the 4 pixels, check if it's within the circular viewport BEFORE adding intensity.
    // This assumes the circular viewport parameters (width, height) are available or derived.
    // For simplicity, let's assume `clip_to_circular_viewport` will check against the canvas's width/height.

    int p_coords[4][2] = {
        {x_floor, y_floor},
        {x_floor+1, y_floor},
        {x_floor, y_floor+1},
        {x_floor+1, y_floor+1}
    };

    float weights[4] = {
        (1.0f-fx)*(1.0f-fy), // Fixed 1.0-fy
        fx * (1.0f - fy),    // Fixed 1.0-fy (originally 1.0-fx)
        (1.0f - fx) * fy,
        fx * fy
    };


    for (int i = 0; i < 4; ++i) {
        int current_px = p_coords[i][0];
        int current_py = p_coords[i][1];

        // Check if pixel is within canvas bounds AND within the circular viewport
        if (current_px >= 0 &&  current_px < canvas->width && 
            current_py >= 0 && current_py < canvas->height &&
            clip_to_circular_viewport(current_px, current_py, canvas->width, canvas->height)) // <--- ADD THIS LINE
        {
            canvas->pixels[current_py][current_px] += intensity * weights[i];
            canvas->pixels[current_py][current_px] = clamp_float(canvas->pixels[current_py][current_px],0.0f,1.0f);
        }
    }
}

void canvas_clear(canvas_t* canvas, float intensity) {
    if (!canvas || !canvas->pixels) return;
    float clamped_intensity = clamp_float(intensity, 0.0f, 1.0f);
    for (int y = 0; y < canvas->height; ++y) {
        for (int x = 0; x < canvas->width; ++x) {
            canvas->pixels[y][x] = clamped_intensity;
        }
    }
}

void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness, float intensity) {
    // Typical error handling as before
    if (!canvas) {
        fprintf(stderr, "Error: Canvas not initialised in draw_line_f\n");
        return;
    }
    intensity = clamp_float(intensity, 0.0f, 1.0f);

    float dx = x1 - x0;
    float dy = y1 - y0;

    float steps;
    if (fabs(dx) > fabs(dy)) {
        steps = fabs(dx);
    } else {
        steps = fabs(dy);
    }

    //handling case when the line is very short or it's actually limited to a point
    if (steps < 1.0f) {
        if (thickness < 1.0f) {
            set_pixel_f(canvas, x0, y0, intensity);
        } else {
            float half_thick = thickness / 2.0f;

            for (float ty = -half_thick; ty <= half_thick; ty += 0.5f){
                for (float tx = -half_thick; tx <= half_thick; tx += 0.5f) {
                    set_pixel_f(canvas, x0 +tx, y0 +ty, intensity);
                }
            }
        }
    return; //done drawing atp
    }

    float x_increment = dx / steps;
    float y_increment = dy / steps;

    float current_x = x0;
    float current_y = y0;

    float half_thick = thickness / 2.0f;
    if (half_thick < 0.5f && thickness > 0.0f) half_thick = 0.5f; //create a min val for small thicknesses

    for (int i = 0; i <= (int)steps; ++i) {
        if(thickness <= 1.0f) {
            set_pixel_f(canvas, current_x, current_y, intensity);
        } else {
            for (float ty = -half_thick + 0.25f; ty < half_thick; ty +=0.5f) {
                for (float tx = -half_thick+0.25f; tx < half_thick; tx += 0.5f) {
                    set_pixel_f(canvas, current_x+tx, current_y+ty, intensity);
                }
            }
        }

        current_x += x_increment;
        current_y += y_increment;
    }
}

int canvas_save_to_pgm(const canvas_t* canvas, const char* filename) {
    //error handling
    if (!canvas || !canvas->pixels){
        fprintf(stderr, "Error: Canvas not initialised or no pixel data is available to save\n");
        return -1;
    }

    FILE* fp = fopen(filename, "w");
    if ((!fp)){
        perror("Error opening file for PGM export");
        return -1;
    }

    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", canvas->width, canvas->height);
    fprintf(fp, "255\n");

    //write pixel data

    for (int y =0; y < canvas->height; ++y){
        for(int x =0; x < canvas->width; ++x) {

            float current_pixel_intensity = clamp_float(canvas->pixels[y][x], 0.0f,1.0f);
            int gray_value = (int)(current_pixel_intensity *255.0f);

            fprintf(fp, "%d ", gray_value);

        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Canvas saved to %s\n", filename);
    return 0;
    
}

