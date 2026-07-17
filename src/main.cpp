#include "camera.h"
#include "Sphere.h"
#include "scene.h"
#include "renderer.h"
#include "material.h"
#include "ppm.h"
#include "threadpool.h"
#include "progress.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <filesystem>
#include <vector>


// --- Scene builder storage caps (FIX-01 / D-02) --------------------------------
// Scene stores NON-OWNING raw Hittable* into these vectors (scene.h:6). A
// std::vector reallocation would dangle every pointer already handed to Scene via
// scene.add(&spheres.back()). We therefore reserve() both vectors UP FRONT to a
// documented cap >= each builder's known maximum object count, so no reallocation
// ever happens after the first &element is taken. The 'complex' builder is the
// max consumer: build_scene_medium's 5 spheres + the 850-iteration loop
// => 855 spheres and 850 lambertians. Caps are rounded up with a small margin.
constexpr std::size_t COMPLEX_SCENE_MAX_SPHERES     = 860;
constexpr std::size_t COMPLEX_SCENE_MAX_LAMBERTIANS = 850;

void build_scene_simple(Scene& scene,
                        Lambertian& mat_zemin,
                        Lambertian& mat_orta,
                        std::vector<Sphere>& spheres) {
    // 2 küre: merkez + zemin
    spheres.push_back(Sphere(Vec3(0, -100.5, -1), 100, &mat_zemin)); // Zemin küresi oluştur ve diziye ekle
    scene.add(&spheres.back()); // Zemini sahneye ekle
    spheres.push_back(Sphere(Vec3(0, 0, -1), 0.5, &mat_orta)); // Orta küre oluştur ve diziye ekle
    scene.add(&spheres.back()); // Orta küreyi sahneye ekle
}

void build_scene_medium(Scene& scene,
                        Lambertian& mat_zemin,
                        Lambertian& mat_orta,
                        Lambertian& mat_sol,
                        Metal& mat_sag,
                        Lambertian& mat_ust,
                        std::vector<Sphere>& spheres) {
    // 5 küre: 3 Lambertian + zemin
    spheres.push_back(Sphere(Vec3(0, -100.5, -1), 100, &mat_zemin)); // Zemin küresi oluştur ve diziye ekle
    scene.add(&spheres.back()); // Zemini sahneye ekle
    spheres.push_back(Sphere(Vec3(0, 0, -1), 0.5, &mat_orta)); // Orta küre oluştur ve diziye ekle
    scene.add(&spheres.back()); // Orta küreyi sahneye ekle
    spheres.push_back(Sphere(Vec3(-1, 0, -1), 0.5, &mat_sol)); // Sol küre oluştur ve diziye ekle
    scene.add(&spheres.back()); // Sol küreyi sahneye ekle
    spheres.push_back(Sphere(Vec3(1, 0, -1), 0.5, &mat_sag)); // Sağ küre oluştur ve diziye ekle
    scene.add(&spheres.back()); // Sağ küreyi sahneye ekle
    spheres.push_back(Sphere(Vec3(0, 1, -1), 0.5, &mat_ust)); // Üst küre oluştur ve diziye ekle
    scene.add(&spheres.back()); // Üst küreyi sahneye ekle
}


void build_scene_complex(Scene& scene,
                        Lambertian& mat_zemin,
                        Lambertian& mat_orta,
                        Lambertian& mat_sol,
                        Metal& mat_sag,
                        Lambertian& mat_ust,
                        std::vector<Sphere>& spheres,
                        std::vector<Lambertian>& lambertians) {
    // 855 küre: 5 ana + 850 rastgele dağıtılmış küçük küre
    build_scene_medium(scene, mat_zemin, mat_orta, mat_sol, mat_sag, mat_ust, spheres); // Önce orta sahneyi oluştur
    for (int i = 0; i < 850; ++i) {
        double x = -5.0 + (rand() / (double)RAND_MAX) * 10.0; // -5 ile 5 arasında rastgele x koordinatı
        double y = -0.5 + (rand() / (double)RAND_MAX) * 2.0; // -0.5 ile 4.5 arasında rastgele y
        double z = -1.0 - (rand() / (double)RAND_MAX) * 10.0; // -1 ile -11 arasında rastgele z koordinatı
        double r = 0.1 + (rand() / (double)RAND_MAX) * 0.4; // 0.1 ile 0.5 arasında rastgele yarıçap
        lambertians.push_back(Lambertian(Color(
            rand() / (double)RAND_MAX, // 0 ile 1 arasında rastgele kırmızı
            rand() / (double)RAND_MAX, // 0 ile 1 arasında rastgele yeşil
            rand() / (double)RAND_MAX)));  // 0 ile 1 arasında rastgele mavi
        spheres.push_back(Sphere(Vec3(x, y, z), r, &lambertians.back())); // Rastgele küre oluştur ve diziye ekle
        scene.add(&spheres.back()); // Küreyi sahneye ekle
    }
}



