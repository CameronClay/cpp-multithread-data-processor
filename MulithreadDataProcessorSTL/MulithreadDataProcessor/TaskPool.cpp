#include "TaskPool.h"
#include <utility>

TaskPool::TaskPool()
	:
	exit(false)
{}

TaskPool::~TaskPool() {
	exit.store(true, std::memory_order_release);
}

void TaskPool::CreateThreads(std::size_t threadCount) {
	auto f = [this](std::size_t threadIndex) {
		DoTasks(threadIndex);
	};
	threads.CreateThreads(threadCount, std::move(f));
	//threads.CreateThreads(threadCount, Function<void(std::size_t)>(f));
}

void TaskPool::QueueTask(const Task& action, std::size_t count) {
	for (std::size_t i = 0u; i < count; ++i) {
		tasks.Enqueue(action);
	}
}

void TaskPool::ClearTasks() {
	tasks.Clear();
}

void TaskPool::DoTasks(std::size_t threadIndex) {
	while (!exit.load(std::memory_order_acquire)) {
		const auto task = tasks.Dequeue();
		if (task) {
			(*task)(threadIndex); //execute task
		}
	}
}

std::size_t TaskPool::ThreadCount() const {
	return threads.ThreadCount();
}

std::size_t TaskPool::TaskCount() {
	return tasks.Size();
}

bool TaskPool::HasTasks() {
	return !tasks.Empty();
}