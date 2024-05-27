#ifndef PARALLEL_PROCESSOR_H
#define PARALLEL_PROCESSOR_H

#include "EventAtomic.h"
#include "TaskPool.h"
#include "PFunc.h"
#include <cassert>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <limits>
#include <optional>

template<typename RT, typename Data>
using GlobalCallback = PFunc<RT, std::size_t, Data&>;

template<typename RT, typename Data, typename C>
using MemberCallback = PMFunc<RT, std::size_t, Data&, C&>;

template<typename Data>
class ParallelProcessorBase
{
public:
	static constexpr int INVALID_IDX = -1;
	static constexpr int START_INDEX = 0;
	static constexpr int EXIT_RESULT = INT_MIN;

	ParallelProcessorBase(TaskPool& taskPool)
		:
		taskPool(taskPool),
		processEv(),
		finishedEv(0u),
		processIncrement(0)
	{}

	~ParallelProcessorBase() {
		AbortProcessing();
	}

	//Returns true if processing was not in progress and started successfully
	bool StartProcessing(Data* data, int dataCount, int processIncrement, std::size_t taskCount) {
		if (CanStartProcessing()) {
			InitializeData(data, dataCount, processIncrement, taskCount);
			StartTasks(taskCount);

			return true;
		}

		return false;
	}

	//Blocks until processing is aborted. 
	//Returns true if processing was in progress and aborted successfully.
	bool AbortProcessing() {
		if (InProgress()) {
			aborting.store(true, std::memory_order_release);

			descriptor.index = descriptor.count;
			WaitForCompetion();

			aborting.store(false, std::memory_order_release);

			return true;
		}

		return false;
	}

	//Returns true if processing is being aborted
	bool IsAborting() const {
		return aborting.load(std::memory_order_acquire);
	}

	//Returns true if processing was started and is not finished.
	bool InProgress() {
		return processEv.IsSet() && !finishedEv.IsSet();
	}

	//Returns true if not aborting and not in progress.
	bool CanStartProcessing() {
		return !aborting.load(std::memory_order_acquire) && !InProgress();
	}

	//Wait until processing is complete.
	void WaitForCompetion() {
		finishedEv.Wait();
		ResetEvents(0u);
	}

	struct DataDescriptor
	{
		DataDescriptor()
			:
			index(START_INDEX),
			count(0),
			data(0)
		{}

		std::atomic<int> index;
		int count; //Entry count in data
		Data* data;
	};
protected:
	virtual void RunAlgorithm(std::size_t threadIndex, int startIndex, int endIndex) = 0;

	DataDescriptor descriptor;
	TaskPool& taskPool;

private:
	//Returns a range of data to process [lowIndex, highIndex[ (returns None if finished processing)
	std::optional<std::pair<int, int>> GetProcRange() {
		//Compare and swap (CAS) loop
		int newIndex;
		int oldIndex = descriptor.index.load(std::memory_order_acquire);
		do {
			if (oldIndex == descriptor.count) {
				return {};
			}

			newIndex = std::min(oldIndex + processIncrement, descriptor.count);
		} while (!descriptor.index.compare_exchange_weak(oldIndex, newIndex, std::memory_order_acq_rel));

		return std::optional{ std::make_pair(oldIndex, newIndex) };
	}

	void ResetEvents(std::size_t taskCount) {
		finishedEv.Reset(taskCount);
		processEv.Reset();
	}

	void InitializeData(Data* data, int dataCount, int processIncrement, std::size_t taskCount) {
		ResetEvents(taskCount);

		this->processIncrement = processIncrement;
		descriptor.index = 0;
		descriptor.count = dataCount;
		descriptor.data = data;
	}

	void StartTasks(std::size_t taskCount) {
		auto f = [this](std::size_t threadIndex) {
			this->ProcessData(threadIndex);
		};
		taskPool.QueueTask(Task{ f }, taskCount);
	    //taskPool.QueueTask(Task{ &ParallelProcessorBase::ProcessData, this }, taskCount);
		processEv.NotifyAll();
	}

	void ProcessData(std::size_t threadIndex) {
		processEv.Wait(); //Wait until data is available

		while (true) {
			if (!Process(threadIndex)) {
				break;
			}
		}

		finishedEv.NotifyAll();
	}

	//returns false when processing is complete and true otherwise
	bool Process(std::size_t threadIndex) {
		if (IsAborting()) {
			return false;
		}

		const auto indicies = GetProcRange();
		if (!indicies) {
			return false;
		}

		const auto [startIndex, endIndex] = *indicies;

		this->RunAlgorithm(threadIndex, startIndex, endIndex);

		return true;
	}

	int processIncrement;
	std::atomic<bool> aborting;
	Event processEv; //used to notify threads data is available and processsing is ready to start

	//could also potentially use std::barrier instead
	EventMPSC finishedEv; //used to signal threads processing is complete (or object is being destroyed)
};

template<typename RT, typename Data>
class ParallelProcessor : public ParallelProcessorBase<Data>
{
public:
	using CallBack_t = GlobalCallback<RT, Data>;
	//using CallBack_t = Function<RT(std::size_t, Data&)>;
	ParallelProcessor(TaskPool& taskPool, CallBack_t function)
		:
		ParallelProcessorBase<Data>(taskPool),
		function(function)
	{}

private:
	virtual void RunAlgorithm(std::size_t threadIndex, int startIndex, int endIndex) override {
		assert(endIndex <= ParallelProcessorBase<Data>::descriptor.count);

		//run algorithm within current thread in series
		for (Data* p = ParallelProcessorBase<Data>::descriptor.data + startIndex,
			*end = ParallelProcessorBase<Data>::descriptor.data + endIndex;
			p != end; ++p)
		{
			function(threadIndex, *p);
		}
	}

	CallBack_t function;
};

template<typename RT, typename Data, typename C>
class MParallelProcessor : public ParallelProcessorBase<Data>
{
public:
	using CallBack_t = MemberCallback<RT, Data, C>;
	MParallelProcessor(TaskPool& taskPool, CallBack_t function, C& obj)
		:
		ParallelProcessorBase<Data>(taskPool),
		function(function),
		obj(obj)
	{}

private:
	virtual void RunAlgorithm(std::size_t threadIndex, int startIndex, int endIndex) override {
		assert(endIndex <= ParallelProcessorBase<Data>::descriptor.count);

		//run algorithm within current thread in series
		for (Data* p = ParallelProcessorBase<Data>::descriptor.data + startIndex + 1,
			*end = ParallelProcessorBase<Data>::descriptor.data + endIndex;
			p != end; ++p)
		{
			function(*p, obj, threadIndex);
		}
	}

	CallBack_t function;
	C& obj;
};

#endif