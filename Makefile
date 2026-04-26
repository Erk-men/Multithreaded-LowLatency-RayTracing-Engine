CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc
SRC      := src/main.cpp src/renderer.cpp src/threadpool.cpp
TARGET   := raytracer
T        ?= 6
OPT      ?= -O2

.PHONY: all debug fast profile run bench animate timelapse plot uml report clean

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

bench:
	bash scripts/benchmark.sh

animate: all
	./$(TARGET) --threads $(T) --mode pool --animate \
	            --width 1280 --height 720 --samples 16 --depth 5 \
	            --output output/animation/frame

timelapse: all
	@for t in 1 2 4 6 8 12; do \
	    rm -f output/timelapse/frame_*.ppm; \
	    ./$(TARGET) --threads $$t --mode pool --timelapse \
	        --width 1280 --height 720 --samples 16 --depth 5 \
	        --output /dev/null; \
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
	python3 scripts/plot.py

uml:
	java -jar plantuml.jar docs/uml/*.puml

report:
	cd docs && pdflatex final_report.tex && pdflatex final_report.tex

clean:
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_fast $(TARGET)_prof
	rm -f $(TARGET)_O0 $(TARGET)_O2 $(TARGET)_O3 raytracer_tsan raytracer_bench
	rm -f output/renders/*.ppm output/renders/*.png
	rm -f output/animation/frame_*.ppm output/timelapse/frame_*.ppm
	rm -f gmon.out
