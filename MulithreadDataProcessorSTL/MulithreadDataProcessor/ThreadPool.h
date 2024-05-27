#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <memory>
#include <utility>
#include "EventAtomic.h"
#include "ScopedThread.h"
#include <concepts>
#include <vector>
#include <algorithm>
#include <memory>

class ThreadPool
{
public:
	ThreadPool() = default;

	//threadIndex is passed to callable as first argument
	template<typename Callable, typename... Args> requires std::invocable<Callable, std::size_t, Args...>
	void CreateThreads(std::size_t threadCount, Callable callable, Args&&... args) {
		auto f = [callable, ...args = std::forward<Args>(args)](std::size_t threadIndex) {
			std::invoke(callable, std::forward<std::size_t>(threadIndex), std::forward<Args>(args)...);
		};

		for (std::size_t i = 0u; i < threadCount; ++i) {
			//threads.emplace_back(std::thread(&ThreadPool::ThreadEntry<Callable, Args...>, this, callable, i, std::forward<Args>(args)...));
			threads.emplace_back(std::thread(f, i));
		}
	}

	////threadIndex is passed to callable as last argument
	//template<typename Callable, typename... Args> requires std::invocable<Callable, Args..., std::size_t>
	//void CreateThreads(std::size_t threadCount, Callable callable, Args&&... args) {
	//	auto f = [callable, ...args = std::forward<Args>(args)](std::stop_token stopToken, std::size_t threadIndex) {
	//		std::invoke(callable, std::forward<Args>(args)..., std::forward<std::size_t>(threadIndex));
	//	};

	//	for (std::size_t i = 0u; i < threadCount; ++i) {
	//		//threads.emplace_back(std::jthread(&ThreadPool::ThreadEntry<Callable, Args...>, this, callable, i, std::forward<Args>(args)...));
	//		threads.emplace_back(std::jthread(f, i));
	//	}
	//}

	//No need to call this automatically as threads are automatically joined when the ThreadPool is destroyed
	void JoinAll() {
		std::ranges::for_each(threads, [](ScopedThread& thread) {
			thread.Join();
		});
	}

	std::size_t ThreadCount() const {
		return threads.size();
	}
private:
	//could also use new std::jthread (C++ 20) instead of ScopedThread
	std::vector<ScopedThread> threads;

	//template<typename Callable, typename... Args>
	//void ThreadEntry(std::size_t threadIndex, Callable callable, Args&&... args) requires std::invocable< std::size_t, Callable, Args...> {
	//	std::invoke(callable, std::forward<Args>(args)..., std::forward<std::size_t>(threadIndex));
	//}
};

#endif