#ifndef CANVAS_H //including guard to make sure another canvas_h file is not defined and if so then proceed
#define CANVAS_H //defining canvas

typedef struct  //defining a struct to carry info about the canvas
{
    int width;
    int height;
    float **pixels;
} canvas_t;

//prototyping some functions

canvas_t* canvas_create(int width, int height);

void canvas_destroy(canvas_t* canvas);

void set_pixel_f(canvas_t* canvas, float x, float y, float intensity);

void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness, float intensity);

int canvas_save_to_pgm(const canvas_t* canvas, const char* filename);

#endif
