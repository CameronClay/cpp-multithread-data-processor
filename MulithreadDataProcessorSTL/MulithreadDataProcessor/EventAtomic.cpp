#include "EventAtomic.h"

IEvent::IEvent() {}

bool IEvent::IsSet() const {
	return isSet.load(std::memory_order_acquire);
}

Event::Event() {
	Reset();
}

void IEvent::NotifyOne() {
	isSet.store(true, std::memory_order_release);
	isSet.notify_one();
}

void IEvent::NotifyAll() {
	isSet.store(true, std::memory_order_release);
	isSet.notify_all();
}

void IEvent::Wait() {
	isSet.wait(false, std::memory_order_acquire);
}


void Event::Reset() {
	isSet.store(false, std::memory_order_release);
}


EventMPSC::EventMPSC(std::size_t notifyCount) {
	Reset(notifyCount);
}

void EventMPSC::NotifyOne() {
	if (--notifyCounter == 0u) {
		IEvent::NotifyOne();
	}
}

void EventMPSC::NotifyAll() {
	if (--notifyCounter == 0u) {
		IEvent::NotifyAll();
	}
}

void EventMPSC::Reset(std::size_t notifyCount) {
	notifyCounter.store(notifyCount, std::memory_order_release);
	isSet.store(false, std::memory_order_release);
}

void EventMPSC::Wait() {
	if (notifyCounter.load(std::memory_order_acquire) > 0) {
		IEvent::Wait();
	}
}


EventCountdown::EventCountdown() {
	Reset();
}

bool EventCountdown::IsSet() const {
	return ev.IsSet();
}

void EventCountdown::Reset() {
	notifyCounter.store(0u, std::memory_order_release);
	ev.Reset();
}

std::size_t EventCountdown::operator--() {
	const std::size_t res = --notifyCounter;

	if (res == 0u) {
		ev.NotifyAll();
	}

	return res;
}
std::size_t EventCountdown::operator++() {
	return ++notifyCounter;
}

void EventCountdown::Wait() {
	if (notifyCounter.load(std::memory_order_acquire) > 0) {
		ev.Wait();
	}
}