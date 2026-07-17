#pragma once
#include "hittable.h"
#include <algorithm>

class Material {
    public:
    
    virtual bool scatter(const Ray& ray_in, 
        const HitRecord& rec, Vec3& attenuation, Ray& scattered) const = 0;
    virtual ~Material() = default;
};

class Lambertian : public Material {
    public:
    Lambertian() : albedo(Vec3(0,0,0)) {}
    Vec3 albedo; // yüzeyin rengi

    Lambertian(const Vec3& a) : albedo(a) {}

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, 
        Vec3& attenuation, Ray& scattered) const override {
        Vec3 scatter_direction = rec.normal + Vec3::random_unit_vector();
        scattered = Ray(rec.point, scatter_direction);
        attenuation = albedo;
        return true;
    }
};

class Metal : public Material {
    public:
    Vec3 albedo; // yüzeyin rengi
    double fuzz; // yüzeyin pürüzlülüğü (0 = mükemmel yansıtıcı, 1 = tamamen dağınık)

    // FIX-15 (D — Claude's discretion): iki taraflı clamp. Eski `f < 1 ? f : 1`
    // yalnızca üst sınırı kısıtlıyordu; negatif fuzz shading'e ulaşabiliyordu.
    Metal(const Vec3& a, double f) : albedo(a), fuzz(std::max(0.0, std::min(f, 1.0))) {}

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, 
        Vec3& attenuation, Ray& scattered) const override {
        Vec3 reflected = ray_in.direction.normalize().reflect(rec.normal);
        scattered = Ray(rec.point, reflected + Vec3::random_unit_vector() * fuzz);
        attenuation = albedo;
        return (scattered.direction.dot(rec.normal) > 0);
    }
};