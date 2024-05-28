export module Queue;

import <mutex>;
import <condition_variable>;
import <queue>;
import <optional>;
import <atomic>;
import EventAtomic;

export template<typename T>
class Queue
{
public:
	Queue() {
		interruptQueue = false;
	}

	~Queue() {
		InterruptQueue();
		dequeuesIP.Wait();
	}

	//Enqueues item and notifies condition variable a new item has been added to the queue (if was previously empty)
	void Enqueue(const T& t) {
		std::unique_lock<std::mutex> lock{ mutex };
		queue.push(t);

		cvQueue.notify_one();
	}

	//Enqueues item (constructs item in place) and notifies condition variable a new item has been added to the queue (if was previously empty)
	template<typename... Args>
	void Enqueue(Args&&... args) {
		std::unique_lock<std::mutex> lock{ mutex };
		queue.emplace(std::forward<Args>(args)...);

		cvQueue.notify_one();
	}

	//Waits for item to become available to dequeue and then dequeue it
	std::optional<T> Dequeue() {
		++dequeuesIP;

		std::unique_lock<std::mutex> lock{ mutex };
		if (interruptQueue) {
			lock.unlock();
			--dequeuesIP;
			return {};
		}

		if (queue.empty()) {
			cvQueue.wait(lock, [this] { return !queue.empty() || interruptQueue; });
		}

		if (interruptQueue) {
			lock.unlock();
			--dequeuesIP;
			return {};
		}

		const T t = queue.front();
		queue.pop();

		lock.unlock();

		--dequeuesIP;
		return std::optional<T>(t);
	}

	//Attempts to dequeue an item without waiting, returns true if an item was dequeued
	bool TryDequeue(T& t) {
		std::unique_lock<std::mutex> lock{ mutex };
		if (queue.empty()) {
			return false;
		}

		t = queue.front();
		queue.pop();

		return true;
	}

	void Clear() {
		std::unique_lock<std::mutex> lock{ mutex };
		while (!queue.empty()) {
			queue.pop();
		}
	}

	bool Empty() {
		std::unique_lock<std::mutex> lock{ mutex };
		return queue.empty();
	}

	size_t Size() {
		std::unique_lock<std::mutex> lock{ mutex };
		return queue.size();
	}

	//Interrupt all blocking Dequeue operations and cause them to return
	void InterruptQueue() {
		std::unique_lock<std::mutex> lock{ mutex };
		interruptQueue = true;
		cvQueue.notify_all();
	}

private:
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable cvQueue;
	bool interruptQueue;
	EventCountdown dequeuesIP;
};

//Thread safe Queue that notifies threads
//export template<typename T>
//class Queue
//{
//public:
//	Queue() {
//		interruptQueue = false;
//		dequeuesFinished = false;
//	}
//
//	~Queue() {
//		InterruptQueue();
//
//
//		if (dequeuesIP > 0u) {
//			dequeuesFinished.wait(false);
//
//			//std::unique_lock<std::mutex> lock{ mutex };
//			//cvQueue.wait(lock, [this] { return dequeuesIP == 0u; });
//		}
//
//		int a = 0;
//	}
//
//	//Enqueues item and notifies condition variable a new item has been added to the queue (if was previously empty)
//	void Enqueue(const T& t) {
//		std::unique_lock<std::mutex> lock{ mutex };
//		const bool empty = queue.empty();
//		queue.push(t);
//
//		if (empty) {
//			cvQueue.notify_one();
//		}
//	}
//
//	//Enqueues item (constructs item in place) and notifies condition variable a new item has been added to the queue (if was previously empty)
//	template<typename... Args>
//	void Enqueue(Args&&... args) {
//		std::unique_lock<std::mutex> lock{ mutex };
//		const bool empty = queue.empty();
//		queue.emplace(std::forward<Args>(args)...);
//
//		if (empty) {
//			cvQueue.notify_one();
//		}
//	}
//
//	//Waits for item to become available to dequeue and then dequeue it
//	std::optional<T> Dequeue() {
//		++dequeuesIP;
//
//		std::unique_lock<std::mutex> lock{ mutex };
//		if (interruptQueue) {
//			if (--dequeuesIP == 0u) {
//				dequeuesFinished = true;
//				dequeuesFinished.notify_all();
//			}
//
//			return {};
//		}
//
//		if (queue.empty()) {
//			cvQueue.wait(lock, [this] {return !queue.empty() || interruptQueue; });
//		}
//
//		if (interruptQueue) {
//			//lock.unlock();
//
//			if (--dequeuesIP == 0u) {
//				dequeuesFinished = true;
//				dequeuesFinished.notify_all();
//			}
//			//cvExit.notify_all();
//			return {};
//		}
//
//		const T t = queue.front();
//		queue.pop();
//
//		lock.unlock();
//
//		if (--dequeuesIP == 0u) {
//			dequeuesFinished = true;
//			dequeuesFinished.notify_all();
//		}
//		return std::optional<T>(t);
//	}
//
//	//Attemps to dequeue an item without waiting, returns true if an item was dequeued
//	bool TryDequeue(T& t) {
//		std::unique_lock<std::mutex> lock{ mutex };
//		if (queue.empty()) {
//			return false;
//		}
//
//		t = queue.front();
//		queue.pop();
//
//		return true;
//	}
//
//	void Clear() {
//		std::unique_lock<std::mutex> lock{ mutex };
//		while (!queue.empty()) {
//			queue.pop();
//		}
//	}
//
//	bool Empty() {
//		std::unique_lock<std::mutex> lock{ mutex };
//		return queue.empty();
//	}
//
//	size_t Size() {
//		std::unique_lock<std::mutex> lock{ mutex };
//		return queue.size();
//	}
//
//	void InterruptQueue() {
//		std::unique_lock<std::mutex> lock{ mutex };
//		interruptQueue = true;
//		cvQueue.notify_all();
//	}
//
//private:
//	std::queue<T> queue;
//	std::mutex mutex;
//	std::condition_variable cvQueue;
//	std::condition_variable cvExit;
//	bool interruptQueue;
//	std::atomic<std::size_t> dequeuesIP; 
//	std::atomic<bool> dequeuesFinished; //C++ 20 atomic<T>::wait_for similar to condition variables but more performant in general where applicable
//};