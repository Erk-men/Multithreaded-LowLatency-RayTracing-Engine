# Sunum Notları — Çok İş Parçacıklı Işın İzleme Motoru
**Enes F. Erkmen — 220309007**

> Bu dosya her aşama tamamlandıktan sonra güncellenir.
> Sunumda teknik detaylara hakim görünmek için buradan çalış.

---

## Aşama 1: Vec3 ve Ray — Matematiksel Temel

### Ne Yaptık?
`vec3.h` ve `ray.h` dosyalarını sıfırdan yazdık. Harici hiçbir kütüphane kullanmadık.

### Neden Vec3 Tek Bir Tip?

Ray tracing'de üç farklı kavram var: **nokta** (konum), **vektör** (yön), **renk** (RGB). Hepsi matematiksel olarak üç sayıdan oluşuyor. Tek bir `Vec3` tipi ikisi için de yeterli — fark sadece kavramsal.

```cpp
Point3 camera(0, 0,  0);    // kamera konumu
Vec3   dir   (0, 0, -1);    // ışın yönü
Color  red   (1, 0,  0);    // kırmızı renk
// Hepsi Vec3 — aynı bellek düzeni, farklı anlam
```

### Dot Product — Neden Kritik?

```
dot(a, b) = ax·bx + ay·by + az·bz = |a||b|·cos(θ)
```

- θ = 0° → cos = 1.0 → ışık yüzeye dik geliyor → **maksimum parlaklık**
- θ = 90° → cos = 0.0 → ışık yüzeye teğet → **karanlık**
- θ > 90° → negatif → **gölge**

Aydınlatma hesabında `dot(normal, ışık_yönü)` ifadesi yüzeyin ne kadar parlak görüneceğini belirliyor.

### Normalize — Neden Şart?

Dot product'ın `cos(θ)` vermesi için her iki vektörün uzunluğunun 1.0 olması gerekiyor. Bu yüzden tüm yön vektörleri (ışın yönü, yüzey normali) `normalize()` ile birim uzunluğa indiriliyor.

```cpp
static Vec3 normalize(const Vec3& v) {
    return v / v.length();  // her bileşeni uzunluğa böl
}
```

### Reflect — Ayna Yüzeyler İçin

```
reflect(d, n) = d - 2·dot(d, n)·n
```

Geometrik yorum: gelen ışının normal eksenindeki bileşenini ters çevir. Bu formülle reflective (ayna gibi) materyaller gerçekçi yansıma üretiyor.

### Ray: r(t) = origin + t × direction

Işın bir parametrik doğru. `t` arttıkça ışın ilerliyor:

```cpp
Point3 p = ray.at(5.0);  // ışın üzerinde 5 birim ileri
```

- `t = 0` → kameranın kendisi
- `t > 0` → kameranın önü (anlamlı kesişimler)
- `t < 0` → kameranın arkası (görmezden gelinir)

### Test Çıktısı

```
dot(right, up)    = 0   ← dik açı, doğru
dot(right, right) = 1   ← aynı yön, doğru
cross(right, up)  = (0, 0, 1)   ← sağ el kuralı
reflect: gelen (0.7, -0.7, 0) → yansıyan (0.7, 0.7, 0)
r(5.0) = (0, 0, -5)   ← 5 birim ileri, doğru
✓ Tüm testler geçti
```

### Sunumda Nasıl Anlatırsın?

> *"Harici hiçbir kütüphane kullanmadım. Vec3 sınıfını sıfırdan yazdım çünkü ray tracing'de nokta, yön ve renk matematiksel olarak aynı yapı — üç float. dot product ile ışık açısını, cross product ile kamera koordinat sistemini, normalize ile birim vektörleri hesaplıyorum. Her operasyon için test yazdım ve doğruladım."*

---

---

## Aşama 2: Küre Kesişimi ve İlk PPM Görüntü

### Ne Yaptık?
`ppm.h`, `hittable.h`, `sphere.h` dosyalarını yazdık. İlk görüntüyü render ettik: normal visualization ile iki küre.

### Işın-Küre Kesişimi Matematiği

Küre üzerindeki her nokta şu denklemi sağlar:

```
|P - C|² = R²
```

Işın `P = o + t*d` olduğundan yerine koyarsak ve `oc = o - C` dersek:

```
t²(d·d) + 2t(d·oc) + (oc·oc - R²) = 0
```

Bu bir ikinci dereceden denklem. Discriminant'a göre:
- `Δ < 0` → kesişim yok (ışın küreyi ıssıyor)
- `Δ = 0` → teğet
- `Δ > 0` → 2 nokta, kameraya yakın olan (`t_min`) alınır

**Optimizasyon:** `b = 2h` substitüsyonu yapılınca formül sadeleşiyor:
```cpp
double h = dot(d, oc);          // b/2
double discriminant = h*h - a*c // b²-4ac yerine h²-ac
double t = (-h - sqrt_d) / a;   // 2 ve 4 çarpanları iptal oldu
```

### HitRecord ve Hittable Soyutlaması

Her nesne `hit()` metodunu uygular. Renderer nesne tipini bilmez, sadece `Hittable*` listesiyle çalışır:

```cpp
for (const auto& obj : scene) {
    if (obj->hit(ray, t_min, closest, rec)) { ... }
}
```

Bu sayede ileride düzlem, üçgen eklemek için renderer'a dokunmak gerekmez.

### Normal Visualization — İlk Görüntü

Henüz ışık kaynağı yok. Yüzey normalini renk olarak gösteriyoruz:
```
normal.x → kırmızı (sola/sağa bakan yüzey)
normal.y → yeşil   (aşağı/yukarı bakan yüzey)
normal.z → mavi    (ileri/geri bakan yüzey)
```
Normal `[-1,1]` aralığında, `(n + 1) / 2` ile `[0,1]`'e map'leniyor.

### Shadow Acne Problemi — `t_min = 0.001`

Kesişim noktasında aynı yüzeyden yeni ışın atarken `t=0` noktasının kendine çarpmasını önlemek için `t_min = 0.001` kullanıyoruz. Floating point hatası yüzünden ışın tam olarak `t=0`'da değil `t=0.000001`'de başlayabilir — bu küçük offset bunu çözer.

### Arka Plan: Linear Interpolation (Lerp)

```cpp
double t = 0.5 * (ray.direction.y + 1.0);  // y: [-1,1] → [0,1]
Color bg  = white * (1.0 - t) + blue * t;  // lerp formülü
```

t=0 (aşağı) → beyaz, t=1 (yukarı) → mavi. Klasik gökyüzü gradyanı.

### Üretilen Görüntü

- 800×450 piksel, 16:9 aspect ratio
- Normal visualization ile renkli küre
- Mavi→beyaz gradient arka plan
- Yeşil zemin küresi (r=100, dev küre = düz zemin görünümü)

### Sunumda Nasıl Anlatırsın?

> *"İkinci aşamada ışın-küre kesişimini türettim. Küre denklemini ışın parametrik denklemine koyunca ikinci dereceden bir denklem çıkıyor. Discriminant negatifse ışın küreyi ıssıyor, pozitifse iki kesişim noktası var ve kameraya yakın olanı alıyorum. Hittable soyutlamasıyla renderer nesne tipine bağımlı değil — ileride yeni geometri eklemek için sadece yeni bir sınıf yazmak yeterli."*

<!-- Sonraki aşamalar buraya eklenecek -->
