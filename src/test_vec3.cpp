#include <iostream>
#include <cassert>
#include <cmath>
#include "vec3.h"
#include "ray.h"

// Floating point karşılaştırması için yardımcı
bool approx(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

int main() {
    std::cout << "=== Vec3 & Ray Testleri ===\n\n";

    // --- Temel aritmetik ---
    Vec3 a(1, 2, 3);
    Vec3 b(4, 5, 6);

    Vec3 sum = a + b;
    assert(sum.x == 5 && sum.y == 7 && sum.z == 9);
    std::cout << "Toplama    : " << a << " + " << b << " = " << sum << "\n";

    Vec3 diff = b - a;
    assert(diff.x == 3 && diff.y == 3 && diff.z == 3);
    std::cout << "Çıkarma    : " << b << " - " << a << " = " << diff << "\n";

    Vec3 scaled = a * 2.0;
    assert(scaled.x == 2 && scaled.y == 4 && scaled.z == 6);
    std::cout << "Skaler *   : " << a << " * 2 = " << scaled << "\n";

    // --- Uzunluk ---
    Vec3 v(3, 4, 0);
    double len = v.length();
    assert(approx(len, 5.0));
    std::cout << "\nUzunluk    : |" << v << "| = " << len << " (beklenen: 5.0)\n";

    // --- Normalize ---
    Vec3 n = Vec3::normalize(v);
    assert(approx(n.length(), 1.0));
    std::cout << "Normalize  : " << n << " | uzunluk = " << n.length() << " (beklenen: 1.0)\n";

    // --- Dot Product ---
    Vec3 right(1, 0, 0);
    Vec3 up(0, 1, 0);
    Vec3 fwd(0, 0, -1);

    double d1 = Vec3::dot(right, up);   // dik açı → 0
    double d2 = Vec3::dot(right, right); // aynı yön → 1
    assert(approx(d1, 0.0));
    assert(approx(d2, 1.0));
    std::cout << "\nDot Product:\n";
    std::cout << "  dot(right, up)    = " << d1 << " (dik açı → 0 beklenir)\n";
    std::cout << "  dot(right, right) = " << d2 << " (aynı yön → 1 beklenir)\n";

    // --- Cross Product ---
    Vec3 c = Vec3::cross(right, up);  // sağ × yukarı = ileri (z+)
    std::cout << "\nCross Product:\n";
    std::cout << "  cross(right, up) = " << c << " (beklenen: (0, 0, 1))\n";
    assert(approx(c.x, 0.0) && approx(c.y, 0.0) && approx(c.z, 1.0));

    // --- Reflect ---
    Vec3 incoming(1, -1, 0);  // 45° açıyla gelen ışın
    Vec3 normal(0, 1, 0);     // yukarı bakan yüzey normali
    Vec3 reflected = Vec3::reflect(Vec3::normalize(incoming), normal);
    std::cout << "\nYansıma:\n";
    std::cout << "  Gelen  : " << Vec3::normalize(incoming) << "\n";
    std::cout << "  Normal : " << normal << "\n";
    std::cout << "  Yansıma: " << reflected << " (beklenen: y pozitif)\n";
    assert(reflected.y > 0);  // yansıyan ışın yukarı gitmeliydi

    // --- Renk operasyonları ---
    Color red(1.0, 0.0, 0.0);
    Color yellow(1.0, 1.0, 0.0);
    Color result = red * yellow;
    std::cout << "\nRenk çarpımı:\n";
    std::cout << "  Kırmızı × Sarı = " << result << " (beklenen: kırmızı (1,0,0))\n";
    assert(approx(result.r(), 1.0) && approx(result.g(), 0.0));

    // --- Ray testi ---
    Point3 cam(0, 0, 0);
    Vec3   dir(0, 0, -1);  // kameradan düz ileriye
    Ray ray(cam, dir);

    Point3 p = ray.at(5.0);  // 5 birim ileri
    std::cout << "\nIşın (Ray):\n";
    std::cout << "  Origin   : " << ray.origin << "\n";
    std::cout << "  Direction: " << ray.direction << "\n";
    std::cout << "  r(5.0)   : " << p << " (beklenen: (0, 0, -5))\n";
    assert(approx(p.z, -5.0));

    std::cout << "\n✓ Tüm testler geçti!\n";
    return 0;
}
