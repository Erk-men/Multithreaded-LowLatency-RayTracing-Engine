#pragma once
#include "scene.h"
#include "camera.h"
#include <chrono>
#include <cstdio>
#include <thread>

class RendererV2 {
    private:
    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;
    public:
    RendererV2(int width, int height, int samples, int depth);
        
        
    long long render(const Scene& scene, const Camera& camera, FILE* out);
};