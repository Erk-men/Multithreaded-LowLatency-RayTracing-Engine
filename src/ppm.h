#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include "vec3.h"

// =============================================================================
// PPM Yazıcı
//
// PPM (Portable Pixel Map) — en basit görüntü formatı. Header + RGB piksel.
// Binary değil, düz metin. eog / feh / ImageMagick ile açılır.
//
// Format:
//   P3              ← "plain text RGB" anlamında magic number
//   800 600         ← genişlik yükseklik
//   255             ← maksimum renk değeri
//   255 0 0         ← piksel (0,0): kırmızı
//   0 255 0         ← piksel (1,0): yeşil
//   ...
//
// Piksel sırası: sol üstten sağ alta, satır satır.
// =============================================================================

class PPMWriter {
public:
    int width, height;
    std::vector<Color> pixels;  // Tüm pikseller [0.0, 1.0] aralığında

    PPMWriter(int w, int h) : width(w), height(h), pixels(w * h) {}

    // Piksel yaz — (row, col) koordinatıyla
    // row=0 → üst satır, col=0 → sol sütun
    void set(int row, int col, const Color& c) {
        pixels[row * width + col] = c;
    }

    // Piksel oku
    Color get(int row, int col) const {
        return pixels[row * width + col];
    }

    // PPM dosyasına kaydet
    // gamma_correct: insan gözü parlaklığı doğrusal algılamaz.
    //   Gamma=2 düzeltmesiyle sqrt() uygulanır → daha doğal görünüm.
    void save(const std::string& filename, bool gamma_correct = true) const {
        std::ofstream file(filename);
        if (!file) throw std::runtime_error("Dosya açılamadı: " + filename);

        // PPM header
        file << "P3\n" << width << " " << height << "\n255\n";

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                Color c = pixels[row * width + col];

                // Gamma düzeltmesi: sqrt() ile gamma=2 yaklaşımı
                if (gamma_correct) c = c.gamma_correct();

                // [0,1] → [0,255] dönüşümü, taşmayı önlemek için clamp
                c = c.clamp();
                int r = static_cast<int>(255.999 * c.r());
                int g = static_cast<int>(255.999 * c.g());
                int b = static_cast<int>(255.999 * c.b());

                file << r << " " << g << " " << b << "\n";
            }
        }
    }
};
