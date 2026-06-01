#include "threadpool.h"

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