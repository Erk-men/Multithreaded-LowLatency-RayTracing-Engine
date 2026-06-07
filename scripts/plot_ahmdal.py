import matplotlib.pyplot as plt
import os

# --- VERİ ---
# X ekseni: 3 sahne (satır sonu \n görsel için)
scenes = ["simple\n(2 sph, 16spp)", "medium\n(5 sph, 16spp)", "heavy\n(200 sph, 64spp)"]

# Her versiyonun v1'e göre hızlanması (comparison.csv, O2)
speedups = {
    "v2 (thread-per-row)": [5.70, 11.26, 14.93],
    "v3 (thread pool)":    [4.70, 11.76, 14.60],
    "v4 (cache aligned)":  [4.69, 11.74, 14.87],
}

# --- FİGÜR ---
# figsize=(genişlik, yükseklik) inç cinsinden
fig, ax = plt.subplots(figsize=(9, 6))

# --- ÇİZGİLER ---
# x = [0, 1, 2] — sahne indeksleri
x = range(len(scenes))

# Her versiyon için otomatik farklı renk atanır
for label, values in speedups.items():
    ax.plot(x, values, marker='o', linewidth=2, label=label)

# Teorik tavan: 12 çekirdek → maksimum 12× hızlanma (Amdahl üst sınırı)
ax.axhline(y=12, color='red', linestyle='--', linewidth=1.5,
           label='Theoretical max (12 cores)')

# --- EKSEN ETİKETLERİ ---
ax.set_xticks(x)                     # x=0,1,2 konumlarına etiket yaz
ax.set_xticklabels(scenes)           # indeks yerine sahne isimlerini göster
ax.set_xlabel("Scene complexity", fontsize=12)
ax.set_ylabel("Speedup vs v1 (×)", fontsize=12)
ax.set_title("Amdahl's Law — Measured Speedup vs v1 (O2)", fontsize=13)

# Y ekseni: 0'dan başlat, biraz üst boşluk bırak
ax.set_ylim(0, 14)

# Izgara: okumayı kolaylaştırır, alpha=saydamlık
ax.grid(True, alpha=0.3)

# Legend (açıklama kutusu) — sol üst köşeye yerleştir
ax.legend(loc='upper left', fontsize=10)

# --- KAYDET ---
# Script nerede çalışırsa çalışsın, proje köküne göre kaydet
script_dir = os.path.dirname(os.path.abspath(__file__))
out_path = os.path.join(script_dir, "..", "docs", "images", "amdahl_speedup.png")
plt.tight_layout()                   # kenar boşluklarını otomatik ayarla
plt.savefig(out_path, dpi=150)       # dpi=150: rapor için yeterli çözünürlük
print(f"Kaydedildi: {out_path}")
