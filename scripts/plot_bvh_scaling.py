import matplotlib.pyplot as plt
import csv
import sys
import os

# --- VERİ: bench_bvh.cpp'nin ürettiği CSV'den oku ---
# CSV yolu argüman olarak verilebilir (bench her çalıştırmada farklı bir yere yazılabilir)
csv_path = sys.argv[1] if len(sys.argv) > 1 else "/tmp/bvh.csv"

n_vals, brute_vals, bvh_vals = [], [], []
with open(csv_path) as f:
    reader = csv.DictReader(f)
    for row in reader:
        n_vals.append(int(row["n"]))
        brute_vals.append(float(row["time_brute_ms"]))
        bvh_vals.append(float(row["time_bvh_ms"]))

# --- FİGÜR ---
fig, ax = plt.subplots(figsize=(9, 6))

# --- ÇİZGİLER (iki eğri: brute force vs BVH) ---
ax.plot(n_vals, brute_vals, marker='o', linewidth=2, label="Brute force O(n)")
ax.plot(n_vals, bvh_vals, marker='o', linewidth=2, label="BVH O(log n)")

# --- LOG-LOG ÖLÇEK ---
# O(n) log-log'da düz bir çizgi, O(log n) düzleşen bir eğri olur —
# doğrusal eksende O(log n) neredeyse görünmez kalırdı.
ax.set_xscale('log')
ax.set_yscale('log')

# --- EKSEN ETİKETLERİ ---
ax.set_xlabel("Nesne sayısı (n)", fontsize=12)
ax.set_ylabel("Kapanış-vuruş süresi (ms)", fontsize=12)
ax.set_title("BVH Ölçeklenme — Brute Force vs BVH (log-log)", fontsize=13)

ax.grid(True, alpha=0.3)
ax.legend(loc='upper left', fontsize=10)

# --- KAYDET ---
# Script nerede çalışırsa çalışsın, proje köküne göre kaydet (plot_ahmdal.py ile aynı idiom)
script_dir = os.path.dirname(os.path.abspath(__file__))
out_path = os.path.join(script_dir, "..", "docs", "images", "bvh_scaling.png")
plt.tight_layout()
plt.savefig(out_path, dpi=150)
print(f"Kaydedildi: {out_path}")
