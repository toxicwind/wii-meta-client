#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

struct DownloadTask {
    std::string url;
    std::string dest_path;
    std::string title;
    int64_t total_size;
    int64_t downloaded;
    bool completed;
    bool failed;
    std::string error;
};

class DownloadManager {
public:
    DownloadManager(int max_concurrent = 2);
    ~DownloadManager();

    void start();
    void stop();

    // Add task to queue
    void enqueue(const std::string& url, 
                 const std::string& dest,
                 const std::string& title = "");

    // Get queue status
    int pending_count();
    int active_count();
    int completed_count();

    // Progress callback: (task_index, downloaded, total, speed_kbps)
    std::function<void(int, int64_t, int64_t, float)> on_progress;

    // Completion callback: (task_index, success, error)
    std::function<void(int, bool, const std::string&)> on_complete;

private:
    std::vector<DownloadTask> tasks;
    std::queue<int> pending;
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    bool running;
    int max_concurrent;

    void worker_loop();
    bool download_task(int task_id);
};

#endif
