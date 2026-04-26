#pragma once
#include <cmath>

class Vec3 {
public:
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {} //initializer list, constructor bodyden önce üyeleri başlatır
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

      // İki vektörü bileşen bileşen toplar: (a+b, c+d, e+f)                                
    Vec3 operator+(const Vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
      // Bir vektörü diğerinden bileşen bileşen çıkarır                                     
    Vec3 operator-(const Vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
      // Vektörü negatife çevirir: yön tersine döner                                   
    Vec3 operator-()              const { return {-x, -y, -z}; }
      // Vektörü bir skaler (sayı) ile ölçekler: yönü korur, büyüklüğü değiştirir           
    Vec3 operator*(double t)      const { return {x*t, y*t, z*t}; }
      // Vektörü bir skalere böler                                                          
    Vec3 operator/(double t)      const { return {x/t, y/t, z/t}; }
    // Bileşen bazlı çarpma: renk × renk hesabında kullanılır (ör: kırmızı yüzey × sarı ışık)
    Vec3 operator*(const Vec3& v) const { return {x*v.x, y*v.y, z*v.z}; }


};