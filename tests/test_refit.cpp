#include <cassert>
#include <cstdio>
#include <cmath>
#include <vector>
#include "Sphere.h"
#include "scene.h"
#include "bvh.h"
#include "camera.h"
#include "scene_builders.h"


int main() {
    std::vector<Sphere> spheres;
    spheres.reserve(10);
    spheres.push_back(Sphere(Point3(0, 0, -1), 0.5, nullptr));
    spheres.push_back(Sphere(Point3(1.5, 0, -2), 0.5, nullptr));
    spheres.push_back(Sphere(Point3(-1.5, 0, -3), 0.7, nullptr));
    spheres.push_back(Sphere(Point3(0, 1.5, -4), 0.3, nullptr));
    spheres.push_back(Sphere(Point3(0, -1.5, -5), 0.4, nullptr));

    Scene scene;
    std::vector<Hittable*> objs;
    for (auto& s : spheres) {
        scene.add(&s);
        objs.push_back(&s);
    }

    Bvh refit_bvh(objs);

    spheres[0].center = Point3(3, 0, -1); // Küreyi hareket ettir
    refit_bvh.refit(); // BVH'yi yeniden inşa et

    Bvh rebuilt_bvh(objs); // BVH'yi yeniden inşa et

    Camera cam;
    const int W = 64, H = 64;
    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            double u = (i + 0.5) / W;
            double v = (j + 0.5) / H;
            Ray ray = cam.get_ray(u, v);
            HitRecord rec_refit, rec_rebuilt;
            bool hit_refit = refit_bvh.hit(ray, 0.001, 1e+9, rec_refit);
            bool hit_rebuilt = rebuilt_bvh.hit(ray, 0.001, 1e+9, rec_rebuilt);
            assert(hit_refit == hit_rebuilt);
            if (hit_refit) {
                assert(std::fabs(rec_refit.t - rec_rebuilt.t) < 1e-6);
                assert(std::fabs(rec_refit.point.x - rec_rebuilt.point.x) < 1e-6);
                assert(std::fabs(rec_refit.point.y - rec_rebuilt.point.y) < 1e-6);
                assert(std::fabs(rec_refit.point.z - rec_rebuilt.point.z) < 1e-6);
                assert(std::fabs(rec_refit.normal.x - rec_rebuilt.normal.x) < 1e-6);
                assert(std::fabs(rec_refit.normal.y - rec_rebuilt.normal.y) < 1e-6);
                assert(std::fabs(rec_refit.normal.z - rec_rebuilt.normal.z) < 1e-6);
            }
        }
    }
    printf("Refit test passed: BVH refit and rebuild produce identical results.\n");
    return 0;
}