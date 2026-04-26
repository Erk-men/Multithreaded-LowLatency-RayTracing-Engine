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

    // sqrt() pahalıdır. sadece karşılaştırma için karesi yeterli
    double length_squared() const { return x*x + y*y + z*z; }
    // Gerçek uzunluk: |v| = sqrt(x²+y²+z²)
    double length() const { return std::sqrt(length_squared()); }

    double dot(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; }

    Vec3 cross(const Vec3& v) const {                                                
        return 
        {
            y*v.z - z*v.y,                                                            
            z*v.x - x*v.z,                                                       
            x*v.y - y*v.x
        };
    }

    Vec3 normalize() const { return *this / length(); } // "bu nesnenin kendisi". Uzunluğuna bölerek birim vektör elde ediyoruz. 

    Vec3 reflect(const Vec3& n) const {                                                   
        return *this - n * (2.0 * dot(n));    
    }   
    static double clamp(double x, double lo, double hi) {
        if (x < lo) return lo;                                                            
        if (x > hi) return hi;                                                       
        return x;             
    }

    static double gamma_correct(double linear) {                                          
      return std::sqrt(clamp(linear, 0.0, 1.0));                                        
    }   
};

// 2.0 * v yazımına izin ver (v * 2.0 zaten var, bu simetri için)
inline Vec3 operator*(double t, const Vec3& v) { return v * t; }

// Tip takma adları — aynı tip, farklı anlam
using Point3 = Vec3;  // 3D konum
using Color  = Vec3;  // RGB renk