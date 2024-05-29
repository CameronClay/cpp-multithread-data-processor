export module EventAtomic;

import <atomic>;

//uses new C++ 20 atomic<T>::wait in place of condition_variable + mutex
export class IEvent
{
public:
	IEvent() = default;

	bool IsSet() const {
		return isSet.load(std::memory_order_acquire);
	}
	virtual void NotifyOne() {
		isSet.store(true, std::memory_order_release);
		isSet.notify_one();
	}
	virtual void NotifyAll() {
		isSet.store(true, std::memory_order_release);
		isSet.notify_all();
	}
	virtual void Wait() {
		isSet.wait(false, std::memory_order_acquire);
	}
protected:
    std::atomic<bool> isSet;
};

//generic event
export class Event : public IEvent
{
public:
	Event() {
		Reset();
	}

	void Reset() {
		isSet.store(false, std::memory_order_release);
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
		notifyCounter.store(notifyCount, std::memory_order_release);
		isSet.store(false, std::memory_order_release);
	}
	virtual void NotifyOne() override {
		if (--notifyCounter == 0u) {
			IEvent::NotifyOne();
		}
	}
	virtual void NotifyAll() override {
		if (--notifyCounter == 0u) {
			IEvent::NotifyAll();
		}
	}
	virtual void Wait() override {
		if (notifyCounter.load(std::memory_order_acquire) > 0) {
			IEvent::Wait();
		}
	}

protected:
	std::atomic<std::size_t> notifyCounter;
};

//event is fired when pre-decrement operator is called and value is 0
export class EventCountdown
{
public:
	EventCountdown() {
		Reset();
	}

	bool IsSet() const {
		return ev.IsSet();
	}
	void Reset() {
		notifyCounter.store(0u, std::memory_order_release);
		ev.Reset();
	}
	//predecrement operator
	std::size_t operator--() {
		const std::size_t res = --notifyCounter;

		if (res == 0u) {
			ev.NotifyAll();
		}

		return res;
	}
	//preincrement operator
	std::size_t operator++() {
		return ++notifyCounter;
	}
	virtual void Wait() {
		if (notifyCounter.load(std::memory_order_acquire) > 0) {
			ev.Wait();
		}
	}

protected:
	Event ev;
	std::atomic<std::size_t> notifyCounter;
};