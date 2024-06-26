export module ScopedThread;

import <thread>;
import <exception>;
import <stdexcept>;

//could also use std::jthread (C++ 20) in place of this
export class ScopedThread
{
public:
    explicit ScopedThread(std::thread&& _t) 
        : 
        t(std::move(_t))
    {
        if (!this->t.joinable())  {
            throw std::logic_error("No thread");
        }
    }

    ~ScopedThread() {
        Join();
    }

    ScopedThread(ScopedThread&& _st) = default;
    ScopedThread& operator=(ScopedThread&& _t) = default;

    void Join() {
        if (t.joinable()) {
            t.join();
        }
    }
private:
    std::thread t;
};