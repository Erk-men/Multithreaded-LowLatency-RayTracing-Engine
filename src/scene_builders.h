#pragma once
#include "scene.h"
#include "Sphere.h"
#include "material.h"
#include "vec3.h"
#include <vector>
#include <cstdlib>

// Materyal cesitliligi (rand()%5==0 -> Metal, aksi halde Lambertian, ~%20 metal) —
// main.cpp'nin build_scene_complex'inde kurulan desenin aynısı, buraya da tasindi.
// metals de spheres/lambertians gibi n kapasiteyle reserve edilir: rand()%5==0
// olasiliksal oldugu icin teorik olarak n'in HEPSI metal cikabilir (garanti degil
// ama mumkun) — reserve(n) bu en-kotu-durumu karsilar, "ortalama ~n/5" degil.
inline void build_scene_bench(Scene& scene, std::vector<Sphere>& spheres,
    std::vector<Lambertian>& lambertians, std::vector<Metal>& metals, int n) {
        spheres.reserve(n);
        lambertians.reserve(n);
        metals.reserve(n);
        for (int i = 0; i < n; ++i) {
            double x = -0.5 + (rand() / (double)RAND_MAX) * 10.0; // x koordinatı -0.5 ile 9.5 arasında
            double y = -0.5 + (rand() / (double)RAND_MAX) * 2.0; // y koordinatı -0.5 ile 1.5 arasında
            double z = -1.0 - (rand() / (double)RAND_MAX) * 10.0; // z koordinatı -0.5 ile 9.5 arasında
            double r = 0.1 + (rand() / (double)RAND_MAX) * 0.4; // yarıçap 0.1 ile 0.5 arasında

            Material* mat_ptr;
            if (rand() % 5 == 0) {
                metals.push_back(Metal(Color(
                    0.5 + (rand() / (double)RAND_MAX) * 0.5,
                    0.5 + (rand() / (double)RAND_MAX) * 0.5,
                    0.5 + (rand() / (double)RAND_MAX) * 0.5),
                    rand() / (double)RAND_MAX));
                mat_ptr = &metals.back();
            } else {
                lambertians.push_back(Lambertian(Color(
                    rand() / (double)RAND_MAX,
                    rand() / (double)RAND_MAX,
                    rand() / (double)RAND_MAX
                    )));
                mat_ptr = &lambertians.back();
            }

            spheres.push_back(Sphere(Vec3(x, y, z), r, mat_ptr));
            scene.add(&spheres.back());
        }
}

inline void build_scene_clustered(Scene& scene, std::vector<Sphere>& spheres,
    std::vector<Lambertian>& lambertians, std::vector<Metal>& metals, int n) {
        spheres.reserve(n);
        lambertians.reserve(n);
        metals.reserve(n);

        const int NUM_CLUSTERS = 5;
        Vec3 cluster_centers[NUM_CLUSTERS] = {
            Vec3(-4, 0, -3),
            Vec3(4, 0, -3),
            Vec3(-4, 0, -9),
            Vec3(4, 0, -9),
            Vec3(0, 2, -6)
        };

        for (int i = 0; i < n; ++i) {
            int cluster_index = rand() % NUM_CLUSTERS; // Rastgele bir cluster seç
            Vec3 center = cluster_centers[cluster_index];

            double x = center.x + (-0.5 + (rand() / (double)RAND_MAX) * 1.0); // x koordinatı cluster merkezine yakın
            double y = center.y + (-0.5 + (rand() / (double)RAND_MAX) * 1.0); // y koordinatı cluster merkezine yakın
            double z = center.z + (-0.5 + (rand() / (double)RAND_MAX) * 1.0); // z koordinatı cluster merkezine yakın
            double r = 0.1 + (rand() / (double)RAND_MAX) * 0.2; // yarıçap 0.1 ile 0.2 arasında

            Material* mat_ptr;
            if (rand() % 5 == 0) {
                metals.push_back(Metal(Color(
                    0.5 + (rand() / (double)RAND_MAX) * 0.5,
                    0.5 + (rand() / (double)RAND_MAX) * 0.5,
                    0.5 + (rand() / (double)RAND_MAX) * 0.5),
                    rand() / (double)RAND_MAX));
                mat_ptr = &metals.back();
            } else {
                lambertians.push_back(Lambertian(Color(
                    rand() / (double)RAND_MAX,
                    rand() / (double)RAND_MAX,
                    rand() / (double)RAND_MAX
                    )));
                mat_ptr = &lambertians.back();
            }

            spheres.push_back(Sphere(Vec3(x, y, z), r, mat_ptr));
            scene.add(&spheres.back());
        }
    }
