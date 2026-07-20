#pragma once
#include "hittable.h"    
#include "vec3.h"
#include "aabb.h"
                                                                                        
   
class Sphere : public Hittable {                                                      
public:         
    Sphere() : center(Vec3(0,0,0)), radius(0), mat_ptr(nullptr) {}
    Point3 center; // kürenin merkezi
    double radius; // kürenin yarıçapı
    Material* mat_ptr; // kürenin materyali (örneğin, renk ve yansıtıcılık bilgisi)

    // constructor
    Sphere(const Point3& center, double radius, Material* mat) : center(center), radius(radius), mat_ptr(mat) {}                                                                                        
    
    // hit() override
    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
    Vec3 oc = ray.origin - center;               
    double a = ray.direction.dot(ray.direction); // ışının yönünün karesi
    double half_b = oc.dot(ray.direction);       // ışının yönü ile merkezden ışının başlangıç noktası arasındaki vektörün nokta çarpımı
    double c = oc.dot(oc) - radius*radius;           // merkezden ışının başlangıç noktasına olan uzaklığın karesi - yarıçapın karesi
    double discriminant = half_b*half_b - a*c; // diskriminant, köklerin var olup olmadığını belirler   
    if (discriminant < 0) return false; // kök yok, çarpışma yok
    double sqrt_disc = std::sqrt(discriminant); // diskriminantın karekökü
    // En yakın kökü bul, geçerli aralıkta olmalı
    double root = (-half_b - sqrt_disc) / a; // ilk kök (küçük olan)
    if (root < t_min || root > t_max) { // ilk kök geçerli değil, ikinci kökü dene
        root = (-half_b + sqrt_disc) / a; // ikinci kök (büyük olan)
        if (root < t_min || root > t_max) // ikinci kök de geçerli değil, çarpışma yok
            return false;

    }                      
    rec.t      = root;                                                                    
    rec.point  = ray.at(root);
    rec.normal = (rec.point - center) / radius;                                           
    rec.front_face = ray.direction.dot(rec.normal) < 0;
    if (!rec.front_face) rec.normal = -rec.normal;
    rec.mat_ptr = mat_ptr;
    return true;
}

    // bounding_box() override
    AABB bounding_box() const override {
        return AABB(center - Vec3(radius, radius, radius), center + Vec3(radius, radius, radius));
    }
};