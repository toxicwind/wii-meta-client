#include "download.h"
#include "http.h"
#include <stdio.h>

DownloadManager::DownloadManager(int max_concurrent) 
    : running(false), max_concurrent(max_concurrent) {
}

DownloadManager::~DownloadManager() {
    stop();
}

void DownloadManager::start() {
    running = true;
    for (int i = 0; i < max_concurrent; i++) {
        workers.emplace_back(&DownloadManager::worker_loop, this);
    }
}

void DownloadManager::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        running = false;
    }
    cv.notify_all();
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
}

void DownloadManager::enqueue(const std::string& url, 
                              const std::string& dest,
                              const std::string& title) {
    std::lock_guard<std::mutex> lock(mutex);
    int id = tasks.size();
    tasks.push_back({url, dest, title, 0, 0, false, false, ""});
    pending.push(id);
    cv.notify_one();
}

int DownloadManager::pending_count() {
    std::lock_guard<std::mutex> lock(mutex);
    return pending.size();
}

int DownloadManager::active_count() {
    std::lock_guard<std::mutex> lock(mutex);
    int count = 0;
    for (const auto& t : tasks) {
        if (!t.completed && !t.failed) count++;
    }
    return count;
}

int DownloadManager::completed_count() {
    std::lock_guard<std::mutex> lock(mutex);
    int count = 0;
    for (const auto& t : tasks) {
        if (t.completed) count++;
    }
    return count;
}

void DownloadManager::worker_loop() {
    while (running) {
        int task_id = -1;
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this] { return !running || !pending.empty(); });
            if (!running) break;
            task_id = pending.front();
            pending.pop();
        }
        if (task_id >= 0) {
            download_task(task_id);
        }
    }
}

bool DownloadManager::download_task(int task_id) {
    // TODO: Implement actual download using HttpClient
    // with progress callbacks
    return false;
}
