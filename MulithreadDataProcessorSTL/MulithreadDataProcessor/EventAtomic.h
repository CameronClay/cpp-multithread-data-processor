#ifndef EVENT_ATOMIC_H
#define EVENT_ATOMIC_H

#include <atomic>

//uses new C++ 20 atomic<T>::wait in place of condition_variable + mutex
class IEvent
{
public:
	IEvent();
	bool IsSet() const;

	virtual void NotifyOne();
	virtual void NotifyAll();
	virtual void Wait();
protected:
    std::atomic<bool> isSet;
};

//generic event
class Event : public IEvent
{
public:
	Event();

	void Reset();
};

//multiple producer single consumer
class EventMPSC : public IEvent
{
public:
	EventMPSC(std::size_t notifyCount);

	void Reset(std::size_t notifyCount);

	virtual void NotifyOne() override;
	virtual void NotifyAll() override;
	virtual void Wait() override;
protected:
	std::atomic<std::size_t> notifyCounter;
};

//event is fired when pre-decrement operator is called and value is 0
class EventCountdown
{
public:
	EventCountdown();

	bool IsSet() const;
	void Reset();

	//preincrement operator
	std::size_t operator--();

	//predecrement operator
	std::size_t operator++();

	virtual void Wait();

protected:
	Event ev;
	std::atomic<std::size_t> notifyCounter;
};


#endif