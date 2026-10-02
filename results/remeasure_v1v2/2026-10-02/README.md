# v1 vs v2 re-measurement — 2026-10-02

**Why:** the course-era comparison (`results/comparison.csv`) reported v2 (thread-per-row) at 14.93x over v1
on the heavy scene, which is above the 12-logical-core count. The raw `perf stat` headers show that the v1
medium/heavy baselines were measured on `./raytracer_prof_O2`, the gprof build (`-pg`), while v2 was
measured on a normal build. The May 2026 raw files are copied, unmodified, into
[`may2026_originals/`](may2026_originals/) (named `<version>_<scene>_<file>`); see the `Performance counter stats for` line in each.

**Setup:** git worktree at `8ebf4be` (the commit that recorded the v2 heavy measurement), so the code is
exactly what was measured in May 2026, not today's code. That commit's `main.cpp` selects v1/v2 by
commenting lines in and out; `main_v1.diff` and `main_medium.diff` show the only edits made. All builds:
`g++ -std=c++17 -Isrc -O2 -pthread`, plus `-pg` for the `prof` binaries. Command:
`perf stat [-r 5] -e task-clock,instructions,cycles ./<binary>`. Ryzen 5 5600X (6C/12T), Linux 6.17.

| Scene | Binary | Time | Instructions | IPC (per thread) | CPUs utilized |
|---|---|---:|---:|---:|---:|
| heavy (200 spheres, 1280x720, 64 spp, depth 15) | v1 `-pg` | 354.7 s | 4.681 T | 2.86 | 1.0 |
| | **v1** | **134.9 s** | **2.6558 T** | **4.26** | 1.0 |
| | **v2** | **21.65 s** | **2.6560 T** | **2.29** | 11.8 |
| medium (5 spheres, 1280x720, 16 spp, depth 10), 5 runs | v1 `-pg` | 7.244 s ±0.16% | 75.00 G | 2.26 | 1.0 |
| | **v1** | **4.378 s ±0.14%** | **47.02 G** | **2.34** | 1.0 |
| | **v2** | **0.641 s ±0.79%** | **47.15 G** | **1.69** | 9.6 |

**Results**

- The May measurement is reproduced: v1 `-pg` heavy = 4.681 T instructions (May: 4.686 T).
- v1 and v2 execute the same work (heavy instruction counts differ by 0.007%).
- gprof instrumentation slowed v1 by **2.63x** (heavy) and **1.65x** (medium). The overhead tracks how many
  non-inlined calls each ray makes (`Sphere::hit` x 200 spheres vs x 5).
- Real speedup: **heavy 6.23x**, **medium 6.83x** (May figures: 14.93x, 11.26x).
- The ceiling is the 6 physical cores, not the 12 logical ones. Per physical core, v2 retires about
  `instructions / (time x 6 x clock)` = 4.51 IPC on heavy vs 4.26 for single-threaded v1: SMT adds ~6%,
  because one thread already keeps the core busy. On medium, single-threaded IPC is lower (2.34), so
  there is more room for SMT, but the short run is diluted by serial work (720 thread creations, PPM output).

## -O2 vs -O3 (v1, heavy)

The course-era finding "`-O3` is 4.8% slower than `-O2`, L1-icache misses +206%" was also measured on gprof
builds. Re-measured on the same worktree, `-r 3`, `-e task-clock,instructions,cycles,L1-icache-load-misses`:

| Binary | Time (3 runs) | Instructions | IPC | L1-icache misses |
|---|---:|---:|---:|---:|
| v1 `-O2` | 134.24 s median (140.27 / 134.24 / 134.22; mean 136.25 ±1.48%) | 2.6558 T | 4.22 | 0.99 M |
| v1 `-O3` | 130.07 s median (130.32 / 130.07 / 129.93; mean 130.11 ±0.09%) | 2.6265 T | 4.37 | 1.10 M |
| v1 `-O3 -pg` (1 run) | 360.5 s | 4.608 T | 2.77 | 2.55 M |

- On normal builds **`-O3` is ~3.1% faster** (medians), with 1.1% fewer instructions and higher IPC.
- L1-icache misses are about 1 M over ~6x10^11 cycles in both builds: negligible.
- Under `-pg`, `-O3` is slower (360.5 s vs 354.7 s for `-O2 -pg`, earlier the same day; May: 343.5 s vs 327.4 s).
  The "`-O3` slower" result only holds for gprof-instrumented builds.
- The May L1-icache comparison mixed builds: `may2026_originals/v1_heavy_perf_cache_O2.txt` names `./raytracer_prof_O2`
  but ran in 134.9 s, which is the non-`-pg` runtime (the same file name ran in 327 s at 08:50 that morning).
  `perf_cache_O3.txt` ran in 346 s (`-pg`). The May counts also include kernel-mode events (no `:u`), so they are
  not comparable with the counts above.
