#pragma once
#include "vec3.h"
#include "ray.h"

// =============================================================================
// HitRecord — Kesişim Bilgisi
//
// Bir ışın bir nesneyle kesiştiğinde ihtiyaç duyduğumuz tüm bilgiyi tutar.
// Renderer bu bilgiyi alıp aydınlatma hesabında kullanır.
// =============================================================================

struct HitRecord {
    Point3 point;       // Dünya uzayında kesişim noktası
    Vec3   normal;      // Yüzey normali (her zaman normalize, ışına karşı bakan)
    double t;           // Işın parametresi — küçük t → kameraya yakın
    bool   front_face;  // Işın dışarıdan mı içeriden mi çarptı?

    // Normal'i ışına göre ayarla.
    // Kural: normal HER ZAMAN ışına karşı baksın (dot(ray_dir, normal) < 0).
    // Böylece aydınlatma hesabı her zaman aynı formülle çalışır.
    //
    // outward_normal: nesnenin dışa bakan normali (küre için merkez→nokta)
    void set_face_normal(const Ray& ray, const Vec3& outward_normal) {
        front_face = Vec3::dot(ray.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// =============================================================================
// Hittable — Soyut Nesne Arayüzü
//
// Sahnedeki her nesne (küre, düzlem, üçgen...) bu arayüzü uygular.
// Renderer sadece Hittable* listesiyle çalışır — nesne tipini bilmesi gerekmez.
// Bu klasik OOP polimorfizmi: "program arayüze karşı, implementasyona değil."
// =============================================================================

class Hittable {
public:
    virtual ~Hittable() = default;

    // Işın bu nesneyle [t_min, t_max] aralığında kesişiyor mu?
    // Kesişiyorsa rec'e bilgileri yaz ve true döndür.
    // t_min > 0 kullanmak "kendine çarpma" hatasını önler (shadow acne).
    virtual bool hit(const Ray& ray, double t_min, double t_max,
                     HitRecord& rec) const = 0;
};
