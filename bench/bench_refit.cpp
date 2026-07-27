#include "scene.h"
#include "bvh.h"
#include "scene_builders.h"
#include "camera.h"
#include "material.h"
#include <chrono>
#include <cstdio>
#include <vector>
#include <cmath>

int main() {
printf("N,frame,sah_cost,avg_visits\n");
int Ns[] = {5,10,20,50,100,200,500,1000,1000000};

    for (int N : Ns) {
        Scene scene;
        std::vector<Sphere> spheres;
        std::vector<Lambertian> lambertians;
        std::vector<Metal> metals;
        build_scene_clustered(scene, spheres, lambertians, metals, 1000);
        std::vector<Hittable*> objs = scene.objects_list();

        const int NUM_MOVERS = 8;
        std::vector<Point3> base__centers(NUM_MOVERS);
        std::vector<double> phase(NUM_MOVERS);
        for (int i = 0; i < NUM_MOVERS; ++i) {
            base__centers[i] = spheres[i].center;
            phase[i] = i * (2.0 * M_PI / NUM_MOVERS); // Her küreye farklı başlangıç
        }


    const double amplitude = 0.5; //yaprak kutusu boyutu(gerekirse ayarla)

    Bvh bvh(objs, BvhBuild::SAH); // BVH oluştur
    
    Camera cam;
    const int W = 200, H = 100;
    const int FRAMES = 100;
    for (int i = 0; i < FRAMES; i++)
    {
        for (int k = 0; k < NUM_MOVERS; k++) {
            spheres[k].center = base__centers[k] + Vec3(0, amplitude * std::sin(2.0 * M_PI * i / FRAMES + phase[k]), 0);
        }

        if (i % N == 0) {
            bvh = Bvh(objs, BvhBuild::SAH); // BVH'yi yeniden oluştur
        } else {
            bvh.refit(); // BVH'yi yeniden oluşturmak yerine refit yap
        }

        long visited = 0, hit_rays = 0;
        for (int j = 0; j < H; ++j) {
            for (int ii = 0; ii < W; ++ii) {
                double u = (ii + 0.5) / W;
                double v = (j + 0.5) / H;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;
                long vis = 0;
                bool h = bvh.hit_instrumented(ray, 0.001, 1e9, rec, vis);
                visited += vis;
                if (h) hit_rays++;
            }
        }
        double avg_visits = hit_rays > 0 ? (double)visited / hit_rays : 0.0;
        printf("%d, %d, %.4f, %.4f\n", N, i, bvh.stat_sah_cost(), avg_visits);
    }
    
}
}