struct Args {
    // animasyon için
    int frames = 72; // Toplam kare sayısı (örneğin, 72 kare = 5 saniye animasyon @ 15 FPS)
    
    
    int         threads = std::thread::hardware_concurrency(); // Varsayılan olarak mevcut CPU çekirdeği sayısı kadar thread kullan
    std::string mode    = "single"; // "single" (tek thread), "naive" (satır bazında thread), "tile" (blok bazında thread) gibi modlar olabilir
    std::string scene   = "simple"; // "simple" (2 küre), "medium" (5 küre), "complex" (200 küre) gibi sahne seçenekleri
    int         width   = 1280; // Görüntü genişliği
    int         height  = 720; // Görüntü yüksekliği
    int         samples = 4; // Piksel başına örnek sayısı (antialiasing için)
    int         depth   = 5;  // Işınların maksimum yansıma derinliği
    std::string output  = "output.ppm"; 
};

// -----------------------------------------------------------------------------
// CLI hata konvansiyonu (FIX-08/09/10, D-05)
//
// D-05: Bozuk CLI girdisi (sayisal olmayan --width abc, aralik disi --width 0 /
// --samples 0 / --depth -5, bilinmeyen bayrak) HARD-FAIL eder: stderr'e temiz bir
// mesaj + exit(1). Sessiz clamp/deger ikamesi yok — standart Unix CLI konvansiyonu.
// (Tek istisna: --threads ust siniri, bkz. FIX-06 / D-06 clamp.)
//
// Not: src/ icindeki ILK exit-tabanli CLI hard-fail'i. Plan 01-01'in ThreadPool
// throw'u (D-07) ile ayni "sessizce duzeltme, gurultuyle basarisiz ol" ruhunu
// izler; burada exit(1) kullaniliyor cunku parse_args deger dondurur, int degil.
// -----------------------------------------------------------------------------
[[noreturn]] static void cli_fail(const std::string& msg) {
    std::cerr << "hata: " << msg << "\n"
              << "kullanim: raytracer [--threads N] [--mode M] [--scene S] "
                 "[--width W] [--height H] [--samples N] [--depth D] "
                 "[--output DOSYA] [--frames N] [--animate] [--timelapse]\n";
    std::exit(1);
}

// Tek bir sayisal argumani guvenli ayristir (FIX-08, D-05).
// Ham std::stoi, --width abc gibi girdide yakalanmamis std::invalid_argument
// atar (terminate + abort). Burada try/catch ile temiz stderr + exit(1)'e
// cevriliyor. Ayrica pos kontrolu ile "12abc" gibi arta kalan cop reddediliyor
// (std::stoi bunu sessizce 12 olarak kabul ederdi).
static int cli_parse_int(const std::string& flag, const char* value) {
    try {
        std::size_t pos = 0;
        int result = std::stoi(value, &pos);
        if (value[pos] != '\0')                       // "12abc" -> arta kalan cop
            cli_fail(flag + " tam sayi olmalidir: " + value);
        return result;
    } catch (const std::invalid_argument&) {
        cli_fail(flag + " tam sayi olmalidir: " + value);
    } catch (const std::out_of_range&) {
        cli_fail(flag + " sayi araligi disinda: " + value);
    }
}

