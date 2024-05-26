#ifndef EVENT_H
#define EVENT_H

#include <mutex>
#include <condition_variable>

class IEvent
{
public:
	IEvent();

	bool IsSet();

	virtual void NotifyOne();
	virtual void NotifyAll();
	virtual void Wait();
	virtual bool WaitFor(uint32_t timeMilli);

protected:
	std::mutex mutex;
	std::condition_variable cv;
	bool isSet;
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
	virtual bool WaitFor(uint32_t timeMilli) override;
private:
	std::size_t notifyCounter;
};

#endif