#include "ThreadPool.h"
#include "NetworkManager.h"

ThreadPool::ThreadPool(NetworkManager* networkManager, int numThreads)
    : networkManager(networkManager)
    , stopping(false)
{
    if (numThreads <= 0)
    {
        numThreads = 1;
    }

    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back(&ThreadPool::Worker, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        stopping = true;
    }

    condition.notify_all();

    for (std::thread& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void ThreadPool::Enqueue(int playerId, sf::Packet packet)
{
    Task task;
    task.playerId = playerId;
    task.packet = packet;

    {
        std::lock_guard<std::mutex> lock(tasksMutex);

        if (stopping)
        {
            return;
        }

        tasks.push(task);
    }

    condition.notify_one();
}

int ThreadPool::Size() const
{
    return static_cast<int>(threads.size());
}

void ThreadPool::Worker()
{
    while (true)
    {
        Task task;

        {
            std::unique_lock<std::mutex> lock(tasksMutex);

            // El thread duerme aqui hasta que llega un paquete
            while (tasks.empty() && !stopping)
            {
                condition.wait(lock);
            }

            if (stopping && tasks.empty())
            {
                return;
            }

            if (!tasks.empty())
            {
                task = tasks.front();
                tasks.pop();
            }
        }

        networkManager->ProcessPacketFromPool(task.playerId, task.packet);
    }
}
