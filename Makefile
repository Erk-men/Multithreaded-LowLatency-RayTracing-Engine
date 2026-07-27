# =============================================================================
# Çok İş Parçacıklı Işın İzleme Motoru — Makefile Kullanım Kılavuzu
# =============================================================================
# Değişkenler (üstteki tanımlar):
#   CXX/CXXFLAGS  — derleyici ve ortak bayraklar (her hedef bunun üstüne kendi
#                   optimizasyon/sanitizer bayrağını ekler)
#   SRC           — ana render motorunun kaynak dosyaları (main+renderer+threadpool)
#   TARGET        — üretilen ikili dosyaların ortak öneki ("raytracer")
#   T             — thread sayısı, `run`/`animate` hedeflerine `T=<n>` ile geçilir
#                   (örn. `make run T=4`), varsayılan 6
#   OPT           — `all` hedefinin optimizasyon seviyesi, `make all OPT=-O3` ile
#                   değiştirilebilir, varsayılan -O2
#
# Günlük geliştirme döngüsü (en sık kullanılan sıra):
#   make all              → normal (-O2) render motorunu derler
#   make run              → derler + 1280x720 tek kare render alır
#   make test             → tests/ altındaki tüm birim testleri koşar
#   make clean            → tüm ikili/çıktı artefaktlarını siler
#
# Faz 2 (BVH) doğrulama araçları:
#   make bench            → bench/bench_bvh.cpp'yi derler (brute-force vs BVH süre ölçümü)
#   make bench-run        → bench'i derler + çalıştırır + CSV'yi log-log grafiğe döker
#   make asan             → AddressSanitizer ile derler — bellek güvenliği kapısı,
#                           bkz. aşağıdaki "asan" hedefinin yorumu için kullanım komutu
#
# Diğer hedefler: `debug` (TSan ile derleme — veri yarışı avı), `fast` (-O3,
# sanitizer'sız), `profile` (gprof için -pg), `animate`/`timelapse` (çoklu kare
# render + video birleştirme, ffmpeg gerektirir), `plot` (Amdahl grafiği),
# `uml` (PlantUML diyagramları, plantuml.jar gerektirir).
#
# Her hedefin tam ne yaptığını görmek için ilgili hedefin üstündeki yorum
# bloğuna bak (test/bench/bench-run/asan) veya `make -n <hedef>` ile hangi
# komutların çalışacağını (çalıştırmadan) önizle.
# =============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc
SRC      := src/main.cpp src/renderer.cpp src/threadpool.cpp
TARGET   := raytracer
T        ?= 6
OPT      ?= -O2

.PHONY: all debug fast profile run animate timelapse plot uml clean test bench bench-run asan

all:
	$(CXX) $(CXXFLAGS) $(OPT) -pthread -o $(TARGET) $(SRC)

debug:
	$(CXX) $(CXXFLAGS) -O0 -g -fsanitize=thread -pthread -o $(TARGET)_debug $(SRC)

fast:
	$(CXX) $(CXXFLAGS) -O3 -pthread -o $(TARGET)_fast $(SRC)

profile:
	$(CXX) $(CXXFLAGS) -O2 -pg -pthread -o $(TARGET)_prof $(SRC)

run: all
	./$(TARGET) --threads $(T) --mode pool --width 1280 --height 720 \
	            --samples 16 --depth 5 --output output/renders/render.ppm

animate: all
	./$(TARGET) --threads $(T) --mode pool --animate \
	            --width 1280 --height 720 --samples 16 --depth 5 \
	            --output output/animation/frame

timelapse: all
	@mkdir -p output/timelapse
	@for t in 1 2 4 6 8 12; do \
	    rm -f output/timelapse/frame_*.ppm output/timelapse/frame_*.png; \
	    ./$(TARGET) --threads $$t --mode pool --timelapse \
	        --width 1280 --height 720 --samples 16 --depth 5 \
	        --output /dev/null; \
	    bash scripts/make_video.sh timelapse $$t; \
	done
	ffmpeg -y \
	    -i output/timelapse/t1.mp4 -i output/timelapse/t2.mp4 \
	    -i output/timelapse/t4.mp4 -i output/timelapse/t6.mp4 \
	    -i output/timelapse/t8.mp4 -i output/timelapse/t12.mp4 \
	    -filter_complex "\
	    [0:v]scale=426:240,drawtext=text='1T':fontsize=24:fontcolor=white:x=10:y=10[v0];\
	    [1:v]scale=426:240,drawtext=text='2T':fontsize=24:fontcolor=white:x=10:y=10[v1];\
	    [2:v]scale=426:240,drawtext=text='4T':fontsize=24:fontcolor=white:x=10:y=10[v2];\
	    [3:v]scale=426:240,drawtext=text='6T':fontsize=24:fontcolor=white:x=10:y=10[v3];\
	    [4:v]scale=426:240,drawtext=text='8T':fontsize=24:fontcolor=white:x=10:y=10[v4];\
	    [5:v]scale=426:240,drawtext=text='12T':fontsize=24:fontcolor=white:x=10:y=10[v5];\
	    [v0][v1][v2]hstack=inputs=3[top];[v3][v4][v5]hstack=inputs=3[bot];[top][bot]vstack[out]" \
	    -map "[out]" output/timelapse/timelapse_comparison.mp4

