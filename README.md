# Çok İş Parçacıklı Işın İzleme Motoru

**Enes F. Erkmen — 220309007**
Sistem Programlama Dönem Projesi (tam puanla teslim edildi) → şimdi serbest geliştirme aşamasında · C++17 · Ubuntu Linux

---

## Proje Hakkında

Sıfır harici bağımlılıkla (yalnızca C++ standart kütüphanesi), `std::thread` kullanarak geliştirilmiş bir **ray tracing (ışın izleme)** motoru. Amaç fotorealistik görüntü üretmek **değil**, sistem/performans mühendisliği kavramlarını ölçülebilir biçimde analiz etmek:

> "Önce naif çözüm → ölç → sorunu gör → düzelt → kanıtla."

**Ders dönemi (donduruldu, tarihi referans):** tek-thread baseline → thread-per-row (kasıtlı kötü tasarım) → ThreadPool/tile-based → `alignas(64)` cache fix. Ölçümler `results/`'de.

**Serbest geliştirme yol haritası (`FIX → BVH → DOD → SIMD → DIST → SCALE`):**

| Faz | İçerik | Durum |
|---|---|---|
| 1 | Stabilize & Unify Baseline — bellek/thread güvenliği, CLI sağlamlığı, build/script hijyeni | ✅ Tamamlandı |
| 2 | BVH Median-Split Baseline — O(log n) closest-hit, flat/index arena | 🔄 Devam ediyor |
| 3 | BVH SAH + Traversal Tooling | ⏳ |
| 4 | Data-Oriented Design (SoA + Arena) | ⏳ |
| 5 | SIMD Vectorization (AVX2) | ⏳ |
| 6-9 | Dağıtık sistem (TCP master-worker → hata toleransı → epoll/io_uring → render farm) | ⏳ |

---

## Faz 2 Öne Çıkan Sonuç — BVH gerçek render yolunda ~419× hızlanma

Aynı 100.000 nesneli sahne, aynı ayarlar, tek fark ışın-kesişim yapısı:

| | Brute-force (`Scene::hit()`, O(n)) | BVH (`Bvh::hit()`, O(log n)) |
|---|---:|---:|
| Süre | 9224 ms | **22 ms** |


---

## Mimari

```
ProjectRayTraycing/
├── src/
│   ├── vec3.h              — 3D vektör/nokta/renk (operator[] dahil)
│   ├── ray.h                — Parametrik ışın: r(t) = o + t·d
│   ├── aabb.h                — Axis-aligned bounding box + NaN-güvenli slab test (Faz 2)
│   ├── hittable.h           — Soyut arayüz: hit() + bounding_box()
│   ├── Sphere.h              — Küre geometrisi, ışın-küre kesişimi
│   ├── scene.h                — Brute-force sahne (D-02: sadece C++-seviyesi test/karşılaştırma referansı)
│   ├── bvh.h                   — Flat/index BVH: median-split build, near-first pruned traversal (Faz 2)
│   ├── scene_builders.h        — build_scene_bench/clustered (BVH ölçüm sahneleri, paylaşılan header)
│   ├── camera.h                 — Viewport → dünya uzayı, animasyon rotasyonu
│   ├── material.h                — Lambertian (mat) + Metal (yansımalı)
│   ├── ppm.h                      — PPM P3 çıktı + PPMWriter tamponu
│   ├── renderer.h/cpp              — render_tile() (const Hittable&, Scene/Bvh ikisiyle de çalışır)
│   ├── threadpool.h/cpp             — Task queue, mutex, condition_variable
│   ├── progress.h                    — Terminal progress bar (atomic, cache-aligned)
│   └── main.cpp                       — CLI, sahne kurulumu, render dispatch
├── tests/                — test_sphere.cpp, test_aabb.cpp, test_bvh.cpp
├── scripts/               — make_video.sh, plot_ahmdal.py
└── Makefile
```

