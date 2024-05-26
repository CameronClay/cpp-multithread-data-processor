//these utility functions are not necessary with C++ 20 can isntead use std::osyncstream(std::cout)

#ifndef THREAD_SAFE_COUT
#define THREAD_SAFE_COUT

#include <iostream>
#include <mutex>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration;

inline std::mutex outMutex{}; //inline variable new in C++ 17

//output will be interlaced unless wrapping cout in mutex
inline void ThreadSafeCout(const std::string& str) {
	std::lock_guard lock{ outMutex };
	std::cout << str;
}

inline void ThreadSafeCerr(const std::string& str) {
	std::lock_guard lock{ outMutex };
	std::cout << str;
}

inline void ThreadSafeDbg(const std::string& str) {
	std::lock_guard lock{ outMutex };
	//const auto time = high_resolution_clock::now().time_since_epoch().count();
	//std::cout << "[" << time << "] " << str;
	std::cout << str;
}

#endif