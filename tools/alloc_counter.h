#pragma once
// alloc_counter.h — DOD-05 tahsis sayacı (Faz 4, Plan 04-01).
//
// C++17'nin 20 replaceable global allocation/deallocation fonksiyonunun
// (8 new + 12 delete) hepsini değiştirir; std::vector'ın iç büyümesi dahil
// programdaki her operator new çağrısının sayısını, byte'ını ve süresini toplar.
// Kullanım: allocstat::reset(); ...ölçülen iş...; allocstat::report("etiket");
//
// KISITLAR:
//  - Yalnızca TEK bir .cpp include edebilir. Replacement fonksiyonlar standart
//    gereği inline olamaz; ikinci bir çeviri birimi "multiple definition of
//    operator new" link hatası verir (ODR). Tek kullanıcı: bench/bench_dod.cpp.
//  - Yalnızca TEK thread'li ölçüm için güvenli. Sayaçlar atomik değil; eşzamanlı
//    tahsisler data race'tir (std::atomic / thread_local'a geçiş planlanıyor).
//  - Asla Makefile'ın SRC listesine (./raytracer'a) girmemeli.
#include <cstddef>  // std::size_t için
#include <chrono>   // std::chrono::steady_clock için (monotonik saat)
#include <cstdio>   // std::fprintf için (C++ akımları değil: kendi tahsisleri sayacı kirletirdi)
#include <cstdlib>  // std::malloc, std::aligned_alloc, std::free için
#include <new>      // std::bad_alloc, std::nothrow_t, std::align_val_t için

namespace allocstat {
    // Sabit-başlatmalı sayaçlar: operator new main()'den önce de çalışabilir,
    // dinamik başlatma static-init sırası tehlikesi yaratırdı.
    inline unsigned long long count = 0;
    inline unsigned long long bytes = 0;
    inline unsigned long long nanos = 0;

    // alloc_func(requested_sz) çağrısını zamanlar ve sayar.
    // requested_sz: allocator'a verilecek boyut, n: bytes'a eklenecek gerçek ayrılan boyut.
    template <class AllocFn>
    void* track(std::size_t requested_sz, AllocFn alloc_func, std::size_t n) {
        auto start = std::chrono::steady_clock::now();
        void* ptr = alloc_func(requested_sz);
        auto end = std::chrono::steady_clock::now();

        nanos += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        count += 1;
        bytes += n;

        return ptr;
    }

    inline void reset() {
        count = 0;
        bytes = 0;
        nanos = 0;
    }

    inline void report(const char* tag) {
        std::fprintf(stderr, "ALLOC[%s] count=%llu bytes=%llu total_ns=%llu\n", tag, count, bytes, nanos);
    }
}

// ---------------------------------------------------------------------------
// Tahsis fonksiyonları (8). Ölçüm yalnızca üçünde yapılır (düz, nothrow,
// aligned); geri kalanlar bunlara iletir (forwarding) — tek doğruluk kaynağı.
// ---------------------------------------------------------------------------

void* operator new(size_t sz) {
    void* ptr = allocstat::track(sz, std::malloc, sz);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
}

void* operator new[](size_t sz) {
    return operator new(sz);
}

void* operator new(size_t sz, const std::nothrow_t&) noexcept {
    return allocstat::track(sz, std::malloc, sz);
}

void* operator new[](size_t sz, const std::nothrow_t&) noexcept {
    return operator new(sz, std::nothrow);
}

// C++17 aligned allocation: hizalaması __STDCPP_DEFAULT_NEW_ALIGNMENT__'ı (16)
// aşan tipler (örn. alignas(64)) için derleyici new T'yi buraya yönlendirir.
void* operator new(size_t sz, std::align_val_t al) {
    std::size_t a = static_cast<std::size_t>(al);
    std::size_t rounded = ((sz + a - 1) / a) * a; // aligned_alloc boyutun hizalamanın katı olmasını ister
    void* ptr = allocstat::track(rounded, [a](std::size_t s) {
        return std::aligned_alloc(a, s);
    }, rounded);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
}

void* operator new[](size_t sz, std::align_val_t al) {
    return operator new(sz, al);
}

// Nothrow sürümler noexcept: fırlatan sürümün bad_alloc'u nullptr'a çevrilir.
void* operator new(size_t sz, std::align_val_t al, const std::nothrow_t&) noexcept {
    try {
        return operator new(sz, al);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](size_t sz, std::align_val_t al, const std::nothrow_t&) noexcept {
    try {
        return operator new[](sz, al);
    } catch (...) {
        return nullptr;
    }
}

// ---------------------------------------------------------------------------
// Serbest bırakma fonksiyonları (12). Her new malloc ailesiyle ayırdığı için
// hepsi std::free; eksik olan her biri ASan'da alloc-dealloc-mismatch demektir.
// Parametre sırası sabittir (size_t → align_val_t → nothrow_t); listede olmayan
// bir imza "değiştirme" değil, hiç çağrılmayan yeni bir overload olur.
// ---------------------------------------------------------------------------

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}
void operator delete[](void* ptr) noexcept {
    std::free(ptr);
}

// Nothrow: yalnızca new(std::nothrow) T'nin constructor'ı fırlatırsa derleyici çağırır.
void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    std::free(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    std::free(ptr);
}

// C++17 aligned deallocation (serbest bırakma).
void operator delete(void* ptr, std::align_val_t) noexcept {
    std::free(ptr);
}
void operator delete[](void* ptr, std::align_val_t) noexcept {
    std::free(ptr);
}

// C++14 sized deallocation: derleyici boyutu biliyorsa bunları çağırır.
void operator delete(void* ptr, std::size_t) noexcept {
    std::free(ptr);
}
void operator delete[](void* ptr, std::size_t) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept {
    std::free(ptr);
}
void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    std::free(ptr);
}
void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    std::free(ptr);
}
