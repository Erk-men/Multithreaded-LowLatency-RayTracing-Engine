# Sunum Notları — Çok İş Parçacıklı Işın İzleme Motoru
**Enes F. Erkmen — 220309007**

> Bu dosya her modül tamamlandıkça güncellenir.
> Her bölüm "Sunumda nasıl anlatırsın?" sorusuna cevap verir.

---

## Büyük Resim — Projenin Amacı

Bu proje bir ray tracer yazmak **değil**, sistem programlama kavramlarını
**ölçülebilir biçimde analiz etmektir.** Anlatı şöyle:

> "Önce naif bir çözüm yazdım → ölçtüm → sorunu gördüm → düzelttim → işte kanıtı."

Dört versiyon:
- **v1** → Tek thread (baseline, referans nokta)
- **v2** → Thread-per-row (kasıtlı kötü tasarım — neden yanlış?)
- **v3** → Thread Pool, tile-based (doğru tasarım)
- **v4** → alignas(64) cache optimizasyonu (düşük seviye iyileştirme)

---

## Modül 1 — Işın İzleme Nedir?

### Temel Mantık
Gerçek hayatta ışık: **Güneş → Nesne → Kamera**
Bizim yaptığımız (backward ray tracing): **Kamera → Nesne → Işık kaynağı**

Neden tersine çevirdik?
Güneşten çıkan milyarlarca ışının yalnızca küçük bir kısmı kameraya ulaşır.
Hepsini hesaplamak imkânsız. Kameradan fırlatırsak sadece
**görüntüye katkıda bulunan ışınları** hesaplarız.

### Neden Paralel?
1280×720 = **921.600 piksel** = 921.600 bağımsız ışın hesabı.
Her piksel birbirinden bağımsız → aynı anda hesaplanabilir → **threading'in temeli.**

### Sunumda nasıl anlatırsın?
"Her piksel için kameradan bir ışın fırlatıyorum. Bu ışın sahnedeki bir nesneye
çarparsa o pikselin rengini o noktadan hesaplıyorum. 921.600 piksel birbirinden
bağımsız olduğu için hepsini aynı anda paralel hesaplayabilirim — bu projenin
threading kısmının temelini oluşturuyor."

---

## Modül 2 — Vec3: 3D Matematik Temeli

### Neden Vec3?
Ray tracing'de üç farklı kavram var: **nokta** (konum), **vektör** (yön), **renk** (RGB).
Hepsi matematiksel olarak `(x, y, z)` üçlüsü → tek sınıf yeterli.

```cpp
using Point3 = Vec3;  // 3D konum
using Color  = Vec3;  // RGB renk [0,1]
```

### Neden double, float değil?
Işın hesapları zincirlenir. float → 7 basamak hassasiyet.
double → 15 basamak. Küçük hatalar katlanınca yanlış kesişimler oluşur.

### Önemli Operatörler
```cpp
a + b          // vektör toplama: yön/renk birleştirme
a - b          // vektör çıkarma
-a             // yön tersine çevirme
a * 2.0        // skaler ile ölçekleme
a * b          // bileşen bazlı: renk × renk hesabı
length()       // |v| = sqrt(x²+y²+z²)  — gerçek uzunluk
length_squared()  // x²+y²+z²  — sqrt() olmadan, karşılaştırma için hızlı
```

### Matematiksel Fonksiyonlar

```cpp
a.dot(b)          // a·b = ax*bx + ay*by + az*bz  → skaler, aydınlatma hesabı
a.cross(b)        // a×b → ikisine dik yeni vektör, kamera eksenleri için
a.normalize()     // yönü korur, uzunluğu 1 yapar → ışın yönleri ve normaller
a.reflect(n)      // gelen ışın + yüzey normali → yansıyan ışın yönü
Vec3::clamp(x,lo,hi)       // değeri [lo,hi] aralığında tutar → renk taşmasını önler
Vec3::gamma_correct(v)     // sqrt(v) → lineer→gamma uzayı, monitör doğru gösterir
```

### Dot Product — Aydınlatma Kalbi
Sonuç > 0: aynı yönde (ışık yüzeye vuruyor)
Sonuç = 0: dik açı (yüzey kenarda, karanlık)
Sonuç < 0: zıt yön (ışık arkadan, karanlık)

### Cross Product — Kamera Koordinat Sistemi
```
right      = forward × world_up   // kameranın sağ yönü
camera_up  = right × forward      // kameranın gerçek yukarısı
```
Bu üç vektörle her pikselin ışın yönü hesaplanır.

### Sunumda nasıl anlatırsın?
"Nokta, vektör ve renk için ayrı sınıf yazmak yerine tek Vec3 sınıfı kullandım.
double tercih ettim çünkü ışın hesapları zincirleniyor — float'ın 7 basamak
hassasiyeti yanlış gölge hesaplarına yol açabilirdi. Dot product aydınlatmanın,
cross product kamera koordinat sisteminin temelidir."

---
<!-- Yeni modüller buraya eklenecek -->
