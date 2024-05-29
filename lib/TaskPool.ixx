export module TaskPool;

import <atomic>;
import <utility>;
import ThreadPool;
import Queue;
import Function;

export using Task = Function<void(std::size_t threadIndex)>;
export class TaskPool
{
public:
	TaskPool()
		:
		exit(false)
	{}
	~TaskPool() {
		exit.store(true, std::memory_order_release);
	}

	void CreateThreads(std::size_t threadCount) {
		auto f = [this](std::size_t threadIndex) {
			DoTasks(threadIndex);
		};
		threads.CreateThreads(threadCount, std::move(f));
		//threads.CreateThreads(threadCount, Function<void(std::size_t)>(f));
	}
	void QueueTask(const Task& action, std::size_t count) {
		for (std::size_t i = 0u; i < count; ++i) {
			tasks.Enqueue(action);
		}
	}
	void ClearTasks() {
		tasks.Clear();
	}

	std::size_t ThreadCount() const {
		return threads.ThreadCount();
	}
	std::size_t TaskCount() {
		return tasks.Size();
	}
	bool HasTasks() {
		return !tasks.Empty();
	}
private:
	void DoTasks(std::size_t threadIndex) {
		while (!exit.load(std::memory_order_acquire)) {
			const auto task = tasks.Dequeue();
			if (task) {
				(*task)(threadIndex); //execute task
			}
		}
	}

	std::atomic<bool> exit;
	ThreadPool threads;
	Queue<Task> tasks;
};