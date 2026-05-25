#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool(int numThreads)
{
    stopping = false;
    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(&ThreadPool::Worker, this);
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        stopping = true;
    }

    for (int i = 0; i < threads.size(); i++)
    {
        if (threads[i].joinable())
            threads[i].join();
    }
}


void ThreadPool::Enqueue(std::function<void()> task)
{
    // Pon un job a la cola
    std::lock_guard<std::mutex> lock(tasksMutex);
    tasks.push(task);
}

int ThreadPool::Size() const
{
    return threads.size();
}


void ThreadPool::Worker()
{
    bool closeThread = false;
    while (!closeThread)
    {
        std::function<void()> task;
        
        {
            std::lock_guard<std::mutex> lock(tasksMutex);
            
            if (!tasks.empty())
            {
                task = tasks.front();
                tasks.pop();
            }
            else if (stopping)
            {
                closeThread = true;
            }
        }

        if (task)
        {
            task();
        }
        else if (!closeThread)
        {
            // Si no hay tarea duemo los cores
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