// Komut satırı argümanlarını ayrıştıran fonksiyon
Args parse_args(int argc, char* argv[]) {
    Args a; // Varsayılan değerlerle başlat
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg == "--threads" && i+1 < argc) a.threads = cli_parse_int("--threads", argv[++i]);
        else if (arg == "--mode"    && i+1 < argc) a.mode    = argv[++i];
        else if (arg == "--scene"   && i+1 < argc) a.scene   = argv[++i];
        else if (arg == "--width"   && i+1 < argc) a.width   = cli_parse_int("--width",   argv[++i]);
        else if (arg == "--height"  && i+1 < argc) a.height  = cli_parse_int("--height",  argv[++i]);
        else if (arg == "--samples" && i+1 < argc) a.samples = cli_parse_int("--samples", argv[++i]);
        else if (arg == "--depth"   && i+1 < argc) a.depth   = cli_parse_int("--depth",   argv[++i]);
        else if (arg == "--output"  && i+1 < argc) a.output  = argv[++i];
        else if (arg == "--frames"  && i+1 < argc) a.frames  = cli_parse_int("--frames",  argv[++i]);
    }

    // Anlamsal aralik dogrulamasi (FIX-09, D-05). Bu alt sinirlar
    // render_tile'daki tehlikeli bolme yollarini kapatir:
    //   u = (i + rnd) / (cfg.width - 1)  -> width==1 iken sifira bolme
    //   avg = pixel_color / cfg.samples  -> samples==0 iken NaN, sonra
    //                                       PPM yazicida tanimsiz float->int cast (UB)
    // depth<0 sonsuz/anlamsiz ozyineleme sinirini bozar; threads<1 sifir/negatif
    // pool demektir (FIX-07 ile tutarli, D-07 defense-in-depth).
    if (a.width   < 2) cli_fail("--width en az 2 olmalidir");
    if (a.height  < 2) cli_fail("--height en az 2 olmalidir");
    if (a.samples < 1) cli_fail("--samples en az 1 olmalidir");
    if (a.depth   < 0) cli_fail("--depth negatif olamaz");
    if (a.frames  < 1) cli_fail("--frames en az 1 olmalidir");
    if (a.threads < 1) cli_fail("--threads en az 1 olmalidir");

    return a;
}


