import <gmock/gmock-matchers.h>;
import <vector>;
import <chrono>;
import <algorithm>;
import <functional>;

import ParallelProcessor;
import TaskPool;
import BenchUtils;
import Function;

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using namespace std::chrono_literals;

const std::size_t NFUNC_CALLS = 100000;
const std::size_t THREAD_COUNT_MAX = std::thread::hardware_concurrency();

using namespace testing;

class ParallelProcessorTest : public testing::Test {
protected:
    static void GenerateData(std::vector<int>& data) {
        std::generate(std::begin(data), std::end(data), [n = 0]() mutable { return n++; }); //reset vector to a list of indices to use as data
    }

    static void GenerateValidationData(std::vector<int>& data) {
        for (std::size_t i = 0u; i < NFUNC_CALLS; ++i) {
            BenchUtils::CubedLambda(0u, data[i]);
        } 
    }

    ParallelProcessorTest() :
        testing::Test(),
        data(NFUNC_CALLS),
        validationData(NFUNC_CALLS),
        taskPool()
    {}

    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}

    virtual void SetUp() override {
        GenerateData(validationData);
        GenerateValidationData(validationData);
        GenerateData(data);
        taskPool.CreateThreads(THREAD_COUNT_MAX);
    }

    virtual void TearDown() override {}

    bool ValidateData() const {
        try {
            BenchUtils::ValidateData(data, validationData);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    TaskPool taskPool;
    std::vector<int> validationData;
    std::vector<int> data;
};

TEST_F(ParallelProcessorTest, With_Lambda)
{
	ParallelProcessor mp(taskPool, BenchUtils::CubedLambda);

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());

}

TEST_F(ParallelProcessorTest, With_LambdaFunctionPointer)
{
	ParallelProcessor mp(taskPool, static_cast<void(*)(std::size_t, int&)>(BenchUtils::CubedLambda));

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_MovedLambda)
{
    auto f = [&](std::size_t threadIndex, int& i) {
        BenchUtils::CubedLambda(threadIndex, i);
    };

	ParallelProcessor mp(taskPool, std::move(f));

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_Functor)
{
    BenchUtils::Functor functor;
	ParallelProcessor mp(taskPool, functor);

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_MovedFunctor)
{
    BenchUtils::Functor functor;
	ParallelProcessor mp(taskPool, std::move(functor));

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_std_function)
{
    auto f = std::function(BenchUtils::CubedLambda);
	ParallelProcessor mp(taskPool, f);

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_Moved_std_function)
{
	ParallelProcessor mp(taskPool, std::function(BenchUtils::CubedLambda));

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_Function)
{
    auto f = Function<void(std::size_t, int&)>(BenchUtils::CubedLambda);
	ParallelProcessor mp(taskPool, f);

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_Moved_Function)
{
	ParallelProcessor mp(taskPool, Function<void(std::size_t, int&)>(BenchUtils::CubedLambda));

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}

TEST_F(ParallelProcessorTest, With_Function_Pointer)
{
	ParallelProcessor mp(taskPool, &BenchUtils::Cubed);

	mp.StartProcessing(data.data(), NFUNC_CALLS, NFUNC_CALLS / (THREAD_COUNT_MAX * 10u), THREAD_COUNT_MAX);
	mp.WaitForCompetion();

    ASSERT_TRUE(ValidateData());
}