#pragma once
#include "vec3.h"
#include "ray.h"
#include <limits> // std::numeric_limits için
#include <utility> //std::swap için



struct AABB
{
    Point3 min, max; // minimum ve maksimum köşe noktaları

    // Axis-Aligned Bounding Box (AABB) sınıfı, 3D uzayda bir kutuyu temsil eder.
    // Bu kutu, eksenlere paralel olacak şekilde tanımlanır ve genellikle çarpışma tespiti ve hızlandırıcı yapılar için kullanılır.


public:
    // Boş sentinel değeri: Henüz hiçbir noktayla büyütülmemiş bir AABB başlangıç durumu için kullanılır.
    // min = +sonsuz ve max = -sonsuz olarak ayarlanır, böylece herhangi bir nokta eklenirse AABB büyütülür.
    // İlk grow() çağrısı ile AABB, eklenen nokta etrafında doğru şekilde büyütülür.
    
    // std::min(+inf, x) = x ve std::max(-inf, x) = x olduğundan, sentinel değerleri AABB'nin ilk nokta ile doğru şekilde büyümesini sağlar.
    AABB() : min(Vec3(std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::infinity())),
             max(Vec3(-std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity())) {}

    AABB(const Point3& a, const Point3& b) : min(a), max(b) {}

    // AABB'yi bir nokta ile büyütür: min ve max köşe noktalarını günceller.
    void grow(const Point3& p) {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);
    };

    // AABB'yi bir başka AABB ile büyütür: min ve max köşe noktalarını günceller.
    void grow(const AABB& b) {
        if (b.min.x > b.max.x) return; // b tamamen bu AABB'nin dışında, büyütmeye gerek yok
        grow(b.min);
        grow(b.max);
    };

    // AABB'nin eksenler boyunca en uzun kenarını döndürür: 0 = x, 1 = y, 2 = z
    int longest_axis() const {
        Vec3 diag = max - min;
        if (diag.x > diag.y && diag.x > diag.z) return 0; // x ekseni en uzun
        if (diag.y > diag.z) return 1;                     // y ekseni en uzun
        return 2;                                          // z ekseni en uzun
    }

    // AABB'nin yarım yüzey alanını hesaplar: (dx*dy + dy*dz + dz*dx)
    double surface_area() const {
        Vec3 d = max- min; // AABB'nin boyutlarını hesapla,
        if (d.x < 0) return 0.0; // negatif boyut, geçersiz AABB, sentinel boş kutu koruması
        return d.x * d.y + d.y * d.z + d.z * d.x; // yüzey alanı formülü(yarım)
    }

    // AABB'nin bir ışın ile kesişip kesişmediğini kontrol eder.
    bool hit(const Ray& r, double t_min, double t_max) const {
        for (int a = 0; a < 3; a++) {
            double invD = 1.0 / r.direction[a];
            double t0 = (min[a] - r.origin[a]) * invD;
            double t1 = (max[a] - r.origin[a]) * invD;
            if (invD < 0.0) std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min) return false;
        }
        return true;
    }
};