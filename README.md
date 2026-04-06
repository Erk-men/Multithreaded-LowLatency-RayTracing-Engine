# Çok İş Parçacıklı Işın İzleme Motoru

**Enes F. Erkmen — 220309007**  
Sistem Programlama Dönem Projesi · C++17 · Ubuntu Linux

---

## Proje Hakkında

Sıfır harici bağımlılıkla, POSIX-uyumlu `std::thread` kullanarak geliştirilmiş bir **ray tracing (ışın izleme)** motorudur. Projenin iki temel hedefi vardır:

1. Fotorealistik görüntü ve animasyon üretmek
2. Multi-threading'in sistem performansına etkisini ölçmek ve Amdahl Yasası ile karşılaştırmak

---

## Örnek Çıktı

> *(Aşama 2 — Normal Visualization, 800×450)*

![render_phase2](output/render_phase2.png)

---

## Mimari

```
ProjectRayTraycing/
├── src/
│   ├── vec3.h          — 3D vektör / nokta / renk (tek tip)
│   ├── ray.h           — Parametrik ışın: r(t) = o + t·d
│   ├── hittable.h      — Soyut nesne arayüzü (hit() metodu)
│   ├── sphere.h        — Küre geometrisi, ışın-küre kesişimi
│   ├── camera.h        — Viewport → dünya uzayı, animasyon rotasyonu
│   ├── material.h      — Diffuse (Lambert) + Reflective (Phong)
│   ├── renderer.h/cpp  — Anti-aliasing, recursive reflection
│   ├── threadpool.h/cpp— Task queue, mutex, condition_variable
│   ├── progress.h      — Terminal progress bar
│   └── ppm.h           — PPM P3 formatında çıktı
├── scripts/
│   ├── benchmark.sh    — 1/2/4/8/16 thread × -O0/-O2/-O3 ölçümü
│   └── plot.py         — Matplotlib ile performans grafikleri
├── output/             — Render edilen görüntüler (.ppm / .png)
├── animation/          — 72 frame + output.mp4
├── results/            — Benchmark CSV dosyaları
└── Makefile
```

---

## Thread Pool Mimarisi

Ekran **64×64 piksel tile**'larına bölünür. Her tile bir `Task` olarak `std::queue`'ya eklenir. N adet worker thread sürekli çalışarak kuyruktan tile alır ve render eder.

```
┌─────────────┐     push      ┌──────────────────┐     pop      ┌──────────────┐
│  Main Thread │ ──────────►  │  Task Queue       │ ──────────►  │  Worker × N  │
│  (tile üret) │              │  (mutex korumalı) │              │  (tile render)│
└─────────────┘              └──────────────────┘              └──────────────┘
                                    ▲ condition_variable ile uyandır/uyut
```

**Senkronizasyon:** `std::mutex` + `std::condition_variable`  
**Thread sayısı:** Komut satırından alınır → `make run T=8`

---

## Işın-Küre Kesişimi

Küre denklemi `|P-C|² = R²`, ışın `P = o + t·d` yerine koyunca:

```
t²(d·d) + 2t(d·oc) + (oc·oc - R²) = 0    (oc = o - C)
```

`b = 2h` substitüsyonu ile sadeleştirilmiş formül:

```cpp
double h            = Vec3::dot(ray.direction, oc);
double discriminant = h*h - a*c;
double t            = (-h - sqrt(discriminant)) / a;
```

`Δ < 0` → ıskalama · `Δ = 0` → teğet · `Δ > 0` → 2 nokta, yakın olan seçilir

---

## Derleme ve Çalıştırma

```bash
make            # -O2 ile derle
make debug      # -O0 -g (hata ayıklama)
make fast       # -O3 (maksimum optimizasyon)

make run T=4    # 4 thread ile render (800×600)
make bench      # 1/2/4/8/16 thread + tüm -O seviyeleri
make animate T=8 # 72 frame → output.mp4
make plot       # Python performans grafikleri
make clean
```

**Gereksinimler:** `g++` (C++17), `ffmpeg`, `python3` + `matplotlib`

---

## Ölçülen Metrikler

| Araç | Ölçüm |
|------|-------|
| `std::chrono` | Thread başına render süresi |
| `perf stat` | CPU cycle ve cache miss |
| `valgrind --tool=helgrind` | Thread güvenliği (data race) |
| `valgrind --tool=massif` | Bellek profili |
| `gprof` | Fonksiyon düzeyinde hotspot |
| Amdahl Yasası | Teorik vs. gerçek hızlanma |
| Cache alignment | False sharing etkisi |

---

## Uygulama Aşamaları

| # | İçerik | Durum |
|---|--------|-------|
| 1 | Vec3, Ray yapıları | Tamamlandı |
| 2 | Küre kesişimi, PPM çıktı | Tamamlandı |
| 3 | Kamera, tek-thread render | Devam ediyor |
| 4 | Phong aydınlatma + materyaller | Bekliyor |
| 5 | Anti-aliasing + recursive reflection | Bekliyor |
| 6 | Thread Pool | Bekliyor |
| 7 | Progress bar | Bekliyor |
| 8 | Animasyon (kamera rotasyonu + ffmpeg) | Bekliyor |
| 9 | Benchmark: thread sayısı + optimizasyon seviyeleri | Bekliyor |
| 10 | False sharing demosu + ölçümü | Bekliyor |
| 11 | Python performans grafikleri | Bekliyor |

---

## Teknik Detaylar

- **Dil:** C++17 · **Derleyici:** g++ · **Platform:** Ubuntu Linux
- **Thread:** `std::thread` (POSIX uyumlu, `<pthread.h>` olmadan)
- **Çıktı formatı:** PPM Plain Text (P3) — sıfır bağımlılık
- **Animasyon:** ffmpeg ile MP4 (72 frame, 24 fps, 5° adım)
- **Çözünürlük:** 800×600 (render) · 1280×720 (animasyon)
- **Anti-aliasing:** Piksel başına 16 ışın (ortalama)
- **Reflection derinliği:** 5 (recursive)
