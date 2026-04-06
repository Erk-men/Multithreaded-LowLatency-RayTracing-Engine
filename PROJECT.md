# Çok İş Parçacıklı Işın İzleme Motoru
**Enes F. Erkmen — 220309007 — Sistem Programlama Dönem Projesi**

---

## Genel Bakış

C++ ile Ubuntu üzerinde, POSIX-uyumlu `std::thread` kullanarak bir ray tracing (ışın izleme) motoru geliştiriyoruz. Projenin amacı hem fotorealistik görüntü üretmek hem de multi-threading'in sistem performansına etkisini ölçmek ve analiz etmektir. Sunum için kamera animasyonu (video) ve performans grafikleri de üretilecek.

---

## Proje Yapısı (Hedef)

```
ProjectRayTraycing/
├── PROJECT.md              ← Bu dosya (proje rehberi)
├── src/
│   ├── main.cpp            ← Giriş noktası, thread pool başlatma, render tetikleme
│   ├── vec3.h              ← 3D vektör matematik kütüphanesi
│   ├── ray.h               ← Işın (Ray) yapısı
│   ├── hittable.h          ← Soyut nesne arayüzü
│   ├── sphere.h            ← Küre geometrisi ve ışın-küre kesişimi
│   ├── scene.h             ← Sahne (nesneler + ışık kaynakları)
│   ├── camera.h            ← Kamera, piksel→ışın dönüşümü, animasyon rotasyonu
│   ├── material.h          ← Diffuse + reflective materyaller, Phong aydınlatma
│   ├── renderer.h/cpp      ← Render fonksiyonu (anti-aliasing, recursive reflection)
│   ├── threadpool.h/cpp    ← Thread pool: task queue, mutex, condition_variable
│   ├── progress.h          ← Terminal progress bar
│   └── ppm.h               ← PPM formatında çıktı yazma
├── output/                 ← Render edilen .ppm / .png görüntüler
├── animation/              ← Animasyon frame'leri ve output.mp4
├── results/                ← Benchmark CSV dosyaları
├── scripts/
│   ├── benchmark.sh        ← 1/2/4/8/16 thread + -O0/-O2/-O3 otomatik ölçüm
│   └── plot.py             ← Python matplotlib ile performans grafikleri
└── Makefile                ← Derleme ve otomasyon hedefleri
```

---

## Bileşenler ve Sorumlulukları

### 1. Vec3 (vec3.h)
Harici kütüphane yok. Elle yazılmış 3D vektör sınıfı.
- Toplama, çıkarma, skaler çarpma
- Dot product (iç çarpım), cross product
- Normalize, length
- Renk için de aynı tip kullanılır (RGB = Vec3)

### 2. Ray (ray.h)
`origin + t * direction` formülüyle ışın tanımı.
```
r(t) = o + t*d
```

### 3. Hittable / Sphere (hittable.h, sphere.h)
Her nesne `hit()` metodunu uygular. Küre için:
```
|o + t*d - c|² = R²  →  at² + bt + c = 0
```
Discriminant ≥ 0 ise kesişim var, t_min en yakın yüzey.

### 4. Camera (camera.h)
- Ekranı sanal bir düzleme (viewport) map'ler. Piksel (i,j) → dünya uzayında bir ışın üretir.
- **Animasyon:** Kamera sahne etrafında bir yay üzerinde döner. Her frame için açı parametresi alır.

### 5. Material (material.h)
- **Diffuse (mat):** Lambert yansıması, ışık kaynağına bağlı renk
- **Reflective:** Gelen ışın normale göre yansıtılır: `r = d - 2(d·n)n`
- Phong aydınlatma: ambient + diffuse + specular
- **Recursive reflection:** Yansıyan ışın tekrar sahnede izlenir (max_depth parametresi, varsayılan 5)

### 6. Renderer (renderer.h/cpp)
Her piksel için:
1. Kameradan ışın üret
2. Sahnedeki her nesneyle kesişim kontrol et
3. En yakın kesişim noktasında aydınlatma hesapla
4. **Anti-aliasing:** Piksel başına N ışın (varsayılan 16), renklerin ortalaması alınır
5. Rengi buffer'a yaz

### 7. ThreadPool (threadpool.h/cpp)
**Thread Pool Mimarisi:**
- Ekran 64×64 piksel tile'lara bölünür
- `std::queue<Task>` ile iş kuyruğu tutulur
- N adet thread sürekli çalışır, kuyruktan tile alır
- `std::mutex` + `std::condition_variable` ile senkronizasyon
- Thread sayısı komut satırından alınır (1, 2, 4, 8, 16)

