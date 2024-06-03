export module ParallelProcessor;

import <cassert>;
import <atomic>;
import <mutex>;
import <condition_variable>;
import <algorithm>;
import <limits>;
import <optional>;
import <concepts>;
import <type_traits>;
import <utility>;
import <iterator>;
import EventAtomic;
import TaskPool;
import FunctionTraits;

//in C++ 17 constructors can deduce class template arguments
export template<typename Callable, typename IOIterator>
class ParallelProcessor
{
public:
	static constexpr int INVALID_IDX = -1;
	static constexpr int START_INDEX = 0;
	static constexpr int EXIT_RESULT = INT_MIN;

    //Parameters:
	//- taskPool: Task pool to use
	//- callable: Function to call (callable(std::size_t threadIndex, std::iter_reference_t<IOIterator> data))
	//- first: mutable random-access iterator to the first element
	//- last: mutable random-access iterator to the last element
	//- cannot use Callable here because perfect forwarding requires a function template parameter
	//requires std::invocable<U, std::size_t, std::add_lvalue_reference_t<std::invoke_result_t<IOIterator::operator*>>> && std::random_access_iterator<IOIterator>
	template<typename U>
	explicit ParallelProcessor(TaskPool& taskPool, U&& callable, IOIterator first, IOIterator last) requires std::invocable<U, std::size_t, std::iter_reference_t<IOIterator>> && std::random_access_iterator<IOIterator>
		:
		taskPool(taskPool),
		callable(std::forward<U>(callable)),
		processEv(),
		finishedEv(0u),
		processIncrement(0),
		descriptor(first, last)
	{}

	ParallelProcessor(ParallelProcessor&&) = default;
	ParallelProcessor& operator=(ParallelProcessor&&) = default;
	ParallelProcessor(const ParallelProcessor&) = delete;
	ParallelProcessor& operator=(const ParallelProcessor&) = delete;

	~ParallelProcessor() {
		AbortProcessing();
	}

	//Returns true if processing was not in progress and started successfully
	bool StartProcessing(int processIncrement, std::size_t taskCount) {
		if (CanStartProcessing()) {
			InitializeData(processIncrement, taskCount);
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
	inline bool IsAborting() const {
		return aborting.load(std::memory_order_acquire);
	}

	//Returns true if processing was started and is not finished.
	inline bool InProgress() {
		return processEv.IsSet() && !finishedEv.IsSet();
	}

	//Returns true if not aborting and not in progress.
	inline bool CanStartProcessing() {
		return !aborting.load(std::memory_order_acquire) && !InProgress();
	}

	//Wait until processing is complete.
	void WaitForCompetion() {
		finishedEv.Wait();
		ResetEvents(0u);
	}

	struct DataDescriptor
	{
		DataDescriptor(IOIterator first, IOIterator last)
			:
			index(START_INDEX),
			count(last - first),
			first(first),
			last(last)
		{}

		IOIterator begin() {
			return first;
		}

		IOIterator end() {
			return last;
		}

		std::atomic<int> index;
		int count; //Entry count in data
		IOIterator first, last;
	};
protected:
	DataDescriptor descriptor;
	TaskPool& taskPool;

private:
	void ResetEvents(std::size_t taskCount) {
		finishedEv.Reset(taskCount);
		processEv.Reset();
	}

	void InitializeData(int processIncrement, std::size_t taskCount) {
		ResetEvents(taskCount);

		this->processIncrement = processIncrement;
		descriptor.index = 0;
	}

	void StartTasks(std::size_t taskCount) {
		auto f = [this](std::size_t threadIndex) {
			this->ProcessData(threadIndex);
		};
		taskPool.QueueTask(Task{ std::move(f) }, taskCount);
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

	//Returns a range of data to process [lowIndex, highIndex[ (returns None if finished processing)
	//fine to inline because only called in one place
	inline std::optional<std::pair<int, int>> GetProcRange() {
		//Compare and swap (CAS) loop
		int newIndex;
		int oldIndex = descriptor.index.load(std::memory_order_acquire);
		do {
			if (oldIndex == descriptor.count) {
				return {};
			}

			newIndex = std::min(oldIndex + processIncrement, descriptor.count);
		} while (!descriptor.index.compare_exchange_weak(oldIndex, newIndex, std::memory_order_relaxed));

		return std::optional{ std::make_pair(oldIndex, newIndex) };
	}


	//returns false when processing is complete and true otherwise
	//fine to inline because only called in one place
	inline bool Process(std::size_t threadIndex) {
		if (IsAborting()) {
			return false;
		}

		const auto indicies = GetProcRange();
		if (!indicies) {
			return false;
		}

		const auto [startIndex, endIndex] = *indicies;

		RunAlgorithm(threadIndex, startIndex, endIndex);

		return true;
	}

	//fine to inline because only called in one place
	inline void RunAlgorithm(std::size_t threadIndex, int startIndex, int endIndex) {
		assert(endIndex <= descriptor.count);

		//run algorithm within current thread in series
		for (auto it = std::begin(descriptor) + startIndex,
			end = std::begin(descriptor) + endIndex;
			it != end; ++it)
		{
			callable(threadIndex, *it);
		}
	}

	Callable callable;
	int processIncrement;
	std::atomic<bool> aborting;
	Event processEv; //used to notify threads data is available and processsing is ready to start

	//could also potentially use std::barrier instead
	EventMPSC finishedEv; //used to signal threads processing is complete (or object is being destroyed)
};

//C++ 17 template deduction guide needed both to deduce typeof Callable from the Universal Reference (perfect forwarding) of template argument U
//... and also to deduce type of T from the passed Callable
export template<typename U, typename IOIterator>
ParallelProcessor(TaskPool&, U&&, IOIterator, IOIterator) -> ParallelProcessor<std::remove_cvref_t<U>, IOIterator>;

// export template <typename U>
// ParallelProcessor(TaskPool&, U callable) -> ParallelProcessor<std::remove_cvref_t<U>, std::remove_reference_t<ftraits::fextract_arg2<U>>>;