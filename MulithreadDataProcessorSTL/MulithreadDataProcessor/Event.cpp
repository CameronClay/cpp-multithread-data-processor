#include "Event.h"

IEvent::IEvent() {}

void IEvent::NotifyOne() {
	std::lock_guard<std::mutex> lock{ mutex };
	isSet = true;

	cv.notify_one();
}

void IEvent::NotifyAll() {
	std::lock_guard<std::mutex> lock{ mutex };
	isSet = true;

	cv.notify_all();
}

void IEvent::Wait() {
	std::unique_lock<std::mutex> lock{ mutex };
	if (isSet)
		return;

	cv.wait(lock, [this] { return isSet; });
}

bool IEvent::WaitFor(uint32_t timeMilli) {
	std::unique_lock<std::mutex> lock{ mutex };
	if (isSet)
		return true;

	return cv.wait_for(lock, std::chrono::milliseconds(timeMilli), [this] { return isSet; });
}


bool IEvent::IsSet() {
	std::lock_guard<std::mutex> lock{ mutex };
	return isSet;
}

Event::Event() {
	Reset();
}

void Event::Reset() {
	std::lock_guard<std::mutex> lock{ mutex };
	isSet = false;
}

EventMPSC::EventMPSC(std::size_t notifyCount) {
	Reset(notifyCount);
}

void EventMPSC::NotifyOne() {
	std::lock_guard<std::mutex> lock{ mutex };
	isSet = isSet || (--this->notifyCounter == 0u);
	cv.notify_one();
}

void EventMPSC::NotifyAll() {
	std::lock_guard<std::mutex> lock{ mutex };
	isSet = isSet || (--this->notifyCounter == 0u);
	cv.notify_all();
}

void EventMPSC::Reset(std::size_t notifyCount) {
	std::lock_guard<std::mutex> lock{ mutex };
	this->notifyCounter = notifyCount;
	isSet = false;
}

void EventMPSC::Wait() {
	std::unique_lock<std::mutex> lock{ mutex };
	if (isSet)
		return;

	cv.wait(lock, [this] { return isSet; });
}

bool EventMPSC::WaitFor(uint32_t timeMilli) {
	std::unique_lock<std::mutex> lock{ mutex };
	if (isSet)
		return true;

	return cv.wait_for(lock, std::chrono::milliseconds(timeMilli), [this] { return isSet; });
}