### 8. Progress Bar (progress.h)
Render sırasında terminalde canlı ilerleme göstergesi:
```
Rendering... [████████░░░░░░░░] 52% — Threads: 8 — Elapsed: 2.3s
```

### 9. PPM Output (ppm.h)
En basit görüntü formatı, header + RGB değerleri:
```
P3
width height
255
r g b  r g b  ...
```

### 10. Animasyon (Makefile + ffmpeg)
- 72 frame render edilir (5° adımlarla 360°)
- Her frame `animation/frame_XXXX.ppm` olarak kaydedilir
- `ffmpeg -r 24 -i frame_%04d.ppm output.mp4` ile video oluşturulur

### 11. Performans Grafikleri (scripts/plot.py)
Python matplotlib ile:
- Thread sayısı vs. render süresi (gerçek vs. Amdahl teorik)
- Optimizasyon seviyesi (-O0/-O2/-O3) vs. render süresi
- Speedup grafiği

---

## Uygulama Aşamaları

| Aşama | İçerik | Durum |
|-------|--------|-------|
| 1 | Vec3, Ray yapıları | Tamamlandı |
| 2 | Küre kesişimi, PPM çıktı | Tamamlandı |
| 3 | Kamera, tek-thread render | Bekliyor |
| 4 | Phong aydınlatma + materyaller | Bekliyor |
| 5 | Anti-aliasing + recursive reflection | Bekliyor |
| 6 | Thread Pool implementasyonu | Bekliyor |
| 7 | Progress bar | Bekliyor |
| 8 | Animasyon (kamera rotasyonu + ffmpeg) | Bekliyor |
| 9 | Benchmark: thread sayısı + -O0/-O2/-O3 | Bekliyor |
| 10 | False sharing demosu + ölçümü | Bekliyor |
| 11 | Python performans grafikleri | Bekliyor |

---

## Ölçülecek Metrikler

- Her thread konfigürasyonu için render süresi (`std::chrono`)
- CPU kullanımı (`perf stat`)
- Thread güvenliği (`valgrind --tool=helgrind`, `-fsanitize=thread`)
- Bellek profili (`valgrind --tool=massif`)
- Fonksiyon düzeyinde hotspot (`gprof`)
- Amdahl Yasası ile teorik vs. gerçek hızlanma karşılaştırması
- False sharing: hizasız vs. cache-aligned veri yapısı farkı
- Derleyici optimizasyonu: -O0 / -O2 / -O3 render süresi farkı

---

## Derleme Hedefleri (Makefile)

```makefile
make              # -O2 ile derle
make debug        # -O0, -g ile derle
make fast         # -O3 ile derle
make run T=4      # 4 thread ile render
make bench        # 1/2/4/8/16 thread + tüm optimizasyon seviyeleri
make animate T=8  # 72 frame render et, ffmpeg ile video oluştur
make plot         # Python ile performans grafiklerini çiz
make clean        # Derleme çıktılarını temizle
```

---

## Çalışma Yöntemi (Claude İçin Hatırlatma)

Her aşama tamamlandığında şu adımlar takip edilmeli:
1. Aşama tablosunda ilgili satırı `Tamamlandı` olarak güncelle
2. `sunum.md` dosyasına o aşamanın sunum notlarını ekle:
   - Ne yaptık? (kısa özet)
   - Temel kavramlar ve matematiği (öğretici, sade dil)
   - Kod örnekleri (kritik kısımlar)
   - Test çıktısı
   - "Sunumda nasıl anlatırsın?" bölümü (hazır cümle)
3. Bir sonraki aşamaya geç

`sunum.md` dosyası Enes'in sunuma hazırlanmak için okuduğu ana referans — her zaman güncel tutulmalı.

---

## Önemli Notlar

- Harici matematik/görüntü kütüphanesi kullanılmıyor (sıfır bağımlılık)
- Çıktı formatı: PPM (binary değil, plain text P3)
- Animasyon video formatı: MP4 (ffmpeg ile)
- Performans grafikleri: Python matplotlib
- İşletim sistemi: Ubuntu Linux
- Derleyici: g++ (C++17)
- Thread kütüphanesi: `std::thread` (POSIX uyumlu)
- Görüntü çözünürlüğü: 800×600 (varsayılan), animasyon için 1280×720
