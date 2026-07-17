#pragma once
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    // n_threads: kaç worker thread başlatılacak
    explicit ThreadPool(int n_threads);
    ~ThreadPool();

    // FIX-05 (D-15): ThreadPool ham bir std::thread* sahibi (dtor'da delete[]).
    // Sığ bir kopya/taşıma double-free'ye yol açardı. Pool yalnızca main.cpp'de
    // yerel stack nesnesi olarak kullanıldığından kopyalamaya/taşımaya hiçbir kod
    // yolu ihtiyaç duymuyor → dört özel üyenin dördü de = delete (PPMWriter'ın
    // deleted-copy desenini bir adım öteye taşıyarak move'u da siliyoruz).
    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&)                 = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;

    // Task kuyruğuna ekle — thread-safe.
    // std::function<void()>: parametre/döndürme değeri olmayan callable.
    void submit(std::function<void()> task);

    // Kuyruktaki tüm işler bitsin, thread'leri join et.
    // Destructor otomatik çağırır — ama render bitince explicit çağrılabilir.
    void shutdown();

    // Çağıran thread'in pool içindeki indeksini döndürür (0..N-1).
    // Main thread veya pool dışı thread: -1.
    // Kullanım: progress.increment(pool.this_thread_idx())
    static int this_thread_idx();

private:
    std::thread* workers;
    int n_workers;
    std::queue<std::function<void()>> queue;
    std::mutex                        mtx;
    std::condition_variable           cv;
    bool                              stop_flag = false;

    void worker_loop(int idx);
};