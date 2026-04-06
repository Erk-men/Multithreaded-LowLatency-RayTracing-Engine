#include <iostream>
#include <memory>
#include <vector>
#include "vec3.h"
#include "ray.h"
#include "hittable.h"
#include "sphere.h"
#include "ppm.h"

// =============================================================================
// İlk Render — Küre ve Gradient Arka Plan
//
// Sahne:
//   - 1 kırmızı küre (merkez: 0,0,-1  yarıçap: 0.5)
//   - 1 büyük yeşil "zemin" küresi (merkez: 0,-100.5,-1  yarıçap: 100)
//   - Mavi→beyaz gradient arka plan
//
// Bu aşamada henüz ışık kaynağı yok.
// Yüzey normalini renk olarak gösteriyoruz (normal visualization).
//   normal.x → kırmızı  (sola/sağa)
//   normal.y → yeşil    (aşağı/yukarı)
//   normal.z → mavi     (ileri/geri)
// Bu teknik debug/görselleştirme için yaygın kullanılır.
// =============================================================================

// Sahnedeki en yakın nesneye çarpıp çarpmadığını kontrol et
bool hit_scene(const std::vector<std::unique_ptr<Hittable>>& scene,
               const Ray& ray, double t_min, double t_max, HitRecord& rec) {
    HitRecord tmp;
    bool hit_anything = false;
    double closest = t_max;

    for (const auto& obj : scene) {
        if (obj->hit(ray, t_min, closest, tmp)) {
            hit_anything = true;
            closest = tmp.t;  // daha yakın nesne bulundu, arama aralığını daralt
            rec = tmp;
        }
    }
    return hit_anything;
}

// Verilen ışın için renk hesapla
Color ray_color(const Ray& ray,
                const std::vector<std::unique_ptr<Hittable>>& scene) {
    HitRecord rec;

    if (hit_scene(scene, ray, 0.001, 1e9, rec)) {
        // Normal visualization: normalin her bileşeni [-1,1] aralığında.
        // [0,1] aralığına map'le: (n + 1) / 2
        return (rec.normal + Vec3(1, 1, 1)) * 0.5;
    }

    // Arka plan: yukarı→mavi, aşağı→beyaz gradient
    // ray.direction.y [-1, +1] aralığında — [0,1]'e normalize et
    double t = 0.5 * (ray.direction.y + 1.0);
    Color white(1.0, 1.0, 1.0);
    Color blue (0.5, 0.7, 1.0);
    return white * (1.0 - t) + blue * t;  // linear interpolation (lerp)
}

int main() {
    // --- Görüntü boyutu ---
    const double aspect_ratio = 16.0 / 9.0;
    const int    image_width  = 800;
    const int    image_height = static_cast<int>(image_width / aspect_ratio); // 450

    // --- Kamera (basit, sabit) ---
    // Viewport: sanal ekran düzlemi (kameradan 1 birim önde)
    double viewport_height = 2.0;
    double viewport_width  = aspect_ratio * viewport_height;
    double focal_length    = 1.0;  // kamera → viewport mesafesi

    Point3 cam_origin(0, 0, 0);
    Vec3 horizontal(viewport_width,  0, 0);   // ekranın yatay ekseni
    Vec3 vertical  (0, viewport_height, 0);   // ekranın dikey ekseni

    // Sol alt köşe: merkez − yatay/2 − dikey/2 − derinlik
    Point3 lower_left = cam_origin
                      - horizontal / 2
                      - vertical   / 2
                      - Vec3(0, 0, focal_length);

    // --- Sahne ---
    std::vector<std::unique_ptr<Hittable>> scene;
    scene.push_back(std::make_unique<Sphere>(Point3(0,    0,   -1), 0.5));   // ana küre
    scene.push_back(std::make_unique<Sphere>(Point3(0, -100.5, -1), 100));   // zemin

    // --- Render ---
    PPMWriter ppm(image_width, image_height);

    std::cout << "Rendering " << image_width << "x" << image_height << "...\n";

    for (int row = image_height - 1; row >= 0; --row) {
        // İlerleme göstergesi
        if (row % 50 == 0)
            std::cerr << "\rSatır: " << (image_height - row) << "/" << image_height << std::flush;

        for (int col = 0; col < image_width; ++col) {
            // Piksel (col, row) → [0,1] normalize koordinat
            double u = double(col) / (image_width  - 1);  // yatay  [0→1]
            double v = double(row) / (image_height - 1);  // dikey  [0→1]

            // Kameradan bu piksele doğru ışın
            Vec3 dir = lower_left + horizontal * u + vertical * v - cam_origin;
            Ray ray(cam_origin, dir);

            Color c = ray_color(ray, scene);
            ppm.set(image_height - 1 - row, col, c);
        }
    }

    std::cerr << "\nKaydediliyor...\n";
    ppm.save("output/render_phase2.ppm");
    std::cout << "Tamamlandı: output/render_phase2.ppm\n";

    return 0;
}
