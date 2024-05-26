#ifndef TASKPOOL_H
#define TASKPOOL_H

#include "ThreadPool.h"
#include "Function.h"
#include "Queue.h"
#include <atomic>

using Task = Function<void(std::size_t threadIndex)>;
class TaskPool
{
public:
	TaskPool();
	~TaskPool();

	void CreateThreads(std::size_t threadCount);
	void QueueTask(const Task& action, std::size_t count);
	void ClearTasks();

	std::size_t ThreadCount() const;
	std::size_t TaskCount();
	bool HasTasks();
private:
	void DoTasks(std::size_t threadIndex);

	std::atomic<bool> exit;
	ThreadPool threads;
	Queue<Task> tasks;
};

#endif