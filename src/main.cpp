#include <cstdio>
#include "camera.h"
#include "ppm.h"

Color ray_color(const Ray& r) {
    Vec3 unit_direction = r.direction.normalize(); // Işının yönünü birim vektöre dönüştür
    double t = 0.5 * (unit_direction.y + 1.0); // Yönün y bileşenine göre t değeri hesapla
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0); // Beyaz ile mavi arasında geçiş yap
}

int main() {
    int width = 400;
    int height = static_cast<int>(width / (16.0 / 9.0)); // Görüntü yüksekliği, 16:9 oranına göre hesaplanır
    Camera cam; // Kamera oluştur

    FILE* f = fopen("output/renders/gradient.ppm", "w"); // PPM dosyası oluştur
    write_ppm_header(f, width, height); // PPM başlığını yaz

    for (int j = height-1; j>= 0; --j) { // Her satır için (üstten alta)
        for (int i = 0; i < width; ++i) { // Her sütun için (soldan sağa)
            double u = double(i) / (width-1); // Görüntü düzlemi boyunca yatay konum
            double v = double(j) / (height-1); // Görüntü düzlemi boyunca dikey konum
            Ray r = cam.get_ray(u, v); // Kameradan bu konuma bir ışın oluştur
            Color pixel_color = ray_color(r); // Işığın rengini hesapla
            write_color(f, pixel_color); // Rengi PPM dosyasına yaz
        }
    }
    fclose(f); // Dosyayı kapat
    printf("PPM dosyasi olusturuldu: output/renders/gradient.ppm\n");
}
