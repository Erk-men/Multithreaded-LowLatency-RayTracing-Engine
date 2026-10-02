# TSan clean-run record — 2026-10-02

Binary: `make debug` (`g++ 13.3 -O0 -g -fsanitize=thread`), commit `f51b48d`, Linux 6.17, Ryzen 5 5600X.

Every run is prefixed with `setarch $(uname -m) -R` (per-process ASLR off). Without it, gcc 13's TSan
aborts at startup on this kernel with `FATAL: ThreadSanitizer: unexpected memory mapping`.

| Log | Mode | Scene | Resolution | Tiles | TSan reports | Completed |
|---|---|---|---|---|---|---|
| pool_simple.log | pool, 12 workers | simple | 160x90, spp 2 | 6 | 0 | yes |
| pool_medium.log | pool, 12 workers | medium | 160x90, spp 2 | 6 | 0 | yes |
| pool_complex.log | pool, 12 workers | complex | 160x90, spp 2 | 6 | 0 | yes |
| pool_bench.log | pool, 12 workers | bench n=1000 | 160x90, spp 2 | 6 | 0 | yes |
| naive_simple.log | thread-per-row (90 threads) | simple | 160x90, spp 2 | — | 0 | yes |
| pool_complex_hd.log | pool, 12 workers | complex | 1280x720, spp 1 | 240 | 0 | yes |
| pool_bench_hd.log | pool, 12 workers | bench n=1000 | 1280x720, spp 1 | 240 | 0 | yes |

"Completed" = the log contains the `Kaydedildi:` line (render finished and image written), checked
separately from the report count: a run that aborts at startup also shows zero TSan reports.

Example:

```bash
setarch $(uname -m) -R ./raytracer_debug --mode pool --threads 12 --scene complex \
  --width 1280 --height 720 --samples 1 --depth 5 --output /tmp/tsan_complex_hd.ppm
```

Scope: TSan is a dynamic tool and only covers the code paths executed above.
