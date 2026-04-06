#pragma once
#include "vec3.h"

// =============================================================================
// Ray — Işın
//
// Matematiksel tanım:
//   r(t) = origin + t * direction
//
// origin    : ışının başladığı nokta (genellikle kamera)
// direction : ışının gittiği yön (normalize edilmiş birim vektör)
// t         : parametre — t arttıkça ışın ilerler
//
// Örnekler:
//   t = 0.0  → origin noktasının kendisi
//   t = 1.0  → origin + direction (1 birim ileri)
//   t = 5.3  → origin + 5.3 * direction
//
// Negatif t değerleri ışının geriye gitmesini temsil eder.
// Kesişim hesaplarında sadece t > 0 (kameranın önü) anlamlıdır.
// =============================================================================

class Ray {
public:
    Point3 origin;
    Vec3   direction;  // Her zaman normalize edilmeli

    Ray() {}
    Ray(const Point3& origin, const Vec3& direction)
        : origin(origin), direction(Vec3::normalize(direction)) {}

    // r(t) = origin + t * direction
    // Verilen t parametresi için ışın üzerindeki noktayı döndürür.
    // Kullanım: kesişim noktasını bulmak için t hesaplandıktan sonra çağrılır.
    Point3 at(double t) const {
        return origin + direction * t;
    }
};
