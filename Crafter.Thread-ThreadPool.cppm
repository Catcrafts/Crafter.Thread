/*
Crafter.Thread
Copyright (C) 2025 Catcrafts
Catcrafts.net

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

module;

#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <condition_variable>

export module Crafter.Thread:ThreadPool;

namespace Crafter::Thread {
	struct ThreadStorage {
		std::vector<std::function<void(void)>> tasks;
		std::vector<std::function<void(void)>> buffer;
		std::condition_variable cv;
		std::mutex mutex;
		std::atomic<unsigned int> taskCount;
		std::thread thread;
	};

	export class ThreadPool {
	public:
		static inline std::atomic<bool> stopped;
		static void Start(int maxThreads = -1);
		static void Stop();
		static void Enqueue(std::function<void(void)> task);
		static void Enqueue(std::vector<std::function<void(void)>> tasks);
		static void JoinPool();
		static inline unsigned int threadCount;
		static inline std::atomic<unsigned int> activeThreads;
	private:
		static inline std::unordered_map<std::thread::id, ThreadStorage*> threadIdMap;
		static inline ThreadStorage* threadStorage;
		static void WaitAndExecuteTask(ThreadStorage* threadStorage);
	};
}