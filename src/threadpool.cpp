#include "threadpool.h"
#include <stdexcept>

// Thread-local worker index: pool dışı thread için -1
thread_local int tl_worker_idx = -1;

int ThreadPool::this_thread_idx() {
    return tl_worker_idx;
}

void ThreadPool::worker_loop(int idx) {
    tl_worker_idx = idx; // Bu thread'in kimliği

    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            // stop_flag=true VE kuyruk boşsa → çık
            // Spurious wakeup: predicate her uyanışta yeniden kontrol edilir
            cv.wait(lock, [this] { return stop_flag || !queue.empty(); });
            if (stop_flag && queue.empty()) return;
            task = std::move(queue.front());
            queue.pop();
        } // lock serbest bırakıldı — task çalışırken kuyruk erişilebilir

        task();
    }
}

ThreadPool::ThreadPool(int n) {
        // FIX-07 (D-07): n<1 hard-fail. Aksi halde n==0 → sıfır uzunluklu pool,
        // hiçbir worker yok, submit edilen tile'lar sonsuza dek kuyrukta kalır
        // (hang); n<0 → new std::thread[n] uncaught std::bad_array_new_length.
        // parse_args doğrulamasını bypass eden bir çağıran olsa bile pool kendini
        // savunur. NOT: bu src/ içindeki ilk throw — kod tabanının tek hata
        // konvansiyonu şimdiye dek print-to-stderr + return idi.
        if (n < 1)
            throw std::invalid_argument("ThreadPool: n_threads must be >= 1");

        n_workers = n;
        workers = new std::thread[n];
        for (int i = 0; i < n; ++i) {
            workers[i] = std::thread([this, i] { worker_loop(i); });
        }
}


ThreadPool::~ThreadPool() {
    if (!stop_flag) shutdown();
    delete[] workers;
}

void ThreadPool::submit(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(std::move(task));
    }
    cv.notify_one(); // Bir worker'ı uyandır
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop_flag = true;
    }
    cv.notify_all(); // Tüm worker'ları uyandır (kuyruk bitti, çıkın)
        for (int i = 0; i < n_workers; ++i)
        if (workers[i].joinable()) workers[i].join();
}