#pragma once
#include <atomic>
#include <thread>
#include <chrono>
#include <cstdio>

static constexpr int MAX_THREADS = 16;

// =============================================================================
// ProgressBar — Unaligned Per-Thread Sayaçlar (v3 / --mode unaligned)
//
// sizeof(std::atomic<int>) = 4 byte, MAX_THREADS=16 → toplam 64 byte.
// Tüm sayaçlar aynı cache line'a sığar → thread 0 per_thread[0]'ı güncellediğinde
// thread 1'in per_thread[1]'i için önbelleği GEÇERSİZ KILAR → false sharing.
// =============================================================================
struct ProgressBar {
    std::atomic<int> per_thread[MAX_THREADS]; // KASITLI: padding yok
    int n_threads;
    int total_tiles;
    std::atomic<bool> running{false};
    std::thread display_thread;

    ProgressBar(int total, int threads) : n_threads(threads), total_tiles(total) {
        for (auto& c : per_thread) c.store(0, std::memory_order_relaxed);
    }

    ~ProgressBar() { stop(); }

    // Worker thread kendi sayacını artırır. thread_idx = 0..N-1
    void increment(int thread_idx) {
        per_thread[thread_idx].fetch_add(1, std::memory_order_relaxed);
    }

    int total_completed() const {
        int sum = 0;
        for (int i = 0; i < n_threads; ++i)
            sum += per_thread[i].load(std::memory_order_relaxed);
        return sum;
    }

    void start() {
        running.store(true);
        display_thread = std::thread([this] { display_loop(); });
    }

    void stop() {
        if (running.exchange(false) && display_thread.joinable())
            display_thread.join();
    }

private:
    void display_loop() {
        auto t0 = std::chrono::steady_clock::now();
        while (running.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            int done = total_completed();
            double pct = (total_tiles > 0) ? 100.0 * done / total_tiles : 0.0;
            double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - t0).count();
            int filled = (int)(pct / 5.0);
            int empty  = 20 - filled;
            printf("\r[");
            for (int i = 0; i < filled; ++i) printf("\xe2\x96\x88"); // █
            for (int i = 0; i < empty;  ++i) printf("\xe2\x96\x91"); // ░
            printf("] %.1f%% — Threads: %d — Elapsed: %.1fs   ",
                   pct, n_threads, elapsed);
            fflush(stdout);
        }
        printf("\n");
    }
};

// =============================================================================
// AlignedProgressBar — Cache-Line Aligned Sayaçlar (v4 / --mode aligned)
//
// alignas(64): her PaddedAtomic tam bir cache line (64 byte) kaplar.
// sizeof(std::atomic<int>)=4 byte → 60 byte padding otomatik eklenir.
// Thread 0 per_thread[0]'ı güncellerken thread 1'in per_thread[1]'i
// FARKLI cache line'da → false sharing YOK.
// =============================================================================
struct alignas(64) PaddedAtomic {
    std::atomic<int> val{0};
    // Compiler 60 byte padding ekler (alignas gereksinimi için)
};

struct AlignedProgressBar {
    PaddedAtomic per_thread[MAX_THREADS]; // Her eleman kendi cache line'ında
    int n_threads;
    int total_tiles;
    std::atomic<bool> running{false};
    std::thread display_thread;

    AlignedProgressBar(int total, int threads) : n_threads(threads), total_tiles(total) {}

    ~AlignedProgressBar() { stop(); }

    void increment(int thread_idx) {
        per_thread[thread_idx].val.fetch_add(1, std::memory_order_relaxed);
    }

    int total_completed() const {
        int sum = 0;
        for (int i = 0; i < n_threads; ++i)
            sum += per_thread[i].val.load(std::memory_order_relaxed);
        return sum;
    }

    void start() {
        running.store(true);
        display_thread = std::thread([this] { display_loop(); });
    }

    void stop() {
        if (running.exchange(false) && display_thread.joinable())
            display_thread.join();
    }

private:
    void display_loop() {
        auto t0 = std::chrono::steady_clock::now();
        while (running.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            int done = total_completed();
            double pct = (total_tiles > 0) ? 100.0 * done / total_tiles : 0.0;
            double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - t0).count();
            int filled = (int)(pct / 5.0);
            int empty  = 20 - filled;
            printf("\r[");
            for (int i = 0; i < filled; ++i) printf("\xe2\x96\x88");
            for (int i = 0; i < empty;  ++i) printf("\xe2\x96\x91");
            printf("] %.1f%% — Threads: %d — Elapsed: %.1fs   ",
                   pct, n_threads, elapsed);
            fflush(stdout);
        }
        printf("\n");
    }
};