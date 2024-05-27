#include "ParallelProcessor.h"
#include "TaskPoolJThread.h"
#include <iostream>
#include <syncstream>
#include <string>
#include <vector>
#include <memory>
#include <numeric>
#include <format>
#include <chrono>
#include <exception>
#include <ranges>
#include <algorithm>
#include <thread>
#include <future>
#include <execution>

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using namespace std::chrono_literals;

const std::size_t NFUNC_CALLS = 1000000;
const std::size_t NTRIALS = 100;
const std::size_t THREAD_COUNT_MAX = std::thread::hardware_concurrency();


//no inline so function call doesnt get optimized out by the compiler
__declspec(noinline) void Cubed(int& data, std::size_t threadIndex) {
	data = (data * data * data);
}

double GetBenchResult(const high_resolution_clock::time_point& t1) {
	return duration<double, std::milli>(high_resolution_clock::now() - t1).count();
}

double AvgBenchResults(const std::vector<double>& results) {
	if (results.empty()) {
		return 0.0;
	}

	const double sum = std::reduce(std::cbegin(results), std::cend(results), 0.0);
	const double avg = sum / results.size();
	return avg;
}

void GenerateData(std::vector<int>& data) {
	std::generate(std::begin(data), std::end(data), [n = 0]() mutable { return n++; }); //reset vector to a list of indices to use as data
}

double BenchLinear(std::vector<int>& data, std::size_t threadIndex) {
	GenerateData(data);

	auto t1 = high_resolution_clock::now();

	for (std::size_t i = 0u; i < NFUNC_CALLS; ++i) {
		Cubed(data[i], threadIndex);
	}

	return GetBenchResult(t1); //measure performance
}

double BenchMultithreaded(std::vector<int>& data, std::size_t threadIndex) {
	GenerateData(data);

	TaskPool taskPool;
	taskPool.CreateThreads(threadIndex);

	auto t1 = high_resolution_clock::now();

	ParallelProcessor<void, int> mp(taskPool, Cubed);
	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (threadIndex * 10u), threadIndex);
	//mp.AbortProcessing();
	mp.WaitForCompetion();

	return GetBenchResult(t1);
}

//this is incredibly slow because it launches a new task NFUNC_CALLS times
double BenchAsync(std::vector<int>& data, std::size_t threadIndex) {
	GenerateData(data);

	auto t1 = high_resolution_clock::now();

	std::vector<std::future<void>> handles(NFUNC_CALLS);
	std::transform(std::begin(data), std::end(data), std::begin(handles), [](auto& num) {
		return std::async(std::launch::async,
			&Cubed, std::ref(num), 0u);
	});
	std::for_each(std::begin(handles), std::end(handles), [](auto& handle) {
		handle.get();
	});

	return GetBenchResult(t1); //measure performance
}

double BenchForEach(std::vector<int>& data, std::size_t threadIndex) {
	GenerateData(data);

	auto t1 = high_resolution_clock::now();

	std::for_each(std::execution::par, std::begin(data), std::end(data), [](auto& data) {
		Cubed(data, 0u);
	});

	return GetBenchResult(t1); //measure performance
}

void HoldConsoleOpen() {
	std::osyncstream(std::cout) << "\nPress any key to continue...";
	std::string res;
	std::getline(std::cin, res);
}

void Benchmark() {
	std::osyncstream(std::cout) << "Starting benchmark..." << "\n";

	std::vector<int> data(NFUNC_CALLS);
	std::vector<double> linearTime, multiTime, asyncTime, forEachTime;
	linearTime.reserve(NFUNC_CALLS);
	multiTime.reserve(NFUNC_CALLS);
	asyncTime.reserve(NFUNC_CALLS);
	forEachTime.reserve(NFUNC_CALLS);
		
	for (std::size_t threadIndex = 1u; threadIndex <= THREAD_COUNT_MAX; ++threadIndex) {
		for (std::size_t trialIndex = 0u; trialIndex < NTRIALS; ++trialIndex) {
			multiTime.push_back(BenchMultithreaded(data, threadIndex));
		}

		const double multiAvg = AvgBenchResults(multiTime);

		multiTime.clear();

		std::osyncstream(std::cout) << std::format("Avgs of {:d} trials of {:d} loops | multithreaded ({:d} threads) time = {:.3f}\n", NTRIALS, NFUNC_CALLS, threadIndex, multiAvg);
	}

	std::osyncstream(std::cout) << "--------------------------------------------------\n";

	{
		for (std::size_t trialIndex = 0u; trialIndex < NTRIALS; ++trialIndex) {
			linearTime.push_back(BenchLinear(data, 0u));
			forEachTime.push_back(BenchForEach(data, 0u));
			//asyncTime.push_back(BenchAsync(data, 0u));
		}

		const double linearAvg = AvgBenchResults(linearTime);
		const double forEachAvg = AvgBenchResults(forEachTime);
		//const double asyncAvg = AvgBenchResults(asyncTime);

		linearTime.clear();
		forEachTime.clear();
		//asyncTime.clear();

		std::osyncstream(std::cout) << std::format("Avgs of {:d} trials of {:d} loops | single threaded time = {:.3f} | std::for_each time = {:.3f} \n", NTRIALS, NFUNC_CALLS, linearAvg, forEachAvg);
		//std::osyncstream(std::cout) << std::format("Avgs of {:d} trials of {:d} loops | single threaded time = {:.3f} | std::async time = {:.3f} | std::for_each time = {:.3f} \n", NTRIALS, NFUNC_CALLS, linearAvg, asyncAvg, forEachAvg);
	}

	std::osyncstream(std::cout) << "Ending benchmark..." << "\n";
}

int main() {
	try {
		Benchmark();
	}
	catch (const std::exception& e) {
		//ThreadSafeCerr(e.what() + std::string("\n"));
		std::osyncstream(std::cerr) << e.what() << "\n";
	}

	HoldConsoleOpen();
	return 0;
}
