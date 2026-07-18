#!/bin/bash
# PPM kareleri PNG'ye cevirir, sonra ffmpeg ile MP4 uretir.
#
# Cikti duzeni (FIX-13 / D-14, Plan 04'te netlesen output/ yapisi):
#   animate            -> output/animation/frame_*.ppm -> output/animation.mp4
#   timelapse <N>      -> output/timelapse/frame_*.ppm -> output/timelapse/t<N>.mp4
#
# Kullanim:
#   scripts/make_video.sh                 # animate (varsayilan)
#   scripts/make_video.sh animate         # animate (acik)
#   scripts/make_video.sh timelapse <N>   # tek bir thread sayisinin karelerini t<N>.mp4'e kodlar
#
# Not: ffmpeg bu makinede kurulu DEGIL (sudo apt install ffmpeg). Yol/mantik
#      artik dogru; uctan uca video dogrulamasi ffmpeg gelene kadar ertelendi (D-14).

set -e

MODE="${1:-animate}"

# Bir kare klasorunu tek bir mp4'e kodlar: $1 = kaynak dizin, $2 = cikti dosyasi
encode_dir() {
    src_dir="$1"
    out_file="$2"
    shopt -s nullglob
    frames=("$src_dir"/frame_*.ppm)
    shopt -u nullglob
    if [ ${#frames[@]} -eq 0 ]; then
        echo "uyari: $src_dir icinde kare (frame_*.ppm) yok — atlaniyor"
        return 0
    fi
    echo "PPM -> PNG donusturuluyor ($src_dir)..."
    for f in "${frames[@]}"; do
        convert "$f" "${f%.ppm}.png"
    done
    echo "Video kodlaniyor -> $out_file"
    ffmpeg -y -framerate 24 -i "$src_dir/frame_%03d.png" \
        -c:v libx264 -pix_fmt yuv420p "$out_file"
    echo "Tamamlandi: $out_file"
}

if [ "$MODE" = "timelapse" ]; then
    TN="${2:?kullanim: scripts/make_video.sh timelapse <thread_sayisi>}"
    mkdir -p output/timelapse
    encode_dir "output/timelapse" "output/timelapse/t${TN}.mp4"
else
    mkdir -p output/animation
    encode_dir "output/animation" "output/animation.mp4"
fi
