#include "renderer.h"
#include "material.h"
#include "ppm.h"
#include <limits>

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

long long Renderer::render(const Scene& scene, const Camera& camera, FILE* out) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    auto start = std::chrono::high_resolution_clock::now();
    write_ppm_header(out, image_width, image_height);
    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                double u = (i + dist(rng)) / (image_width - 1);
                double v = (j + dist(rng)) / (image_height - 1);
                Ray ray = camera.get_ray(u, v);
                pixel_color = pixel_color + ray_color(ray, scene, max_depth);
            }
            Color avg = pixel_color / samples_per_pixel;
            Color gc = Color(Vec3::gamma_correct(avg.x), 
                            Vec3::gamma_correct(avg.y), 
                            Vec3::gamma_correct(avg.z));
            write_color(out, gc); // Rengi dosyaya yaz
            
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    fprintf(stderr, "Rendering time: %ld ms\n", duration.count());
    return duration.count();
}