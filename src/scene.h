#pragma once
#include "hittable.h"
#include <vector>

class Scene : public Hittable {
    std::vector<Hittable*> objects; // Sahnedeki tüm nesneleri tutan vektör
public:
    void add(Hittable* object) { objects.push_back(object); } // Sahneye yeni bir nesne ekler
    
    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override 
    {
        bool hit_anything = false; // Hiçbir nesneye çarpmadıysa false kalır
        double closest_so_far = t_max; // Şu ana kadar bulunan en yakın çarpışma mesafesi
        HitRecord temp_rec; // Geçici çarpışma kaydı
        for(Hittable* object : objects) { // Sahnedeki her nesne için
            if(object->hit(ray, t_min, closest_so_far, temp_rec)) { // Eğer ışın bu nesneye çarparsa
                hit_anything = true; // Çarpışma oldu
                closest_so_far = temp_rec.t; // En yakın çarpışma mesafesini güncelle
                rec = temp_rec; // Çarpışma kaydını güncelle
            }
        }
        return hit_anything; // Hiçbir nesneye çarpmadıysa false, çarptıysa true döndür
    }
};