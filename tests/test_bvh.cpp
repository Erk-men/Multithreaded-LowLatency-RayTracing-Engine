#include <cassert>
#include <cstdio>
#include <cmath>
#include <vector>
#include "Sphere.h"
#include "scene.h"
#include "bvh.h"
#include "camera.h"

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
    Bvh bvh(objs);

    // Sabit ışınlar(RNG yok). Her biri için Scene::hit() ve Bvh::hit() çağrılır ve sonuçlar karşılaştırılır.
    Ray rays[] = {
        Ray(Point3(0, 0, 5), Vec3(0, 0, -1)),  // Düz çarpma, il küreye
        Ray(Point3(1.5, 0, 5), Vec3(0, 0, -1)), // Düz çarpma, sağ küreye
        Ray(Point3(-1.5, 0, 5), Vec3(0, 0, -1)), // Düz çarpma, sol küreye
        Ray(Point3(0, 1.5, 5), Vec3(0, 0, -1)), // Düz çarpma, üst küreye
        Ray(Point3(0, -1.5, 5), Vec3(0, 0, -1)), // Düz çarpma, alt küreye
        Ray(Point3(0, 0, 5), Vec3(1.5, 0, -1)), // Sağ üstten çarpma, sağ küreye
        Ray(Point3(0, 0, 5), Vec3(-1.5, 0, -3)), // Sol üstten çarpma, sol küreye
        Ray(Point3(0, 0, 5), Vec3(0, 1.5, -4)), // Üst üstten çarpma, üst küreye
        Ray(Point3(0, 0, 5), Vec3(0, -1.5, -5)), // Alt üstten çarpma, alt küreye
        Ray(Point3(2, 2, 2), Vec3(-1.5, -1.5, -2)), // Sağ alttan çarpma, sağ küreye
        Ray(Point3(-2, -2, -2), Vec3(1.5, 1.5, 2)), // Sol alttan çarpma, sol küreye
        Ray(Point3(10, 10, 5), Vec3(0, 0, -1)), // Tam ıska
        Ray(Point3(0, 0, -1), Vec3(1, 0, 0)), // İlk kürenin içinden başlayan ışın
        Ray(Point3(0, 0, -10), Vec3(0, 0, 1)), // Arkadan en uzak küreye doğru
    };

    for (const auto& ray : rays) {
        HitRecord rec_scene, rec_bvh;
        bool hit_scene = scene.hit(ray, 0.001, 1e+9, rec_scene);
        bool hit_bvh = bvh.hit(ray, 0.001, 1e+9, rec_bvh);

        assert(hit_scene == hit_bvh);
        if (hit_scene) {
            assert(std::fabs(rec_scene.t - rec_bvh.t) < 1e-9);
            assert(std::fabs(rec_scene.normal.x - rec_bvh.normal.x) < 1e-9);
            assert(std::fabs(rec_scene.normal.y - rec_bvh.normal.y) < 1e-9);
            assert(std::fabs(rec_scene.normal.z - rec_bvh.normal.z) < 1e-9);
            assert(rec_scene.mat_ptr == rec_bvh.mat_ptr);
        }
    }
    printf("Sabit ışınlar Scene vs Bvh eşdeğer\n");

    // Tek tek seneryo yerine ölçekte (binlerce karşılaştırma) test edelim. Bu, BVH'nin performansını ve doğruluğunu daha iyi test eder.
    Camera cam;
    const int W = 64, H = 64;
    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            double u = (i + 0.5) / W;
            double v = (j + 0.5) / H;
            Ray ray = cam.get_ray(u, v);
            HitRecord rec_scene, rec_bvh;
            bool hit_scene = scene.hit(ray, 0.001, 1e+9, rec_scene);
            bool hit_bvh = bvh.hit(ray, 0.001, 1e+9, rec_bvh);
            assert(hit_scene == hit_bvh);
            if (hit_scene) {
                assert(std::fabs(rec_scene.t - rec_bvh.t) < 1e-9);
                assert(rec_scene.mat_ptr == rec_bvh.mat_ptr);
            }
        }
    }
    printf("%dx%d ışınlar Scene vs Bvh eşdeğer %d\n", W, H, W*H);

    // Sağlık kontrolü: Dengeli ağaç derinliği yaklaşık log2(n/threshold) olmalı. Çok dengesizse BVH'nin performansı düşer.
    int n = (int)spheres.size();
    double expected_depth = std::log2(n / (double)BVH_LEAF_THRESHOLD);
    assert(bvh.stat_depth() <= expected_depth + 3); // +3 tolerans, küçük sahnelerde dengesizlik olabilir
    printf("BVH derinliği: %d, beklenen <= %.2f\n", bvh.stat_depth(), expected_depth + 3);

    printf("BVH yaprak başına ortalama nesne sayısı: %.2f\n", bvh.stat_avg_leaf());

    printf("ALL BVH TESTS PASSED\n");
    return 0;
}