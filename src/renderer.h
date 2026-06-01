#pragma once
#include "scene.h"
#include "camera.h"
#include "ppm.h"
#include <chrono>
#include <cstdio>


class Renderer {
    public:
    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;

    Renderer(int width, int height, int samples, int depth) 
        : image_width(width), image_height(height), 
          samples_per_pixel(samples), max_depth(depth) {} 
        
    long long render(const Scene& scene, const Camera& camera, FILE* out);

};


struct RenderConfig {
      int width;
      int height;
      int samples;
      int max_depth;
};

void render_tile(int x0, int y0, int x1, int y1,
                const Scene& scene,
                const Camera& camera,
                PPMWriter& writer,
                const RenderConfig& cfg);
