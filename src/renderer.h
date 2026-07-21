#pragma once
#include "scene.h"
#include "hittable.h"
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

// FIX-16 (D-11): Tile boyutunun TEK kaynağı. Eskiden main.cpp'de üç ayrı yerde
// (pool 128, aligned 64, animate 64) tekrarlanan magic number'dı. progress.h'daki
// MAX_THREADS single-source-of-truth desenini taklit eder. Bilinçli olarak bir
// constexpr, YENİ bir CLI bayrağı DEĞİL — bir --tile-size bayrağı ileriki bir faza
// (BVH/SIMD tile-boyutu taraması gerekirse) ertelendi (bkz. 01-CONTEXT Deferred).
constexpr int TILE_SIZE = 64;

void render_tile(int x0, int y0, int x1, int y1,
                const Hittable& world,
                const Camera& camera,
                PPMWriter& writer,
                const RenderConfig& cfg);
