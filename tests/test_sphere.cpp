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
                                
    
    // Senaryo 2: Iskayan isın                                                              
    Ray r_miss(Point3(0, 0, 0), Vec3(1, 0, 0));
    HitRecord rec_miss;                                                                     
    bool miss = s.hit(r_miss, 0.001, 1e9, rec_miss);                                        
    assert(miss == false);                                                                  
    printf("PASS: miss\n"); 

    // Senaryo 3: t_max araligi disi
    bool out_of_range = s.hit(r, 0.001, 0.3, rec);                           
    assert(out_of_range == false);                                           
    printf("PASS: t_max\n");   

    // Senaryo 4: normal birim uzunluk
    double len = rec.normal.length();
    assert(len > 0.9999 && len < 1.0001);
    printf("PASS: normal\n");
    return 0;   
}