#pragma once
#include "ray.h"
#include <cmath>

class Camera {
public:
    Point3 origin; // Kameranın konumu
    Point3 lower_left_corner; // Görüntü düzleminin sol alt köşesi
    Vec3 horizontal; // Görüntü düzleminin yatay vektörü
    Vec3 vertical; // Görüntü düzleminin dikey vektörü

    Camera() {
        double aspect_ratio = 16.0 / 9.0; // Görüntü oranı
        double viewport_height = 2.0; // Görüntü düzleminin yüksekliği
        double viewport_width = aspect_ratio * viewport_height; // Görüntü düzleminin genişliği
        double focal_length = 1.0; // Odak uzaklığı

        origin = Point3(0, 0, 0); // Kamera orijinal konumu
        horizontal = Vec3(viewport_width, 0, 0); // Yatay vektör
        vertical = Vec3(0, viewport_height, 0); // Dikey vektör
        lower_left_corner = origin 
                        - horizontal/2 
                        - vertical/2 
                        - Vec3(0, 0, focal_length); // Görüntü düzleminin sol alt köşesi 
    }

    Ray get_ray(double u, double v) const {
        Vec3 direction = lower_left_corner + u*horizontal + v*vertical - origin; // Işının yönü
        return Ray(origin, direction); // Yeni bir ışın oluştur ve döndür
    }

    // Animasyon için yeni constructor ekleyelim
    Camera(Point3 look_from, Point3 look_at, Vec3 vup, double vfov, double aspect)
    {

        //Görüntü düzleminin boyutlarını hesaplayalım
        double theta = vfov * M_PI / 180.0; // Radyan cinsine çevir
        double h = std::tan(theta/2); // Görüntü düzleminin yarı yüksekliği
        double viewport_height = 2.0 * h; // Görüntü düzleminin yüksekliği
        double viewport_width = aspect * viewport_height; // Görüntü düzleminin genişliği

        // Ortonormal baz oluşturmak için gerekli vektörleri hesaplayalım
        Vec3 w = (look_from - look_at); // Kameranın baktığı yönün tersine (görüş yönü)
        w = w/w.length(); // Birim vektör yap

        Vec3 r = vup.cross(w); // "right" yönü
        r = r/r.length(); // Birim vektör yap

        Vec3 u_up = w.cross(r); // "up" yönü, ortonormal bazın üçüncü vektörü
    
        // Member değişkenlerini hesaplayalım
        origin = look_from; // Kameranın konumu
        horizontal = viewport_width * r; // Yatay vektör
        vertical = viewport_height * u_up; // Dikey vektör
        lower_left_corner = origin - horizontal/2 - vertical/2 - w; // Görüntü düzleminin sol alt köşesi
    }
};
