#include "ThreadPool.h"

ThreadPool::ThreadPool(int numThreads) : stopping(false)
{
    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(&ThreadPool::Worker, this);
}

ThreadPool::~ThreadPool()
{
    stopping = true;
    cv.notify_all();

    for (std::thread& t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::Enqueue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

int ThreadPool::Size() const
{
    return static_cast<int>(threads.size());
}

void ThreadPool::Worker()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(tasksMutex);
            // Duerme hasta hay trabajo o se quiera parar
            cv.wait(lock, [this] { return !tasks.empty() || stopping.load(); });

            if (stopping && tasks.empty())
                return;

            task = std::move(tasks.front());
            tasks.pop();
        }

        task();
    }
}
