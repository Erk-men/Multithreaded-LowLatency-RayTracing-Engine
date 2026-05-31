#include "renderer_v2.h"
#include "material.h"
#include "ppm.h"
#include <thread>   
#include <limits>
#include <chrono>
#include <random>


static Color ray_color(const Ray& ray, const Hittable& world, int depth) {
    if (depth <= 0)
        return Color(0, 0, 0); //Maks derinliğe ulaşıldığında siyah döndür  
    HitRecord rec;
    if (world.hit(ray, 0.001, std::numeric_limits<double>::infinity(), rec)) {
        Ray scattered;
        Vec3 attenuation;
        if (rec.mat_ptr->scatter(ray, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1); //Yansıyan ışının rengini hesapla
        return Color(0, 0, 0);
    }
    Vec3 unit_dir = ray.direction.normalize();
    double t = 0.5 * (unit_dir.y + 1.0);
    return (1.0 - t) * Color(1, 1, 1) + t * Color(0.5, 0.7, 1.0); //Arka plan rengi: gökyüzü mavisi ile beyaz arasında geçiş
}
RendererV2::RendererV2(int width, int height, int samples, int depth)
    : image_width(width), image_height(height),
    samples_per_pixel(samples), max_depth(depth) {}

long long RendererV2::render(const Scene& scene, const Camera& camera, FILE* out) {
    auto start = std::chrono::high_resolution_clock::now();
    write_ppm_header(out, image_width, image_height);
    Color* buffer = new Color[image_height * image_width]; // Render sonucunu geçici olarak tutacak bir buffer oluştur heap te
    std::thread* threads = new std::thread[image_height]; // Her satır için bir thread oluşturmak için bir dizi oluştur
    for (int j = 0; j < image_height; ++j) {
        threads[j] = std::thread([&, j] { // Her satır için ayrı bir thread başlat
            static thread_local std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            for (int i = 0; i < image_width; ++i) {
                Color pixel_color(0, 0, 0);
                for (int s = 0; s < samples_per_pixel; ++s) {
                    double u = (i + dist(rng)) / (image_width - 1);
                    double v = (j + dist(rng)) / (image_height - 1);
                    Ray ray = camera.get_ray(u, v);
                    pixel_color = pixel_color + ray_color(ray, scene, max_depth);
                }
                buffer[j * image_width + i] = pixel_color; // Render sonucunu buffer a kaydet
            }
        });
    }
    for (int j = 0; j < image_height; ++j) {
        threads[j].join(); // Tüm thread lerin bitmesini bekle
    }
        delete[] threads; // Thread dizisini temizle
        // Buffer ı sırala ve dosyaya yaz
        for (int j = image_height - 1; j >= 0; --j) {
            for (int i = 0; i < image_width; ++i) {
                Color avg = buffer[j * image_width + i] / samples_per_pixel;
                Color gc = Color(Vec3::gamma_correct(avg.x), 
                                Vec3::gamma_correct(avg.y), 
                                Vec3::gamma_correct(avg.z));
                write_color(out, gc); // Rengi dosyaya yaz
            }
        }
        delete[] buffer; // Buffer ı temizle
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        fprintf(stderr, "Rendering time: %ld ms\n", duration.count());
        return duration.count();
}
