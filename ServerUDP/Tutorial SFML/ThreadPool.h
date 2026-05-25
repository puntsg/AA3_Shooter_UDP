#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>

#define MAX_THREADS std::thread::hardware_concurrency()


class ThreadPool
{
public:
    ThreadPool(int numThreads);
    ~ThreadPool();

    // Mete un job en la cola
    void Enqueue(std::function<void()> task);

    int Size() const;

private:

    void Worker();

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasksMutex;
    bool stopping;
};
