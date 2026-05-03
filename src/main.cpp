#include <cstdio>
#include "camera.h"
#include "ppm.h"
#include "Sphere.h"
#include "scene.h"


Color ray_color(const Ray& r, const Hittable& world) {
    HitRecord rec; // Çarpışma bilgilerini tutacak yapı
    if (world.hit(r, 0.0, 1e9, rec)) { 
        return 0.5 * (rec.normal + Vec3(1, 1, 1)); // Çarpışma varsa, normal vektörünü renk olarak döndür (0.5 ile ölçeklenmiş)
    }
    Vec3 unit_direction = r.direction.normalize(); // Işının yönünü birim vektöre dönüştür
    double t = 0.5 * (unit_direction.y + 1.0); // Yatay koordinat [0,1] aralığında
    return (1.0 -t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);  // Arka plan rengi: üstte açık mavi, altta beyaz olacak şekilde lineer interpolasyon yaparak döndür
}

int main() {
    int width = 400;
    int height = static_cast<int>(width / (16.0 / 9.0)); // Görüntü yüksekliği, 16:9 oranına göre hesaplanır
    Camera cam; // Kamera oluştur

    Sphere sphere(Vec3(0, 0, -1), 0.5); // Küre oluştur
    Scene scene; // Sahne oluştur
    scene.add(&sphere); // Küreyi sahneye ekle
    Sphere ground(Vec3(0, -100.5, -1), 100); // Zemin küresi oluştur
    scene.add(&ground); // Zemini sahneye ekle
    FILE* out = fopen("output/renders/sphere.ppm", "w"); // PPM dosyası oluştur
    write_ppm_header(out, width, height); // PPM başlığını yaz
    for (int j = height - 1; j >= 0; --j) { // Satırları tersten yaz (PPM formatı için)
        for (int i = 0; i < width; ++i) { // Her sütun için
            double u = double(i) / (width - 1); // Yatay koordinat [0,1] aralığında
            double v = double(j) / (height - 1); // Dikey koordinat [0,1] aralığında
            Ray r = cam.get_ray(u, v); // Kameradan piksele giden ışını al
            Color pixel_color = ray_color(r, scene); // Işığın rengini hesapla
            write_color(out, pixel_color); // Rengi PPM dosyasına yaz
        }
    }
    fclose(out); // Dosyayı kapat
    return 0;
}
