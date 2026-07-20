#pragma once
#include "ray.h"
#include "aabb.h"

class Material;  // forward declaration — tam tanım material.h'da

struct HitRecord {
    Point3 point;
    Vec3   normal;
    double t;
    bool   front_face;           // ışın dışarıdan mı içeriden mi çarptı
    Material* mat_ptr;           // bu yüzeyin materyali
  };

  class Hittable {
        public:
        virtual bool hit(const Ray& ray, double t_min, double t_max,
            HitRecord& rec) const = 0;
        virtual ~Hittable() = default;

        // Bounding box (AABB) hesaplaması için saf sanal fonksiyon

        virtual AABB bounding_box() const = 0;
    };

    