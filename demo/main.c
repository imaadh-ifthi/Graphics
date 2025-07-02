#include <stdio.h>
#include <math.h>
#include "canvas.h"

#ifndef M_PI
#define M_PI 3.1415926
#endif

int main(){
    int width = 600;
    int height = 600;

    printf("Creating canvas...\n");
    canvas_t* my_canvas = canvas_create(width, height);

    if(!my_canvas){
        fprintf(stderr, "Failed to create canavs in main.\n");
        return 1;
    }

    float center_x = width/2.0f;
    float center_y = height/2.0f;

    float line_length = (width < height ? width : height) /2.5f;

    float line_thickness = 1.5f;
    float line_intensity = 0.8f;

    printf("Drawing clock face pattern.. \n");

    for (int angle_deg = 0; angle_deg < 360; angle_deg +=15) {
        float angle_rad = angle_deg * (M_PI/180.0f);

        float end_x = center_x + line_length * cosf(angle_rad);
        float end_y = center_y + line_length * sinf(angle_rad);

        draw_line_f(my_canvas, center_x, center_y, end_x, end_y, line_thickness, line_intensity);

    }

    float border_intensity = 0.5f;
    float border_thickness = 3.0f;

    draw_line_f(my_canvas, 0.0f, 0.0f, width-1.0f, 0.0f, border_thickness, border_intensity);
    draw_line_f(my_canvas, 0.0f, height-1.0f, width-1.0f, height-1.0f, border_thickness, border_intensity);
    draw_line_f(my_canvas, 0.0f, 0.0f, 0.0f, height-1.0f, border_thickness, border_intensity);
    draw_line_f(my_canvas, width-1.0f, 0.0f, width-1.0f, height-1.0f, border_thickness, border_intensity);

    printf("Saving canvas to a PGM file");

    if (canvas_save_to_pgm(my_canvas, "build/stage1_demo.pgm")==0){
        printf("Demo output saved to build/stage1_demo.pgm succesfully");
    } else {
        fprintf(stderr, "Failed to save canvas.\n");
    }

    printf("Destroying canvas...\n");
    canvas_destroy(my_canvas);

    printf("Demo finished.\n");

    return 0;
}