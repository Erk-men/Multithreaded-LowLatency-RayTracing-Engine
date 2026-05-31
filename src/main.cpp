#include <cstdio>
#include "camera.h"
#include "Sphere.h"
#include "scene.h"
#include "renderer.h"
#include "renderer_v2.h"
#include "material.h"
#include "ppm.h"


void build_scene_simple(Scene& scene, 
                        Lambertian& mat_zemin, 
                        Lambertian& mat_orta,
                        Sphere* spheres,
                        int& sphere_count) {
    // 2 küre: merkez + zemin
    spheres[sphere_count++] = Sphere(Vec3(0, -100.5, -1), 100, &mat_zemin); // Zemin küresi oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Zemini sahneye ekle
    spheres[sphere_count++] = Sphere(Vec3(0, 0, -1), 0.5, &mat_orta); // Orta küre oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Orta küreyi sahneye ekle
}

void build_scene_medium(Scene& scene, 
                        Lambertian& mat_zemin, 
                        Lambertian& mat_orta, 
                        Lambertian& mat_sol, 
                        Metal& mat_sag,
                        Lambertian& mat_ust, 
                        int& sphere_count,
                        Sphere* spheres) {
    // 5 küre: 3 Lambertian + zemin
    spheres[sphere_count++] = Sphere(Vec3(0, -100.5, -1), 100, &mat_zemin); // Zemin küresi oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Zemini sahneye ekle
    spheres[sphere_count++] = Sphere(Vec3(0, 0, -1), 0.5, &mat_orta); // Orta küre oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Orta küreyi sahneye ekle
    spheres[sphere_count++] = Sphere(Vec3(-1, 0, -1), 0.5, &mat_sol); // Sol küre oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Sol küreyi sahneye ekle
    spheres[sphere_count++] = Sphere(Vec3(1, 0, -1), 0.5, &mat_sag); // Sağ küre oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Sağ küreyi sahneye ekle
    spheres[sphere_count++] = Sphere(Vec3(0, 1, -1), 0.5, &mat_ust); // Üst küre oluştur ve diziye ekle
    scene.add(&spheres[sphere_count - 1]); // Üst küreyi sahneye ekle
}
    

void build_scene_complex(Scene& scene, 
                        Lambertian& mat_zemin, 
                        Lambertian& mat_orta, 
                        Lambertian& mat_sol, 
                        Metal& mat_sag,
                        Lambertian& mat_ust, 
                        Sphere* spheres,
                        int& sphere_count,
                        Lambertian* lambertians,
                        int& lambertian_count) {
    // 200 küre: 5 ana + 195 rastgele dağıtılmış küçük küre
    build_scene_medium(scene, mat_zemin, mat_orta, mat_sol, mat_sag, mat_ust, sphere_count, spheres); // Önce orta sahneyi oluştur
    for (int i = 0; i < 195; ++i) {
        double x = -5.0 + (rand() / (double)RAND_MAX) * 10.0; // -5 ile 5 arasında rastgele x koordinatı
        double y = -0.5 + (rand() / (double)RAND_MAX) * 2.0; // -0.5 ile 4.5 arasında rastgele y
        double z = -1.0 - (rand() / (double)RAND_MAX) * 10.0; // -1 ile -11 arasında rastgele z koordinatı
        double r = 0.1 + (rand() / (double)RAND_MAX) * 0.4; // 0.1 ile 0.5 arasında rastgele yarıçap
        lambertians[lambertian_count++] = Lambertian(Color(
            rand() / (double)RAND_MAX, // 0 ile 1 arasında rastgele kırmızı
            rand() / (double)RAND_MAX, // 0 ile 1 arasında rastgele yeşil
            rand() / (double)RAND_MAX));  // 0 ile 1 arasında rastgele mavi
        spheres[sphere_count++] = Sphere(Vec3(x, y, z), r, &lambertians[lambertian_count - 1]); // Rastgele küre oluştur ve diziye ekle
        scene.add(&spheres[sphere_count - 1]); // Küreyi sahneye ekle
    }
}

int main() {
    Sphere spheres[210]; // Küreler için bir dizi oluştur
    int sphere_count = 0; // Küre sayısını takip etmek için bir sayaç
    Lambertian lambertians[200]; // Lambertian malzemeler için bir dizi oluştur
    int lambertian_count = 0; // Lambertian malzeme sayısını takip etmek için bir sayaç
    Lambertian mat_zemin(Color(0.3, 0.7, 0.2));
    Lambertian mat_orta(Color(0.8, 0.3, 0.3));
    Lambertian mat_sol(Color(0.1, 0.2, 0.8));
    Metal      mat_sag(Color(0.8, 0.8, 0.8), 0.1);
    Lambertian mat_ust(Color(0.8, 0.6, 0.2));

    // OUTPUT LAR
    // v1 için
    //FILE* out = fopen("output_v1.ppm", "w"); // Render sonucunu output.ppm dosyasına yazmak için dosya açılır
    
    // v2 için
    FILE* out = fopen("output_v2.ppm", "w"); // Render sonucunu output_v2.ppm dosyasına yazmak için dosya açılır

    int width = 1280; // Görüntü genişliği, 16:9 oranına göre hesaplanır
    int height = static_cast<int>(width / (16.0 / 9.0)); // Görüntü yüksekliği, 16:9 oranına göre hesaplanır
    Camera cam; // Kamera oluştur
    Scene scene; // Sahne oluştur

    // BUILD SCENE LER

    build_scene_simple(scene, mat_zemin, mat_orta, spheres, sphere_count); // Basit sahne oluştur
    //build_scene_medium(scene, mat_zemin, mat_orta, mat_sol, mat_sag, mat_ust, sphere_count, spheres); // Orta sahne oluştur
    //build_scene_complex(scene, mat_zemin, mat_orta, mat_sol, mat_sag, mat_ust, spheres, sphere_count, lambertians, lambertian_count); // Karmaşık sahne oluştur
    
    // RENDERERLER 

    // RENDERER V1

    // Renderer v1 oluştur: 1280x720 , simple için 16 5, medium için 16 10, complex için 64 15 önerilir
    //Renderer renderer(width, height, 64, 15); 
    //long long ms = renderer.render(scene, cam, out); // Render işlemini başlat ve sonucu standart çıktıya yaz 
    //fprintf(stderr, "Render süresi: %lld ms\n", ms); // Render süresini standart hataya yaz
    //fclose(out); // Dosyayı kapat

    // RENDERER V2
    RendererV2 renderer_v2(width, height, 16, 5);
    long long ms_v2 = renderer_v2.render(scene, cam, out); // Render işlemini başlat ve sonucu standart çıktıya yaz 
    fprintf(stderr, "Render süresi (V2): %lld ms\n", ms_v2); // Render süresini standart hataya yaz
    fclose(out); // Dosyayı kapat
    
    
    return 0;
}
