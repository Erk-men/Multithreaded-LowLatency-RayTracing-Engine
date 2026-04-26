# Çok İş Parçacıklı Işın İzleme Motoru

**Enes F. Erkmen — 220309007**  
Sistem Programlama Dönem Projesi · C++17 · Ubuntu Linux

---

## Proje Hakkında

Sıfır harici bağımlılıkla, `std::thread` kullanarak geliştirilmiş bir **ray tracing (ışın izleme)** motorudur. Projenin amacı fotorealistik görüntü üretmek **değil**, sistem programlama kavramlarını ölçülebilir biçimde analiz etmektir:

> "Önce naif çözüm → ölç → sorunu gör → düzelt → kanıtla."

Dört versiyon:

| Versiyon | Açıklama |
|----------|----------|
| **v1** | Tek thread — referans nokta (baseline) |
| **v2** | Thread-per-row — kasıtlı kötü tasarım, neden yanlış? |
| **v3** | Thread Pool, tile-based — doğru tasarım |
| **v4** | `alignas(64)` cache optimizasyonu — düşük seviye iyileştirme |

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
├── docs/               — Tasarım dokümanları, planlar, LaTeX rapor
├── output/             — Render edilen görüntüler (.ppm / .png)
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
double h            = ray.direction.dot(oc);
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

make run T=4    # 4 thread ile render (1280×720)
make bench      # 1/2/4/8/16 thread + tüm -O seviyeleri
make animate T=8 # animasyon frame'leri
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

## Geliştirme Aşamaları

| # | İçerik | Durum |
|---|--------|-------|
| 1 | Vec3 — vektör, nokta, renk matematiği | Tamamlandı |
| 2 | Ray — parametrik ışın | Tamamlandı |
| 3 | Hittable — soyut nesne arayüzü | Tamamlandı |
| 4 | Sphere — ışın-küre kesişim geometrisi | Devam ediyor |
| 4 | PPM, Camera — görüntü çıktısı ve viewport | Bekliyor |
| 5 | Renderer v1 — tek thread baseline | Bekliyor |
| 6 | Renderer v2 — thread-per-row (naif) | Bekliyor |
| 7 | Renderer v3 — Thread Pool, tile-based | Bekliyor |
| 8 | Renderer v4 — alignas(64) cache fix | Bekliyor |
| 9 | Benchmark + analiz + grafikler | Bekliyor |
| 10 | LaTeX rapor + PlantUML diyagramlar | Bekliyor |

---

## Teknik Detaylar

- **Dil:** C++17 · **Derleyici:** g++ · **Platform:** Ubuntu Linux
- **Thread:** `std::thread` (POSIX uyumlu)
- **Çıktı formatı:** PPM Plain Text (P3) — sıfır bağımlılık
- **Çözünürlük:** 1280×720 · **Örnekleme:** 16 ışın/piksel · **Yansıma derinliği:** 5
