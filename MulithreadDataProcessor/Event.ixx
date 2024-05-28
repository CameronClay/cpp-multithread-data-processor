export module Event;

import <mutex>;
import <condition_variable>;

export class IEvent
{
public:
	IEvent() = default;

	bool IsSet() {
		std::lock_guard<std::mutex> lock{ mutex };
		return isSet;
	}
	virtual void NotifyOne() {
		std::lock_guard<std::mutex> lock{ mutex };
		isSet = true;

		cv.notify_one();
	}
	virtual void NotifyAll() {
		std::lock_guard<std::mutex> lock{ mutex };
		isSet = true;

		cv.notify_all();
	}
	virtual void Wait() {
		std::unique_lock<std::mutex> lock{ mutex };
		if (isSet)
			return;

		cv.wait(lock, [this] { return isSet; });
	}
	virtual bool WaitFor(uint32_t timeMilli) {
		std::unique_lock<std::mutex> lock{ mutex };
		if (isSet)
			return true;

		return cv.wait_for(lock, std::chrono::milliseconds(timeMilli), [this] { return isSet; });
	}

protected:
	std::mutex mutex;
	std::condition_variable cv;
	bool isSet;
};

//generic event
export class Event : public IEvent
{
public:
	Event() {
		Reset();
	}

	void Reset() {
		std::lock_guard<std::mutex> lock{ mutex };
		isSet = false;
	}
};

//multiple producer single consumer
export class EventMPSC : public IEvent
{
public:
	EventMPSC(std::size_t notifyCount) {
		Reset(notifyCount);
	}

	void Reset(std::size_t notifyCount) {
		std::lock_guard<std::mutex> lock{ mutex };
		this->notifyCounter = notifyCount;
		isSet = false;
	}
	virtual void NotifyOne() override {
		std::lock_guard<std::mutex> lock{ mutex };
		isSet = isSet || (--this->notifyCounter == 0u);
		cv.notify_one();
	}
	virtual void NotifyAll() override {
		std::lock_guard<std::mutex> lock{ mutex };
		isSet = isSet || (--this->notifyCounter == 0u);
		cv.notify_all();
	}
	virtual void Wait() override {
		std::unique_lock<std::mutex> lock{ mutex };
		if (isSet)
			return;

		cv.wait(lock, [this] { return isSet; });
	}
	virtual bool WaitFor(uint32_t timeMilli) override {
		std::unique_lock<std::mutex> lock{ mutex };
		if (isSet)
			return true;

		return cv.wait_for(lock, std::chrono::milliseconds(timeMilli), [this] { return isSet; });
	}
private:
	std::size_t notifyCounter;
};