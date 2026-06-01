#pragma once
#include "vec3.h"
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>


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
// Basit bir PPM dosyası yazıcı sınıfı. Render işlemi sırasında piksel renklerini bellekte tutar ve sonunda dosyaya kaydeder.
class PPMWriter {
    Color* pixels;
    int width, height;
public:
    PPMWriter(int w, int h) : width(w), height(h) {
        pixels = new Color[w * h]();  // () → sıfırla başlat
    }

    ~PPMWriter() { delete[] pixels; }
    PPMWriter(const PPMWriter&)            = delete;
    PPMWriter& operator=(const PPMWriter&) = delete;

    void set_pixel(int x, int y, Color c) {
        pixels[y * width + x] = c;
    }

    void save(const std::string& filename) {
        std::ofstream f(filename);
        if (!f) {
            std::cerr << "PPMWriter::save: dosya acilamadi: " << filename << "\n";
            return;
        }
        f << "P3\n" << width << " " << height << "\n255\n";
        for (int j = height - 1; j >= 0; --j) {
            for (int i = 0; i < width; ++i) {
                Color c = pixels[j * width + i];
                int ir = static_cast<int>(255.999 * c.x);
                int ig = static_cast<int>(255.999 * c.y);
                int ib = static_cast<int>(255.999 * c.z);
                f << ir << " " << ig << " " << ib << "\n";
            }
        }
    }
};