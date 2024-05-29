import <gmock/gmock-matchers.h>;
// import <gtest/gtest.h>;
import <future>;
import <chrono>;
import <memory>;
import <syncstream>;
import <iostream>;
import Queue;

TEST(QueueTest, EnqueueDequeue_ImmediateReturn)
{
    Queue<int> q;

    q.Enqueue(1);

    EXPECT_EQ(1, *q.Dequeue());
}

TEST(QueueTest, EnqueueDequeue_WaitToReturn)
{
    Queue<int> q;

    auto enqueueTask = std::async(std::launch::async,
        [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            q.Enqueue(1);
        }
    );

    EXPECT_EQ(1, *q.Dequeue());
    enqueueTask.get();
}

TEST(QueueTest, PendingDequeus_Aborted)
{
    Queue<int> q;
    auto destroyTask = std::async(std::launch::async,
        [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            q.~Queue();
        }
    );

    EXPECT_FALSE(q.Dequeue());
    destroyTask.get();
}

TEST(QueueTest, EnqueueTryDequeue_Empty)
{
    Queue<int> q;

    q.Enqueue(1);

    int res = -1;
    EXPECT_TRUE(q.TryDequeue(res));
    EXPECT_EQ(res, 1);
}

TEST(QueueTest, EnqueueTryDequeue_NotEmpty)
{
    Queue<int> q;

    int res = -1;
    EXPECT_FALSE(q.TryDequeue(res));
    EXPECT_EQ(res, -1);
}

TEST(QueueTest, ClearQueue_ClearsQueue)
{
    Queue<int> q;

    q.Enqueue(1);

    q.Clear();

    int res = -1;
    EXPECT_FALSE(q.TryDequeue(res));
    EXPECT_EQ(res, -1);
    EXPECT_TRUE(q.Empty());
}

TEST(QueueTest, Empty_WhenEmpty)
{
    Queue<int> q;

    EXPECT_TRUE(q.Empty());
}


TEST(QueueTest, Empty_WhenNotEmpty)
{
    Queue<int> q;

    q.Enqueue(1);

    EXPECT_FALSE(q.Empty());
}