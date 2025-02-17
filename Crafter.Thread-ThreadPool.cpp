module;

#include <thread>
#include <functional>
#include <mutex>
#include <print>
#include <iostream>

module Crafter.Thread;

using namespace Crafter::Thread;


void ThreadPool::Start(int maxThreads)
{
	stopped = false;
	if (maxThreads > 0) {
		threadCount = maxThreads;
	}
	else {
		threadCount = std::thread::hardware_concurrency();
	}
	threadStorage = new ThreadStorage[threadCount];
	for (unsigned int i = 0; i < threadCount - 1; i++)
	{
		threadStorage[i].thread = std::thread(ThreadPool::WaitAndExecuteTask, threadStorage + i);
		threadIdMap[threadStorage[i].thread.get_id()] = &threadStorage[i];
	}
	threadIdMap[std::this_thread::get_id()] = &threadStorage[threadCount - 1];
	activeThreads = threadCount - 1;
}

void ThreadPool::Stop()
{
	stopped = true;
	for (unsigned int i = 0; i < threadCount - 1; i++)
	{
		threadStorage[i].cv.notify_one();
		threadStorage[i].thread.join();
	}
}

void ThreadPool::Enqueue(std::function<void(void)> task)
{
	unsigned int min = std::numeric_limits<unsigned int>::max();
	unsigned int minIndex = 0;
	for (unsigned int i = 0; i < activeThreads; i++)
	{
		unsigned int taskCount = threadStorage[i].taskCount;
		if (taskCount == 0) {
			minIndex = i;
			break;
		}
		if (taskCount < min) {
			min = taskCount;
			minIndex = i;
		}
	}
	{
		std::lock_guard lk(threadStorage[minIndex].mutex);
		threadStorage[minIndex].buffer.push_back(task);
		threadStorage[minIndex].taskCount++;
	}
	threadStorage[minIndex].cv.notify_one();
}

void ThreadPool::Enqueue(std::vector<std::function<void(void)>> tasks)
{
	unsigned int size = tasks.size();
	unsigned int tasksPerThread = size / activeThreads;
	int remainder = size % activeThreads;

	if(remainder) {
		for (unsigned int i = 0; i < activeThreads - 1; i++)
		{
			{
				std::lock_guard lk(threadStorage[i].mutex);
				threadStorage[i].buffer.insert(threadStorage[i].buffer.end(), tasks.begin() + tasksPerThread * i, tasks.begin() + (tasksPerThread * i + tasksPerThread));
				threadStorage[i].taskCount += tasksPerThread * i;
			}
			threadStorage[i].cv.notify_one();
		}
		{
			std::lock_guard lk(threadStorage[activeThreads - 1].mutex);
			threadStorage[activeThreads - 1].buffer.insert(threadStorage[activeThreads - 1].buffer.end(), tasks.begin() + tasksPerThread * (activeThreads - 1), tasks.begin() + (tasksPerThread * (activeThreads - 1) + remainder));
			threadStorage[activeThreads - 1].taskCount += tasksPerThread + remainder;
		}
		threadStorage[activeThreads - 1].cv.notify_one();
	} else{
		for (unsigned int i = 0; i < activeThreads; i++)
		{
			{
				std::lock_guard lk(threadStorage[i].mutex);
				threadStorage[i].buffer.insert(threadStorage[i].buffer.end(), tasks.begin() + tasksPerThread * i, tasks.begin() + (tasksPerThread * i + tasksPerThread));
				threadStorage[i].taskCount += tasksPerThread * i;
			}
			threadStorage[i].cv.notify_one();
		}
	}
}

void ThreadPool::JoinPool()
{
	activeThreads++;
	ThreadPool::WaitAndExecuteTask(threadStorage + threadCount - 1);
}

void ThreadPool::WaitAndExecuteTask(ThreadStorage* threadStorage)
{
	while (!stopped || !threadStorage->buffer.empty()) {
		std::unique_lock<std::mutex> lock(threadStorage->mutex);
		threadStorage->cv.wait(lock, [threadStorage] { return !threadStorage->buffer.empty() || stopped; });
		threadStorage->buffer.swap(threadStorage->tasks);
		lock.unlock();
		for (std::function<void(void)> task : threadStorage->tasks)
		{
			task();
			threadStorage->taskCount--;
		}
		threadStorage->tasks.clear();
	}
}