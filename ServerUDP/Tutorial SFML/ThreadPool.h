#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool
{
public:
    /// Crea pool con num de threads
    ThreadPool(int numThreads);

    /// Espera a que terminen y destruye el pool.
    ~ThreadPool();

    /// Encola tarea
    void Enqueue(std::function<void()> task);

    int Size() const;

private:

    void Worker();

    std::vector<std::thread>            threads;
    std::queue<std::function<void()>>   tasks;
    std::mutex                          tasksMutex;
    std::condition_variable             cv;
    std::atomic<bool>                   stopping;
};