plot:
	python3 scripts/plot_ahmdal.py

uml:
	java -jar plantuml.jar docs/uml/*.puml

# ---------------------------------------------------------------------------
# test: tests/ altındaki her standalone test TU'sunu derleyip HEMEN çalıştırır.
# Her satır ayrı bir shell çağrısıdır ve `make`, bir satır sıfır olmayan bir
# çıkış koduyla dönerse (derleme hatası VEYA test içindeki bir assert patlarsa)
# hedefi orada durdurur — yani "make test" kırmızıysa hangi test dosyasında
# durduğunu terminal çıktısından direkt görürsün, ekstra bir CI script'ine
# gerek yok. Testler birbirinden bağımsız (ortak state paylaşmıyorlar), bu
# yüzden sıralama önemli değil.
# Kullanım: make test
# ---------------------------------------------------------------------------
test:
	$(CXX) $(CXXFLAGS) -o /tmp/t_aabb tests/test_aabb.cpp && /tmp/t_aabb
	$(CXX) $(CXXFLAGS) -o /tmp/t_bvh tests/test_bvh.cpp && /tmp/t_bvh
	$(CXX) $(CXXFLAGS) -o /tmp/t_sphere tests/test_sphere.cpp && /tmp/t_sphere
	$(CXX) $(CXXFLAGS) -o /tmp/t_refit tests/test_refit.cpp && /tmp/t_refit

# ---------------------------------------------------------------------------
# bench: bench/bench_bvh.cpp'yi -O3 ile derler, $(TARGET)_bench (raytracer_bench)
# çıktısını üretir. -O3 şart: -O0/-O2 ile ölçülen süreler gerçek kullanım
# senaryosunu (all/run hedefi de -O2 kullanıyor ama bench için en agresif
# optimizasyon tercih edildi, BVH'nin gerçek üst sınır performansını görmek
# için) yansıtmaz. Bu hedef SADECE derler, ÇALIŞTIRMAZ — çıktı stdout'a CSV
# bastığı için nereye yönlendireceğin (dosya/pipe) sana kalsın diye.
# Kullanım: make bench && ./raytracer_bench > /tmp/bvh.csv
# ---------------------------------------------------------------------------
bench:
	$(CXX) $(CXXFLAGS) -O3 -o $(TARGET)_bench bench/bench_bvh.cpp

# ---------------------------------------------------------------------------
# bench-run: bench'i derler, CSV'yi output/bvh_scaling.csv'ye yazar, ardından
# plot_bvh_scaling.py'yi o CSV üzerinde çalıştırır — "make bench" + manuel
# yönlendirme + manuel plot komutunu tek adıma indiren kısayol.
# Kullanım: make bench-run
# ---------------------------------------------------------------------------
bench-run: bench
	@mkdir -p output
	./$(TARGET)_bench > output/bvh_scaling.csv
	python3 scripts/plot_bvh_scaling.py output/bvh_scaling.csv

# ---------------------------------------------------------------------------
# asan: AddressSanitizer (bellek güvenliği sanitizer'ı) ile ana render
# hedefini derler. DİKKAT: mevcut "debug" hedefi -fsanitize=thread (TSan,
# thread'ler arası veri yarışlarını yakalar) kullanıyor — bu FARKLI bir alet,
# TSan tek-thread'lik bir use-after-free/buffer-overflow'u YAKALAMAZ. "asan"
# hedefi Faz 2'nin iki bilinen risk noktasını (Bvh'nin flat node arena'sı ve
# build_scene_bench'in reserve-sonra-&element deseni) doğrulamak için var —
# ikisi de teoride dangling pointer/overflow üretebilecek "idiom"lar, ASan
# bunları çalışma zamanında (derleme zamanında değil) yakalar.
# -O1 (ASan -O0 ile çok yavaş, -O2/-O3 bazı hataları optimize edip gizleyebilir)
# ve -g (hata mesajlarında dosya/satır numarası göstermesi için) kullanılıyor.
# Kullanım (gate — sıfır ASan hatası beklenir):
#   make asan && ./raytracer_asan --scene bench --count 10000 --mode single \
#       --width 32 --height 32 --samples 1 --output /tmp/asan.ppm
# ---------------------------------------------------------------------------
asan:
	$(CXX) $(CXXFLAGS) -O1 -g -fsanitize=address -pthread -o $(TARGET)_asan $(SRC)

clean:
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_fast $(TARGET)_prof
	rm -f $(TARGET)_O0 $(TARGET)_O2 $(TARGET)_O3 raytracer_tsan raytracer_bench raytracer_asan
	rm -f output/renders/*.ppm output/renders/*.png
	rm -f output/animation/frame_*.ppm output/timelapse/frame_*.ppm
	rm -f output/bvh_scaling.csv
	rm -f gmon.out
