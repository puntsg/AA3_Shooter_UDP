#pragma once

#include <SFML/Network.hpp>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class NetworkManager;

class ThreadPool
{
public:
    // Crea el pool con varios threads.
    ThreadPool(NetworkManager* networkManager, int numThreads);

    // Espera que los threads acaben.
    ~ThreadPool();

    // Mete un paquete en la cola.
    void Enqueue(int playerId, sf::Packet packet);

    int Size() const;

private:
    struct Task
    {
        int playerId;
        sf::Packet packet;
    };

    void Worker();

    NetworkManager* networkManager;
    std::vector<std::thread> threads;
    std::queue<Task> tasks;
    std::mutex tasksMutex;
    bool stopping;
};
