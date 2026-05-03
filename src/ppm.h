#pragma once
#include "vec3.h"
#include <cstdio>

inline void write_color(FILE* out, Color pixel_color) {
    // Renk değerlerini [0,255] aralığına dönüştür
    int ir = static_cast<int>(255.999 * pixel_color.x);
    int ig = static_cast<int>(255.999 * pixel_color.y);
    int ib = static_cast<int>(255.999 * pixel_color.z);
    fprintf(out, "%d %d %d\n", ir, ig, ib); // PPM formatında renk yaz
}

inline void write_ppm_header(FILE* out, int width, int height) {
    fprintf(out, "P3\n%d %d\n255\n", width, height); // PPM formatında başlık yaz
}