**İki mimari menteşe noktası** (yeni fazlar bunları değiştirmeden eklenir): `Hittable::hit()` (geometri katmanı — `Bvh` burada `Scene`'in yerini alıyor) ve `render_tile()` (yürütme katmanı — dağıtık sistem tile görevini buradan alacak).

---

## Derleme ve Çalıştırma

```bash
make            # -O2 ile derle
make debug      # -O0 -g -fsanitize=thread
make fast       # -O3

make run T=8              # 8 thread ile render (1280×720, --scene complex)
make animate T=8           # animasyon kareleri (output/animation/)
make timelapse              # 1/2/4/6/8/12 thread karşılaştırmalı timelapse
make plot                    # Amdahl grafiği (scripts/plot_ahmdal.py)
make clean
```

**Doğrudan CLI (tüm bayraklar):**
```bash
./raytracer --threads 12 --mode pool --scene bench --count 10000 \
            --width 1280 --height 720 --samples 16 --depth 5 \
            --output output.ppm [--seed N] [--animate] [--timelapse]
```

| Bayrak | Açıklama |
|---|---|
| `--scene` | `simple` \| `medium` \| `complex` \| `bench` \| `clustered` (son ikisi `--count` alır, BVH ölçümü için) |
| `--count N` | `bench`/`clustered` sahnelerinde nesne sayısı (varsayılan 1000) |
| `--mode` | `single` \| `naive` \| `pool` (ThreadPool, önerilen) \| `animate` \| `animate-single` |
| `--seed N` | Verilmezse deterministik (eski davranış korunur); verilirse farklı-ama-tekrarlanabilir sahne |

Her render sonunda BVH istatistikleri stderr'e yazılır: `BVH: nodes=... depth=... avg_leaf=... build=... ms` ve `BVH_LEAF_HIST: ...` (yaprak-boyutu dağılımı).

**Gereksinimler:** `g++` (C++17), `ffmpeg` (video için), `python3` + `matplotlib` (grafik için), `convert`/ImageMagick (PPM→PNG).

---

## Testler

```bash
g++ -std=c++17 -Isrc -Wall -Wextra tests/test_sphere.cpp -o /tmp/t1 && /tmp/t1
g++ -std=c++17 -Isrc -Wall -Wextra tests/test_aabb.cpp   -o /tmp/t2 && /tmp/t2
g++ -std=c++17 -Isrc -Wall -Wextra tests/test_bvh.cpp    -o /tmp/t3 && /tmp/t3
```

`test_bvh.cpp`, `Scene::hit()` (brute-force referans) ile `Bvh::hit()`'i ray/HitRecord seviyesinde karşılaştırır (14 sabit ışın + 64×64 deterministik kamera ışın-ızgarası, RNG yok).

---

## Işın-Küre Kesişimi

Küre denklemi `|P-C|² = R²`, ışın `P = o + t·d` yerine koyunca:

```
t²(d·d) + 2t(d·oc) + (oc·oc - R²) = 0    (oc = o - C)
```

`b = 2h` sadeleştirmesiyle:

```cpp
double h            = ray.direction.dot(oc);
double discriminant = h*h - a*c;
double t            = (-h - sqrt(discriminant)) / a;
```

`Δ < 0` → ıskalama · `Δ = 0` → teğet · `Δ > 0` → 2 nokta, yakın olan seçilir

---

## BVH — Median-Split (Faz 2)

- **Flat/index arena** (`std::vector<BVHNode>`, çocuklar pointer değil `int` index) — `nodes.resize(2n-1)` inşa öncesi tam kapasiteyle ayrılır, inşa boyunca `push_back` hiç çağrılmaz (dangling-referans riskini yapısal olarak ortadan kaldırır).
- **Build:** `std::nth_element` ile centroid'lerin en geniş yayıldığı eksende ortancaya göre böl (O(n) per düğüm, tam sıralama değil).
- **Traversal:** iteratif, elle yönetilen yığın; ışının yönüne göre önce geometrik olarak yakın çocuk gezilir, mevcut en-yakın `t` ile budama yapılır.
- **Doğruluk kanıtı:** `tests/test_bvh.cpp`, C++ seviyesinde, RNG'siz.

---

## Teknik Detaylar

- **Dil:** C++17 · **Derleyici:** g++ 13.3 · **Platform:** Ubuntu Linux, AMD Ryzen 5 5600X (12 mantıksal çekirdek)
- **Thread:** `std::thread` + elle yazılmış `ThreadPool` (POSIX uyumlu)
- **Çıktı formatı:** PPM Plain Text (P3) — sıfır bağımlılık
- **Varsayılan render:** 1280×720 · 4 örnek/piksel (CLI ile ayarlanabilir) · 5 yansıma derinliği