int main(int argc, char* argv[]) {
      Args args = parse_args(argc, argv);
      RenderConfig cfg{args.width, args.height, args.samples, args.depth};
      // Materyal ve sahne kurulumu (mevcut koddan taşı)
      // FIX-01 (D-01/D-02): sabit C-dizileri yerine std::vector; Scene non-owning
      // Hittable* tuttuğu için reallocation'ı önlemek üzere ilk &element alınmadan
      // ÖNCE reserve() ile kapasite ayrılıyor (belgelenmiş constexpr cap).
      std::vector<Sphere>     spheres;
      std::vector<Lambertian> lambertians;
      spheres.reserve(COMPLEX_SCENE_MAX_SPHERES);
      lambertians.reserve(COMPLEX_SCENE_MAX_LAMBERTIANS);
      Lambertian mat_zemin(Color(0.3, 0.7, 0.2));
      Lambertian mat_orta (Color(0.8, 0.3, 0.3));
      Lambertian mat_sol  (Color(0.1, 0.2, 0.8));
      Metal      mat_sag  (Color(0.8, 0.8, 0.8), 0.1);
      Lambertian mat_ust  (Color(0.8, 0.6, 0.2));

      Scene  scene;
      Camera cam;

    if      (args.scene == "simple")  build_scene_simple(scene, mat_zemin,
        mat_orta, spheres);
    else if (args.scene == "medium")  build_scene_medium(scene, mat_zemin,
        mat_orta, mat_sol, mat_sag, mat_ust, spheres);
    else                              build_scene_complex(scene, mat_zemin,
        mat_orta, mat_sol, mat_sag, mat_ust, spheres, lambertians);
  
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
        
    } else if (args.mode == "pool" || args.mode == "unaligned") {
      const int tile_w = 128, tile_h = 128;
      int tiles_x = (cfg.width  + tile_w - 1) / tile_w;
      int tiles_y = (cfg.height + tile_h - 1) / tile_h;
      int total_tiles = tiles_x * tiles_y;

      ThreadPool  pool(args.threads);
      ProgressBar progress(total_tiles, args.threads);
      progress.start();

      for (int ty = 0; ty < cfg.height; ty += tile_h) {
          for (int tx = 0; tx < cfg.width; tx += tile_w) {
              int x0 = tx, y0 = ty;
              int x1 = std::min(tx + tile_w, cfg.width);
              int y1 = std::min(ty + tile_h, cfg.height);
              pool.submit([&, x0, y0, x1, y1] {
                  render_tile(x0, y0, x1, y1, scene, cam, writer, cfg);
                  int idx = ThreadPool::this_thread_idx();
                  if (idx >= 0 && idx < args.threads)
                      progress.increment(idx);
              });
          }
      }
      pool.shutdown();
      progress.stop();

    } else if (args.mode == "aligned") {
        const int tile_w = 64, tile_h = 64;
        int tiles_x = (cfg.width  + tile_w - 1) / tile_w;
        int tiles_y = (cfg.height + tile_h - 1) / tile_h;
        int total_tiles = tiles_x * tiles_y;

        ThreadPool         pool(args.threads);
        AlignedProgressBar progress(total_tiles, args.threads);
        progress.start();

        for (int ty = 0; ty < cfg.height; ty += tile_h) {
            for (int tx = 0; tx < cfg.width; tx += tile_w) {
                int x0 = tx, y0 = ty;
                int x1 = std::min(tx + tile_w, cfg.width);
                int y1 = std::min(ty + tile_h, cfg.height);
                pool.submit([&, x0, y0, x1, y1] {
                    render_tile(x0, y0, x1, y1, scene, cam, writer, cfg);
                    int idx = ThreadPool::this_thread_idx();
                    if (idx >= 0 && idx < args.threads)
                    progress.increment(idx);
                });
            }
        }
        pool.shutdown();
        progress.stop();

    } else if (args.mode == "animate") {
        std::filesystem::create_directories("frames");

        double radius = 3.0;
        double cam_height = 1.0;
        double aspect = (double)args.width / args.height;
        RenderConfig frame_cfg{args.width, args.height, args.samples, args.depth};
        const int tile = 64;

        for (int i = 0; i < args.frames; ++i) {
            double angle = 2.0 * M_PI * i / args.frames;
            Point3 from(radius * std::cos(angle), cam_height, radius * std::sin(angle) - 1.0);
            Point3 at(0, 0, -1);
            Camera cam_frame(from, at, Vec3(0, 1, 0), 45.0, aspect);

            PPMWriter frame_writer(args.width, args.height);
            int tiles_x = (args.width  + tile - 1) / tile;
            int tiles_y = (args.height + tile - 1) / tile;
            ThreadPool pool(args.threads);

            for (int ty = 0; ty < tiles_y; ++ty)
                for (int tx = 0; tx < tiles_x; ++tx) {
                    int x0 = tx * tile, x1 = std::min(x0 + tile, args.width);
                    int y0 = ty * tile, y1 = std::min(y0 + tile, args.height);
                    pool.submit([&, x0, y0, x1, y1] {
                        render_tile(x0, y0, x1, y1, scene, cam_frame, frame_writer, frame_cfg);
                    });
                }
            pool.shutdown();

            char fname[64];
            std::snprintf(fname, sizeof(fname), "frames/frame_%03d.ppm", i);
            frame_writer.save(fname);
            std::cout << "Kare " << i + 1 << "/" << args.frames << " -> " << fname << "\n";
        }

    } 
    else if (args.mode == "animate-single") {
    std::filesystem::create_directories("frames");

    double radius = 3.0;
    double cam_height = 1.0;
    double aspect = (double)args.width / args.height;
    RenderConfig frame_cfg{args.width, args.height, args.samples, args.depth};

    for (int i = 0; i < args.frames; ++i) {
        double angle = 2.0 * M_PI * i / args.frames;
        Point3 from(radius * std::cos(angle), cam_height, radius * std::sin(angle) - 1.0);
        Point3 at(0, 0, -1);
        Camera cam_frame(from, at, Vec3(0, 1, 0), 45.0, aspect);

        PPMWriter frame_writer(args.width, args.height);
        render_tile(0, 0, args.width, args.height, scene, cam_frame, frame_writer, frame_cfg);

        char fname[64];
        std::snprintf(fname, sizeof(fname), "frames/frame_%03d.ppm", i);
        frame_writer.save(fname);
        std::cout << "Kare " << i + 1 << "/" << args.frames << " -> " << fname << "\n";
    }
    
    } else {
        std::cerr << "Bilinmeyen mod: " << args.mode << "\n";
        return 1;
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double ms  = std::chrono::duration<double, std::milli>(t_end -
    t_start).count();
    std::cout << "Tamamlandi: " << ms << " ms\n";
      
    if (args.mode != "animate") {
        writer.save(args.output);
        std::cout << "Kaydedildi: " << args.output << "\n";
    }
    return 0;
}   