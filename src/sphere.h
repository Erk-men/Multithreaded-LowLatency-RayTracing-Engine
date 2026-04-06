#pragma once
#include <cmath>
#include "hittable.h"

// =============================================================================
// Sphere — Küre Geometrisi
//
// Işın-küre kesişiminin türetimi:
//
//   Küre denklemi : |P - C|² = R²
//   Işın          : P = o + t*d
//
//   Yerine koy    : |o + t*d - C|² = R²
//   oc = o - C dersek:
//                   |t*d + oc|² = R²
//                   t²(d·d) + 2t(d·oc) + (oc·oc) - R² = 0
//
//   İkinci dereceden denklem: at² + bt + c = 0
//     a = d·d       (d normalize ise a = 1)
//     b = 2(d·oc)
//     c = oc·oc - R²
//
//   Discriminant: Δ = b² - 4ac
//     Δ < 0  → kesişim yok
//     Δ = 0  → teğet (1 nokta)
//     Δ > 0  → 2 nokta, en küçük pozitif t'yi al
//
// Optimizasyon: b = 2h yazılırsa (h = d·oc):
//   Δ = 4h² - 4ac = 4(h² - ac)
//   t = (-b ± √Δ) / 2a = (-2h ± 2√(h²-ac)) / 2a = (-h ± √(h²-ac)) / a
//   Böylece 2 ve 4 çarpanları iptali — daha temiz formül.
// =============================================================================

class Sphere : public Hittable {
public:
    Point3 center;
    double radius;

    Sphere(const Point3& center, double radius)
        : center(center), radius(radius) {}

    bool hit(const Ray& ray, double t_min, double t_max,
             HitRecord& rec) const override {

        Vec3 oc = ray.origin - center;

        // Optimizasyonlu formül (b=2h yerine koy):
        double a = ray.direction.length_squared();  // d·d
        double h = Vec3::dot(ray.direction, oc);    // d·oc (= b/2)
        double c = oc.length_squared() - radius * radius;

        double discriminant = h * h - a * c;

        // Δ < 0 → kesişim yok
        if (discriminant < 0) return false;

        double sqrt_d = std::sqrt(discriminant);

        // İki kök: önce küçük t'yi dene (kameraya yakın yüzey)
        double t = (-h - sqrt_d) / a;
        if (t < t_min || t > t_max) {
            // Küçük kök aralık dışındaysa büyük köke bak
            t = (-h + sqrt_d) / a;
            if (t < t_min || t > t_max) return false;
        }

        // Kesişim kaydını doldur
        rec.t     = t;
        rec.point = ray.at(t);

        // Dışa bakan normal: küre merkezi → kesişim noktası (normalize edilmiş)
        Vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(ray, outward_normal);

        return true;
    }
};
