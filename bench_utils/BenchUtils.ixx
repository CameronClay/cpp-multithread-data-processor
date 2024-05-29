export module BenchUtils;

import <chrono>;
import <vector>;
import <algorithm>;
import <numeric>;

export namespace BenchUtils {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using namespace std::chrono_literals;


    void Cubed(std::size_t threadIndex, int& data) {
        data = (data * data * data);
    }

    //use lambda to test performance to make benchmarks more fair (need a lambda for algorithms library)
    auto CubedLambda = [](std::size_t threadIndex, int& data) {
        Cubed(threadIndex, data);
    };

    class A {
    public:
        void Do(std::size_t threadIndex, int& i) const {
            i = i * i * i;
        }
    };

    class Functor {
    public:
        void operator()(std::size_t threadIndex, int& i) {
            i = i * i * i;
        }
    };

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

    //throws std::logic_error if validation fails
    void ValidateData(const std::vector<int>& data, const std::vector<int>& toCompare) {
        if (!std::ranges::equal(data, toCompare)) {
            throw std::logic_error("Alogorithm did not process correctly (data and toCompare not equal)...");
        }
    }
}