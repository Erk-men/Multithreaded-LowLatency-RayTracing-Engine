#!/bin/bash
# PPM frame'leri PNG'ye çevir, sonra ffmpeg ile MP4 yap

mkdir -p output

echo "PPM -> PNG dönüştürülüyor..."
for f in frames/frame_*.ppm; do
    convert "$f" "${f%.ppm}.png"
done

echo "Video oluşturuluyor..."
ffmpeg -y -framerate 24 -i frames/frame_%03d.png \
    -c:v libx264 -pix_fmt yuv420p output/animation.mp4

echo "Tamamlandı: output/animation.mp4"
