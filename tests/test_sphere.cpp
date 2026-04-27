#include <cassert>                                                                    
#include <cstdio>
#include "Sphere.h"

    int main() {
    Sphere s(Point3(0, 0, -1), 0.5);
    Ray r(Point3(0, 0, 0), Vec3(0, 0, -1));                                           
    HitRecord rec;                                                                    
                                                                                        
    bool hit = s.hit(r, 0.001, 1e9, rec);                                             
                  
    assert(hit == true);                                                              
    printf("PASS\n");
    return 0;                                                                         
}