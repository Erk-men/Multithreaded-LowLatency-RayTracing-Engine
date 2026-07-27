#include "camera.h"
#include "Sphere.h"
#include "scene.h"
#include "renderer.h"
#include "bvh.h"
#include "scene_builders.h"
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
#include <cstdio>
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
// Materyal cesitliligi (rand()%5==0 -> Metal): en kotu durumda 850 kurenin HEPSI
// Metal cikabilir (RNG sansina bagli, garanti degil ama teorik olarak mumkun) —
// reserve() bu ust siniri, "ortalama ~170" gibi beklenen degeri degil, karsilamali.
constexpr std::size_t COMPLEX_SCENE_MAX_METALS      = 850;

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
                        std::vector<Lambertian>& lambertians,
                        std::vector<Metal>& metals) {
    // 855 küre: 5 ana + 850 rastgele dağıtılmış küçük küre
    build_scene_medium(scene, mat_zemin, mat_orta, mat_sol, mat_sag, mat_ust, spheres); // Önce orta sahneyi oluştur
    for (int i = 0; i < 850; ++i) {
        double x = -5.0 + (rand() / (double)RAND_MAX) * 10.0; // -5 ile 5 arasında rastgele x koordinatı
        double y = -0.5 + (rand() / (double)RAND_MAX) * 2.0; // -0.5 ile 4.5 arasında rastgele y
        double z = -1.0 - (rand() / (double)RAND_MAX) * 10.0; // -1 ile -11 arasında rastgele z koordinatı
        double r = 0.1 + (rand() / (double)RAND_MAX) * 0.4; // 0.1 ile 0.5 arasında rastgele yarıçap

        // Materyal çeşitliliği: ~%20 ihtimalle Metal (ayna), aksi halde Lambertian (mat).
        // rand()%5==0 -> [0,5) araliginda 1 deger, yani 850 denemede BEKLENEN ~170 metal
        // kure — ama bu bir UST SINIR degil, sadece ortalama; RNG sansina gore teorik
        // olarak 850'sinin de Metal cikmasi mumkun. Bu yuzden `metals` de (asagida,
        // main()'de) 850'lik BEKLENEN degil, 850'lik EN KOTU DURUM kapasitesiyle
        // reserve() edilmeli — reserve-before-&element degismezi (main.cpp:22-31,
        // BVH Pitfall 1 ile ayni ders) burada da gecerli.
        Material* mat_ptr;
        if (rand() % 5 == 0) {
            metals.push_back(Metal(Color(
                0.5 + (rand() / (double)RAND_MAX) * 0.5, // 0.5-1.0 arasi parlak renkler (ayna icin daha gercekci)
                0.5 + (rand() / (double)RAND_MAX) * 0.5,
                0.5 + (rand() / (double)RAND_MAX) * 0.5),
                rand() / (double)RAND_MAX)); // fuzz: 0.0-1.0 rastgele puruzluluk
            mat_ptr = &metals.back();
        } else {
            lambertians.push_back(Lambertian(Color(
                rand() / (double)RAND_MAX, // 0 ile 1 arasında rastgele kırmızı
                rand() / (double)RAND_MAX, // 0 ile 1 arasında rastgele yeşil
                rand() / (double)RAND_MAX)));  // 0 ile 1 arasında rastgele mavi
            mat_ptr = &lambertians.back();
        }

        spheres.push_back(Sphere(Vec3(x, y, z), r, mat_ptr)); // Rastgele küre oluştur ve diziye ekle
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
    int         seed    = -1; // -1 = verilmedi, eski deterministik davranis (srand() cagrilmaz) korunur
    int         count = 1000; // --scene bench/clustered için kaç nesne uretileceği
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
                 "[--output DOSYA] [--frames N] [--animate] [--timelapse] [--seed N] [--count N]\n";
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
        else if (arg == "--seed"    && i+1 < argc) a.seed    = cli_parse_int("--seed", argv[++i]);
        else if (arg == "--count"   && i+1 < argc) a.count   = cli_parse_int("--count",   argv[++i]);
        else if (arg == "--frames"  && i+1 < argc) a.frames  = cli_parse_int("--frames",  argv[++i]);
        // FIX-11 (D-13): --animate / --timelapse artik taninan bayraklar; mode'u
        // dogrudan set ediyorlar (Makefile bunlari zaten geciriyor, parse_args
        // sadece yutmayi birakiyor). --mode'dan SONRA geldikleri icin mode'u en
        // son onlar belirler -> animate/timelapse kazanir.
        else if (arg == "--animate")   a.mode = "animate";
        else if (arg == "--timelapse") a.mode = "timelapse";
        // FIX-10 (D-05): tanimsiz her bayrak/deger sessizce yutulmak yerine
        // hard-fail eder — scriptlerdeki/benchmark'lardaki yazim hatalari
        // artik gizlice kaybolmuyor.
        else cli_fail("bilinmeyen arguman: " + arg);
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
    if (a.count < 1) cli_fail("--count en az 1 olmalidir");
    // FIX-06 (D-06): --threads N (N > MAX_THREADS) GECERSIZ DEGIL — bugunku sabit
    // 16-slotluk progress sayaci dizisini (progress.h) asan mesru bir istek. D-05
    // hard-fail'inin bilincli istisnasi: hard-fail yerine MAX_THREADS'e KIRP + uyar.
    // Bu kirpma, out-of-bounds yazmayi kapatan sey: ThreadPool::this_thread_idx()
    // args.threads-1'e kadar index dondurebilir; progress.increment(idx) ve
    // total_completed() per_thread[MAX_THREADS] dizisini indeksler. threads<=16
    // olunca mevcut 'idx < args.threads' guard'i progress.h'a dokunmadan guvenli olur.
    if (a.threads > MAX_THREADS) {
        std::cerr << "uyari: " << a.threads << " thread istendi, "
                  << MAX_THREADS << "'e kirpiliyor\n";
        a.threads = MAX_THREADS;
    }

    return a;
}

//thermal_color (mavi -> cam böceği -> yeşil -> sarı -> kirmizi) renk skalasi: 0.0 (mavi) -> 1.0 (kirmizi)

Color thermal_color(double t) {
    t = Vec3::clamp(t, 0.0, 1.0); // t'yi [0,1] araligina kirp
    double r = Vec3::clamp(3.0 * t - 1.5, 0.0, 1.0); // kırmızı bileşen
    double g = Vec3::clamp(1.0 - 3.0 * std::fabs(t - 0.5), 0.0,1.0); // yeşil bileşen
    double b = Vec3::clamp(1.5 - 3.0 * t, 0.0, 1.0); // mavi bileşen
    return Color(r, g, b);
    
    

}


int main(int argc, char* argv[]) {
    Args args = parse_args(argc, argv);
    // --seed verilmemisse (varsayilan -1) srand() hic cagrilmaz; rand() C standardina
    // gore hep sabit varsayilan tohumla (1) baslar -> eski deterministik sahne davranisi
    // (v1-v4 benchmark karsilastirilabilirligi) degismeden korunur.
    if (args.seed >= 0) srand(static_cast<unsigned>(args.seed));
    RenderConfig cfg{args.width, args.height, args.samples, args.depth};
    // Materyal ve sahne kurulumu (mevcut koddan taşı)
    // FIX-01 (D-01/D-02): sabit C-dizileri yerine std::vector; Scene non-owning
    // Hittable* tuttuğu için reallocation'ı önlemek üzere ilk &element alınmadan
    // ÖNCE reserve() ile kapasite ayrılıyor (belgelenmiş constexpr cap).
    std::vector<Sphere>     spheres;
    std::vector<Lambertian> lambertians;
    std::vector<Metal>      metals;
    spheres.reserve(COMPLEX_SCENE_MAX_SPHERES);
    lambertians.reserve(COMPLEX_SCENE_MAX_LAMBERTIANS);
    metals.reserve(COMPLEX_SCENE_MAX_METALS);
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
    else if (args.scene == "bench" || args.scene == "clustered") {
        if (args.scene == "bench") {
            build_scene_bench(scene, spheres, lambertians, metals, args.count);
        } else {
            build_scene_clustered(scene, spheres, lambertians, metals, args.count);
        }
    }
    else                              build_scene_complex(scene, mat_zemin,
        mat_orta, mat_sol, mat_sag, mat_ust, spheres, lambertians, metals);

    Bvh bvh(scene.objects_list()); // Sahnedeki nesnelerden BVH oluştur
    PPMWriter writer(cfg.width, cfg.height);
      
    std::cout << "Mode: " << args.mode << " | Scene: " << args.scene
            << " | " << cfg.width << "x" << cfg.height
            << " | samples=" << cfg.samples << "\n";
                
    auto t_start = std::chrono::high_resolution_clock::now();



    if (args.mode == "single") {
          render_tile(0, 0, cfg.width, cfg.height, bvh, cam, writer, cfg);

    } else if (args.mode == "naive") {
        std::thread* threads = new std::thread[cfg.height];
        for (int y = 0; y < cfg.height; ++y)
            threads[y] = std::thread([&, y] {
                render_tile(0, y, cfg.width, y + 1, bvh, cam, writer, cfg);
            });
        for (int y = 0; y < cfg.height; ++y)
            threads[y].join();
        delete[] threads;
        
    } else if (args.mode == "pool" || args.mode == "aligned") {
        // FIX-04 (D-12): v3 (ThreadPool/tile, unaligned ProgressBar) ve v4
        // (alignas(64) cache-fix / AlignedProgressBar) TEK bir paralel yola
        // birleştirildi. Eski false-sharing'e açık un-aligned "pool" dalı
        // KALDIRILDI — tek kalan yol AlignedProgressBar kullanır. v4'ün düzeltmesinin
        // ölçülmüş bir dezavantajı yok, dolayısıyla eski varyantı seçilebilir tutmak
        // sadece dispatch'i karmaşıklaştırırdı. v3-vs-v4 false-sharing karşılaştırması
        // docs/final_report*.md + engineering_journal.md'de tarihsel ölçüm verisi
        // olarak korunuyor (çalıştırılabilir kalmasına gerek yok).
        //
        // "--mode pool" string'i KORUNDU ama artık aligned impl'i çalıştırıyor:
        // Makefile'ın run/animate/timelapse hedefleri hepsi --mode pool geçiyor,
        // string'i yeniden anlamlandırmak bu hedefleri Makefile'a dokunmadan çalışır
        // tutuyor. "--mode aligned" da aynı yola alias. (Bu isimlendirme seçimi
        // kullanıcı tarafından önceden açıkça onaylandı — bkz. journal 2026-07-18.)
        const int tile_w = TILE_SIZE, tile_h = TILE_SIZE;
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
                    render_tile(x0, y0, x1, y1, bvh, cam, writer, cfg);
                    int idx = ThreadPool::this_thread_idx();
                    if (idx >= 0 && idx < args.threads)
                    progress.increment(idx);
                });
            }
        }
        pool.shutdown();
        progress.stop();

    } else if (args.mode == "animate" || args.mode == "timelapse") {
        // FIX-13 (D-14, Plan 04): kare cikti yolu artik output/ duzeniyle uyumlu —
        // eski frames/ yolu birakildi, boylece hicbir script/Makefile hedefi artik
        // var olmayan bir dizini okumuyor. Ikisi de "tekrarli kare render'i" ama
        // ciktilari ayri klasorlere gidiyor:
        //   animate   -> output/animation/frame_*.ppm  (make_video.sh animate stitch)
        //   timelapse -> output/timelapse/frame_*.ppm  (Makefile timelapse dongusu her
        //                thread sayisini output/timelapse/t<N>.mp4'e kodlar)
        const std::string frame_dir =
            (args.mode == "timelapse") ? "output/timelapse" : "output/animation";
        std::filesystem::create_directories(frame_dir);

        double radius = 3.0;
        double cam_height = 1.0;
        double aspect = (double)args.width / args.height;
        RenderConfig frame_cfg{args.width, args.height, args.samples, args.depth};
        const int tile = TILE_SIZE;   // FIX-16 (D-11): tek kaynaktan

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
                        render_tile(x0, y0, x1, y1, bvh, cam_frame, frame_writer, frame_cfg);
                    });
                }
            pool.shutdown();

            char fname[128];
            std::snprintf(fname, sizeof(fname), "%s/frame_%03d.ppm", frame_dir.c_str(), i);
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
        render_tile(0, 0, args.width, args.height, bvh, cam_frame, frame_writer, frame_cfg);

        char fname[64];
        std::snprintf(fname, sizeof(fname), "frames/frame_%03d.ppm", i);
        frame_writer.save(fname);
        std::cout << "Kare " << i + 1 << "/" << args.frames << " -> " << fname << "\n";
    }
    
    } 
    else if (args.mode == "heatmap") {
        std::vector<long> visits(cfg.width * cfg.height);
        long max_visits = 0;
        for (int j = 0; j < cfg.height; ++j) {
            for (int i = 0; i < cfg.width; ++i) {
                double u = (i + 0.5) / cfg.width;
                double v = (j + 0.5) / cfg.height;
                Ray ray = cam.get_ray(u, v);
                HitRecord rec;
                long visited = 0;
                bvh.hit_instrumented(ray, 0.001, 1e30, rec, visited);
                visits[j * cfg.width + i] = visited;
                max_visits = std::max(max_visits, visited);
            }
        }

        if (max_visits == 0) {
            for(int j = 0; j < cfg.height; ++j) 
                for (int i = 0; i < cfg.width; ++i) 
                    writer.set_pixel(i, j, Color(0, 0, 0.2)); // Mavi renk
        } else {
            for(int j = 0; j < cfg.height; ++j) {
                for (int i = 0; i < cfg.width; ++i) {
                    double t = visits[j * cfg.width + i] / (double)max_visits;
                    Color c = thermal_color(t);
                    writer.set_pixel(i, j, c);
                }            
            }
        }
    }

    
    else {
        std::cerr << "Bilinmeyen mod: " << args.mode << "\n";
        return 1;
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double ms  = std::chrono::duration<double, std::milli>(t_end -
    t_start).count();
    std::cout << "Tamamlandi: " << ms << " ms\n";

    fprintf(stderr, "BVH: nodes=%d depth=%d avg_leaf=%.1f build=%ld ms\n", 
        bvh.stat_nodes(), bvh.stat_depth(), bvh.stat_avg_leaf(), bvh.stat_build_ms());

        fprintf(stderr, "BVH_LEAF_HIST: ");
        std::vector<int> hist = bvh.leaf_size_histogram();
        for (std::size_t sz = 0; sz < hist.size(); ++sz) {
            if (hist[sz] > 0)
                fprintf(stderr, "%zu:%d ", sz, hist[sz]);
        }
        fprintf(stderr, "\n");
      
    if (args.mode != "animate" && args.mode != "timelapse") {
        writer.save(args.output);
        std::cout << "Kaydedildi: " << args.output << "\n";
    }
    return 0;
}   