#pragma once                                                                          
#include "vec3.h"                                                                     
                                                                                        
class Ray {                                                                           
    public:                                                                               
        Point3 origin;  // ışının başladığı nokta - kamera konumu                                                                 
        Vec3 direction;       // ışının gittiği yön                                           
                                                                                        
        Ray() {}        // boş constructor                                                             
        Ray(const Point3& origin, const Vec3& direction)  // kopyalama yok, sadece referans. Değiştirme yok, sadece okuma.                                
            : origin(origin), direction(direction) {}
                                                                                        
        Point3 at(double t) const {    // at(t) ile çarpışma noktasını hesaplaırz                            
            return origin + direction * t;                 
        }                                   
};