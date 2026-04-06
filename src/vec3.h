#pragma once
#include <cmath>
#include <iostream>

// =============================================================================
// Vec3 — 3 Boyutlu Vektör / Nokta / Renk
//
// Ray tracing'de 3 farklı kavram için tek tip kullanıyoruz:
//   - 3D Nokta    : sahne koordinatları (kamera, nesne merkezi, ışık)
//   - 3D Vektör   : yön (ışın yönü, yüzey normali)
//   - RGB Renk    : r, g, b değerleri [0.0, 1.0] aralığında
//
// Neden hepsi aynı tip? Matematiksel olarak hepsi (x, y, z) üçlüsü.
// Ayrım sadece kavramsal — kod tarafında tek sınıf yeterli.
// =============================================================================

class Vec3 {
public:
    double x, y, z;

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    // Renk kısayolu: r(), g(), b() — x/y/z ile tamamen aynı bellek alanı
    double r() const { return x; }
    double g() const { return y; }
    double b() const { return z; }

    // -------------------------------------------------------------------------
    // Temel Aritmetik Operatörler
    // -------------------------------------------------------------------------

    Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator-()              const { return {-x, -y, -z}; }

    // Skaler çarpma: vektörü bir sayıyla ölçekle
    // Kullanım: ışın yönünü t kadar ilerlet → origin + t * direction
    Vec3 operator*(double t)      const { return {x * t, y * t, z * t}; }
    Vec3 operator/(double t)      const { return {x / t, y / t, z / t}; }

    // Bileşen bazlı çarpma: renk × renk (materyal rengi × ışık rengi)
    // Örnek: kırmızı yüzey (1,0,0) × sarı ışık (1,1,0) = kırmızı (1,0,0)
    Vec3 operator*(const Vec3& v) const { return {x * v.x, y * v.y, z * v.z}; }

    // Atama operatörleri
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator*=(double t)      { x *= t;   y *= t;   z *= t;   return *this; }

    // -------------------------------------------------------------------------
    // Uzunluk (Magnitude)
    //
    // |v| = sqrt(x² + y² + z²)
    //
    // length_squared() ayrıca tanımlandı çünkü sqrt() pahalı bir işlem.
    // Sadece karşılaştırma yapıyorsak (hangi t daha küçük?) sqrt gerekmez.
    // -------------------------------------------------------------------------

    double length_squared() const { return x*x + y*y + z*z; }
    double length()         const { return std::sqrt(length_squared()); }

    // -------------------------------------------------------------------------
    // Dot Product (İç Çarpım)
    //
    // dot(a, b) = ax*bx + ay*by + az*bz = |a||b|cos(θ)
    //
    // Kullanım alanları:
    //   1. Işık açısı: dot(normal, ışık_yönü) → yüzeyin parlaklığı
    //      θ=0° → cos=1.0 → max parlaklık (dik gelen ışık)
    //      θ=90° → cos=0.0 → hiç ışık yok (teğet ışık)
    //   2. Işın-küre kesişimi: ikinci dereceden denklem katsayıları
    //   3. Yansıma vektörü hesabı
    // -------------------------------------------------------------------------

    static double dot(const Vec3& a, const Vec3& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    // -------------------------------------------------------------------------
    // Cross Product (Dış Çarpım)
    //
    // cross(a, b) = vektör a ve b'ye dik olan vektör
    // Büyüklüğü: |a||b|sin(θ)
    //
    // Kullanım: kamera koordinat sistemi kurmak için
    //   - "ileri" yönü ve "yukarı" yönünden "sağ" yönü türet
    //   - cross(forward, up) = right
    // -------------------------------------------------------------------------

    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    // -------------------------------------------------------------------------
    // Normalize (Birim Vektör)
    //
    // Yön vektörlerinin uzunluğu 1.0 olmalı — buna "unit vector" denir.
    // Neden? Çünkü dot(a,b) = cos(θ) olması için |a|=|b|=1 şart.
    // Işın yönleri, yüzey normalleri her zaman normalize edilmeli.
    // -------------------------------------------------------------------------

    static Vec3 normalize(const Vec3& v) {
        return v / v.length();
    }

    // -------------------------------------------------------------------------
    // Yansıma Vektörü
    //
    // Gelen ışın d, yüzey normali n ise yansıyan ışın:
    //   reflect(d, n) = d - 2 * dot(d, n) * n
    //
    // Geometrik yorum: d'nin n eksenindeki bileşenini ters çevir.
    // Kullanım: reflective (ayna gibi) materyallerde
    // -------------------------------------------------------------------------

    static Vec3 reflect(const Vec3& d, const Vec3& n) {
        return d - n * (2.0 * dot(d, n));
    }

    // -------------------------------------------------------------------------
    // Yardımcı
    // -------------------------------------------------------------------------

    // Renk değerlerini [0,1]'den [0,255]'e çevir (PPM çıktısı için)
    // Gamma düzeltmesi: sqrt() ile gamma=2 yaklaşımı — gerçekçi parlaklık
    Vec3 gamma_correct() const {
        return {std::sqrt(x), std::sqrt(y), std::sqrt(z)};
    }

    // Değerleri [0,1] aralığında tut (renk taşmasını önle)
    Vec3 clamp(double min = 0.0, double max = 1.0) const {
        auto cl = [min, max](double v) {
            return v < min ? min : (v > max ? max : v);
        };
        return {cl(x), cl(y), cl(z)};
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    }
};

// Skaler * Vec3 (t * v yazımına izin vermek için — v * t zaten var)
inline Vec3 operator*(double t, const Vec3& v) { return v * t; }

// Tip takma adları — okunabilirlik için
using Point3 = Vec3;   // 3D nokta
using Color  = Vec3;   // RGB renk
