#include "camera.h"
#include "Sphere.h"
#include "scene.h"
#include "renderer.h"
#include "material.h"
#include "ppm.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>


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



struct Args {
    int         threads = std::thread::hardware_concurrency(); // Varsayılan olarak mevcut CPU çekirdeği sayısı kadar thread kullan
    std::string mode    = "single"; // "single" (tek thread), "naive" (satır bazında thread), "tile" (blok bazında thread) gibi modlar olabilir
    std::string scene   = "simple"; // "simple" (2 küre), "medium" (5 küre), "complex" (200 küre) gibi sahne seçenekleri
    int         width   = 1280; // Görüntü genişliği
    int         height  = 720; // Görüntü yüksekliği
    int         samples = 16; // Piksel başına örnek sayısı (antialiasing için)
    int         depth   = 5;  // Işınların maksimum yansıma derinliği
    std::string output  = "output.ppm"; 
};

// Komut satırı argümanlarını ayrıştıran fonksiyon
Args parse_args(int argc, char* argv[]) {  //
    Args a; // Varsayılan değerlerle başlat
    for (int i = 1; i < argc; ++i) { 
        std::string arg = argv[i]; 
        if      (arg == "--threads" && i+1 < argc) a.threads = 
    std::stoi(argv[++i]); //
        else if (arg == "--mode"    && i+1 < argc) a.mode    = argv[++i];
        else if (arg == "--scene"   && i+1 < argc) a.scene   = argv[++i];
        else if (arg == "--width"   && i+1 < argc) a.width   =
    std::stoi(argv[++i]);
        else if (arg == "--height"  && i+1 < argc) a.height  =
    std::stoi(argv[++i]);
        else if (arg == "--samples" && i+1 < argc) a.samples =
    std::stoi(argv[++i]);
        else if (arg == "--depth"   && i+1 < argc) a.depth   =
    std::stoi(argv[++i]);
        else if (arg == "--output"  && i+1 < argc) a.output  = argv[++i];
    }
    return a;
}


int main(int argc, char* argv[]) {
      Args args = parse_args(argc, argv);
      RenderConfig cfg{args.width, args.height, args.samples, args.depth};
      // Materyal ve sahne kurulumu (mevcut koddan taşı)
      Sphere     spheres[210];    int sphere_count    = 0;
      Lambertian lambertians[200]; int lambertian_count = 0;
      Lambertian mat_zemin(Color(0.3, 0.7, 0.2));
      Lambertian mat_orta (Color(0.8, 0.3, 0.3));
      Lambertian mat_sol  (Color(0.1, 0.2, 0.8));
      Metal      mat_sag  (Color(0.8, 0.8, 0.8), 0.1);
      Lambertian mat_ust  (Color(0.8, 0.6, 0.2));

      Scene  scene;
      Camera cam;

      if      (args.scene == "simple")  build_scene_simple(scene, mat_zemin,
  mat_orta, spheres, sphere_count);
      else if (args.scene == "medium")  build_scene_medium(scene, mat_zemin,
  mat_orta, mat_sol, mat_sag, mat_ust, sphere_count, spheres);
      else                              build_scene_complex(scene, mat_zemin,
  mat_orta, mat_sol, mat_sag, mat_ust, spheres, sphere_count, lambertians,
  lambertian_count); 
  
      PPMWriter writer(cfg.width, cfg.height);
      
      std::cout << "Mode: " << args.mode << " | Scene: " << args.scene
                << " | " << cfg.width << "x" << cfg.height
                << " | samples=" << cfg.samples << "\n";
                
      auto t_start = std::chrono::high_resolution_clock::now();

      if (args.mode == "single") {
          render_tile(0, 0, cfg.width, cfg.height, scene, cam, writer, cfg);

      } else if (args.mode == "naive") {
      std::thread* threads = new std::thread[cfg.height];
      for (int y = 0; y < cfg.height; ++y)
          threads[y] = std::thread([&, y] {
              render_tile(0, y, cfg.width, y + 1, scene, cam, writer, cfg);
          });
      for (int y = 0; y < cfg.height; ++y)
          threads[y].join();
      delete[] threads;
        
      } else {
          std::cerr << "Bilinmeyen mod: " << args.mode << "\n";
          return 1;
      }   
      
      auto t_end = std::chrono::high_resolution_clock::now();
      double ms  = std::chrono::duration<double, std::milli>(t_end -
  t_start).count();
      std::cout << "Tamamlandi: " << ms << " ms\n";
      
      writer.save(args.output);
      std::cout << "Kaydedildi: " << args.output << "\n";
      return 0;
  }   