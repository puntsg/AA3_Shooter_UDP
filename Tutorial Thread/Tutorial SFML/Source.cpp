#include <iostream>
#include<thread>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <functional>

#define MAX_THREADS std::thread::hardware_concurrency()

std::mutex consoleMutex;
std::atomic <int> atomicInt(0);

int sharedInt = 0;
std::mutex sharedIntMutex;

//std::queue <std::function<void>> tasks


void Increase() {
	atomicInt.fetch_add(1);
}
void Count()
{
	int count = 0; 
	while (count < 10)
		count++;
}

void PrintThread() {
	
	//Tipos de Mutex
	//std::unique_lock <std::mutex> uniqueLock(consoleMutex);
	//std::lock_guard <std::mutex> lock(consoleMutex);
	//consoleMutex.lock();
	std::cout << "- Thread ID: " << std::this_thread::get_id() << std::endl;
	//consoleMutex.unlock();
}
/*void Worker() {
	bool closeThread = false;
	while (!closeThread)
	{
		std::function<void> task;
		{
			std::lock_guard<std::mutex> lock(taksMutex);
			if (!tasks.empty(){
				task = tasks.front();
				tasks.pop();
			}
			else
				closeThread = true;
		}
		if(task)
			task();
	}
}*/
void main() {
	std::cout<< "Max thread count: " << std::thread::hardware_concurrency() << std::endl;
	std::vector <std::thread> threads;

	//for (int i = 0; i < MAX_THREADS; i++) {
	//	tasks.push(Count);
	//	tasks.push(PrintThread)
	//	tasks.push(Increase);
	//}

	for (int i = 0; i < MAX_THREADS; i++) 
		threads.emplace_back(PrintThread);
	
	for (int i = 0; i < MAX_THREADS; i++)
		threads[i].join();

	std::cout << "Atomic Int value: " << atomicInt;
}