# BVH render-path speedup, re-measured — 2026-10-02

**Why:** the README's headline figure (9224.37 ms → 22.07 ms, "418x") came from one run on each side.

**Code:** the original measuring commits, built from clean `git archive` copies (public-history hashes;
private-history equivalents in parentheses, identical `src/` trees):

| Role | Commit | Notes |
|---|---|---|
| brute force | `f5b1ecd` (`12866c7`) | last commit before the BVH was wired into `render_tile()` |
| BVH (median split) | `399f6f1` (`27f16fb`) | BVH wired in; SAH did not exist yet |
| BVH build time | `28264bd` (`81a9326`) | same render path plus the `BVH: ... build=` stderr line |

Build (the Makefile of that time): `g++ -std=c++17 -Wall -Wextra -Isrc -O2 -pthread -o raytracer src/main.cpp src/renderer.cpp src/threadpool.cpp`.
Command: `--scene bench --count 100000 --width 128 --height 128 --samples 1` (default `--mode single`, one thread).
`perf stat -r N -e task-clock`. Ryzen 5 5600X, Linux 6.17.

| | Runs | Render time (program's own timer), median | Spread |
|---|---:|---:|---|
| brute force | 5 | **8964.12 ms** | 8948.31 – 9024.70 |
| BVH | 10 | **21.69 ms** | 21.64 – 21.84 |
| BVH build | 5 | **76 ms** | 76 – 78 |

- Render-only speedup: 8964.12 / 21.69 = **413x**. The earlier single-run 418x was within run-to-run noise.
- The program's timer covers rendering only; the one-time BVH build happens before it starts. Including the
  build: 8964.12 / (21.69 + 76) = **~92x** for this single 128x128 frame.
- Whole-process wall time (`perf stat` elapsed: scene creation, build, render, output):
  8.984 s vs 0.117 s = **~77x**.
