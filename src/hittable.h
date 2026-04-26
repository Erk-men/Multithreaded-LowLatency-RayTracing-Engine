#pragma once                                                                          
#include "ray.h"                                                                      

// Çarpışma bilgisi: nokta, normal, t
struct HitRecord {                                                                    
    Point3 point;      // çarpışma noktasının 3D koordinatı                                                          
    Vec3   normal;     // yüzeyin o noktadaki dik vektörü                                                             
    double t;          // ışın parametresi, ne kadar uzakta çarptı                                                           
};                                                                                    
                         
// Soyut arayüz, hit() saf sanal
class Hittable {                                                                      
    public:                                                                          
        virtual bool hit(const Ray& ray, double t_min, double t_max, // geçerli aralık: arkayı ve uzağı eler
            HitRecord& rec) const = 0; //HitRecord& referansla doldurur, kopyalanmaz
        virtual ~Hittable() = default;                                                    
};