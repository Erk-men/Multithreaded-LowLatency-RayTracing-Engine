#include "scene.h"
#include "bvh.h"
#include "scene_builders.h"
#include "camera.h"
#include "material.h"
#include <chrono>
#include <cstdio>
#include <vector>

int main() {
    printf("n,time_brute_ms,time_bvh_ms,time_sah_ms,nodes,depth,build_ms,build_ms_sah,sah_cost_median,sah_cost_sah,checksum_bvh,checksum_sah,avg_visits_median,avg_visits_sah\n");
    int steps[] = {10, 30, 100, 300, 1000, 3000, 10000};
    for (int n : steps) {
        Scene scene;
        std::vector<Sphere> spheres;
        std::vector<Lambertian> lambertians;
        std::vector<Metal> metals;
        build_scene_bench(scene, spheres, lambertians, metals, n);
        auto t0 = std::chrono::high_resolution_clock::now();
        Camera cam;
        double checksum_brute = 0.0;
        const int W = 200, H = 200;
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                double u = (i + 0.5) / W;
                double v = (j + 0.5) / H;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;
                if(scene.hit(ray, 0.001, 1e9, rec)) {
                    checksum_brute += rec.t;
                }
            }
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        fprintf(stderr, "n=%d checksum_brute=%.4f\n", n, checksum_brute);
        Bvh bvh(scene.objects_list());
        auto t2 = std::chrono::high_resolution_clock::now();
        double checksum_bvh = 0.0;
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                double u = (i + 0.5) / W;
                double v = (j + 0.5) / H;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;  
                if(bvh.hit(ray, 0.001, 1e9, rec)) {
                    checksum_bvh += rec.t;
                }
            }
        }
        auto t3 = std::chrono::high_resolution_clock::now();
        fprintf(stderr, "n=%d checksum_bvh=%.4f\n", n, checksum_bvh);
        Bvh bvh_sah(scene.objects_list(), BvhBuild::SAH);
        auto t4 = std::chrono::high_resolution_clock::now();
        double checksum_sah = 0.0;
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                double u = (i + 0.5) / W;
                double v = (j + 0.5) / H;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;
                if(bvh_sah.hit(ray, 0.001, 1e9, rec)) {
                    checksum_sah += rec.t;
                }
            }
        }
        auto t5 = std::chrono::high_resolution_clock::now();
        fprintf(stderr, "n=%d checksum_sah=%.4f\n", n, checksum_sah);
        double time_sah_ms = std::chrono::duration<double, std::milli>(t5 - t4).count();
        double time_brute_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double time_bvh_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        long visited_median = 0, hit_rays_median = 0;
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                double u = (i + 0.5) / W;
                double v = (j + 0.5) / H;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;
                long visited = 0;
                bool h = bvh.hit_instrumented(ray, 0.001, 1e9, rec, visited);
                visited_median += visited;
                if (h) hit_rays_median++;
            }
        }
        double avg_visits_median = hit_rays_median > 0 ? (double)visited_median / hit_rays_median : 0.0;

        long visited_sah = 0, hit_rays_sah = 0;
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                double u = (i + 0.5) / W;   
                double v = (j + 0.5) / H;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;
                long visited = 0;
                bool h = bvh_sah.hit_instrumented(ray, 0.001, 1e9, rec, visited);
                visited_sah += visited;
                if (h) hit_rays_sah++;
            }
        }
        double avg_visits_sah = hit_rays_sah > 0 ? (double)visited_sah / hit_rays_sah : 0.0;

        printf("%d,%.4f,%.4f,%.4f,%d,%d,%ld,%ld,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
        n, time_brute_ms, time_bvh_ms, time_sah_ms,
        bvh.stat_nodes(), bvh.stat_depth(), bvh.stat_build_ms(), bvh_sah.stat_build_ms(), bvh.stat_sah_cost(), bvh_sah.stat_sah_cost(), checksum_bvh, checksum_sah, avg_visits_median, avg_visits_sah);
    }
}