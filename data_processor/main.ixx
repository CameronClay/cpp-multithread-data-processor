import <iostream>;
import <syncstream>;
import <string>;
import <vector>;
import <memory>;
import <numeric>;
import <format>;
import <chrono>;
import <exception>;
import <ranges>;
import <algorithm>;
import <thread>;
import <future>;
import <execution>;
import <stdexcept>;
import BenchUtils;
import ParallelProcessor;
import TaskPool;

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using namespace std::chrono_literals;

const std::size_t NFUNC_CALLS = 4000000;
const std::size_t NTRIALS = 100;
const std::size_t THREAD_COUNT_MAX = std::thread::hardware_concurrency();

double BenchLinear(std::size_t trialIndex, std::vector<int>& data, std::size_t threadIndex) {
	BenchUtils::GenerateData(data);

	const auto t1 = high_resolution_clock::now();

	for (std::size_t i = 0u; i < NFUNC_CALLS; ++i) {
		BenchUtils::CubedLambda(threadIndex, data[i]);
	}

	return BenchUtils::GetBenchResult(t1); //measure performance
}

double BenchMultithreaded(std::size_t trialIndex, std::vector<int>& data, std::size_t threadIndex) {
	BenchUtils::GenerateData(data);

	TaskPool taskPool;
	taskPool.CreateThreads(threadIndex);

	const auto t1 = high_resolution_clock::now();

	ParallelProcessor mp(taskPool, BenchUtils::CubedLambda);
	//ParallelProcessor mp(taskPool, static_cast<decltype(CubedLambda)>(CubedLambda)); //static_cast converts lvalue to rvalue (not ncessary with perfect forwarding)
	//ParallelProcessor mp(taskPool, static_cast<void(*)(std::size_t, int&)>(CubedLambda));
	//ParallelProcessor mp(taskPool, &Cubed);
	 
	//Functor functor;
	//ParallelProcessor mp(taskPool, std::move(functor));
	 
	//auto f = [&](std::size_t threadIndex, int& i) {
	//	Cubed(threadIndex, i);
	//};	 
	//ParallelProcessor mp(taskPool, std::move(f));

	//ParallelProcessor mp(taskPool, std::function(CubedLambda));

	//ParallelProcessor mp(taskPool, Function<void(std::size_t, int&)>(&Cubed));

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (threadIndex * 10u), threadIndex);
	//mp.AbortProcessing();
	mp.WaitForCompetion();

	return BenchUtils::GetBenchResult(t1);
}

double BenchForEach(std::size_t trialIndex, std::vector<int>& data, std::size_t threadIndex) {
	BenchUtils::GenerateData(data);

	const auto t1 = high_resolution_clock::now();

	std::for_each(std::execution::par, std::begin(data), std::end(data), [](auto& data) { // passing lambda directly provides massive speed improvement (probably because Cubed is getting inlined)
		BenchUtils::CubedLambda(0u, data);
	});

	return BenchUtils::GetBenchResult(t1); //measure performance
}

//this is incredibly slow because it launches a new task NFUNC_CALLS times
double BenchAsync(std::size_t trialIndex, std::vector<int>& data, std::size_t threadIndex) {
	BenchUtils::GenerateData(data);

	const auto t1 = high_resolution_clock::now();

	std::vector<std::future<void>> handles(NFUNC_CALLS);
	std::transform(std::begin(data), std::end(data), std::begin(handles), [](auto& num) {
		return std::async(std::launch::async,
			BenchUtils::CubedLambda, 0u, std::ref(num));
	});
	std::for_each(std::begin(handles), std::end(handles), [](auto& handle) {
		handle.get();
	});

	return BenchUtils::GetBenchResult(t1); //measure performance
}

void HoldConsoleOpen() {
	std::osyncstream(std::cout) << "\nPress any key to continue...";
	std::string res;
	std::getline(std::cin, res);
}

void Benchmark() {
	std::osyncstream(std::cout) << "Starting benchmark..." << "\n";

	std::vector<int> data(NFUNC_CALLS);
	std::vector<int> dataToCompare(NFUNC_CALLS);
	std::vector<double> linearTime, multiTime, asyncTime, forEachTime;
	linearTime.reserve(NFUNC_CALLS);
	multiTime.reserve(NFUNC_CALLS);
	asyncTime.reserve(NFUNC_CALLS);
	forEachTime.reserve(NFUNC_CALLS);

	{
		for (std::size_t trialIndex = 0u; trialIndex < NTRIALS; ++trialIndex) {
			linearTime.push_back(BenchLinear(trialIndex, dataToCompare, 0u));
			forEachTime.push_back(BenchForEach(trialIndex, data, 0u));
			BenchUtils::ValidateData(data, dataToCompare); //validate all data was processed correctly (systems test)
			//asyncTime.push_back(BenchAsync(data, 0u));
		}

		const double linearAvg = BenchUtils::AvgBenchResults(linearTime);
		const double forEachAvg =BenchUtils:: AvgBenchResults(forEachTime);
		//const double asyncAvg = AvgBenchResults(asyncTime);

		linearTime.clear();
		forEachTime.clear();
		//asyncTime.clear();

		std::osyncstream(std::cout) << std::format("Avgs of {:d} trials of {:d} loops | single threaded time = {:.3f} | std::for_each time = {:.3f} \n", NTRIALS, NFUNC_CALLS, linearAvg, forEachAvg);
	}

	std::osyncstream(std::cout) << "--------------------------------------------------\n";
		
	for (std::size_t threadIndex = 1u; threadIndex <= THREAD_COUNT_MAX; ++threadIndex) {
		for (std::size_t trialIndex = 0u; trialIndex < NTRIALS; ++trialIndex) {
			multiTime.push_back(BenchMultithreaded(trialIndex, data, threadIndex));
			BenchUtils::ValidateData(data, dataToCompare); //validate all data was processed correctly (systems test)
		}

		const double multiAvg = BenchUtils::AvgBenchResults(multiTime);

		multiTime.clear();

		std::osyncstream(std::cout) << std::format("Avgs of {:d} trials of {:d} loops | multithreaded ({:d} threads) time = {:.3f}\n", NTRIALS, NFUNC_CALLS, threadIndex, multiAvg);
	}

	std::osyncstream(std::cout) << "--------------------------------------------------\n";
	std::osyncstream(std::cout) << "Ending benchmark..." << "\n";
}

int main() {
	try {
		Benchmark();
	}
	catch (const std::exception& e) {
		std::osyncstream(std::cerr) << e.what() << "\n";
	}

	HoldConsoleOpen();
	return 0;
}