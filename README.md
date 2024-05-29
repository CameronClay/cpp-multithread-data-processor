# Multithreaded Data Processor
This project explores many of the built-in thread synchronization objects and utilities in C++ 17/C++ 20, including std::thread, std::condition_variable, std::mutex, std::atomic, std::atomic::wait, std::async, std::future, std::for_each with execution_policy, and more. This project also contains a custom ParallelProcessor built with STL synchronization objects.

## Benchmarks
Multiple execution methods including sequential, parallel, std::for_each, std::async are bnechmarked to see which is the fastest and to demonstrate they are all working correctly.

### Unit Testing and Systems Testing
- Unit testing and systems testing is done using Google Test.
- The main program also performs basic systems testing inbetween the benchmarking

### Tooling
- C++ 20 (with C++ 20 modules)
- Google Test
- CMake
- vcpkg
- Visual Studio Code
- Visual Studio

### Discussion
My ParallelProcessor implementation seems to be around 30% faster than std::for_each with parallel execution in the benchmarks. This performance difference could be due to compiler optimizations around inlining functions.

```
Avgs of 100 trials of 4000000 loops | single threaded time = 12.594 | std::for_each time = 1.383 
--------------------------------------------------
Avgs of 100 trials of 4000000 loops | multithreaded (1 threads) time = 8.027
Avgs of 100 trials of 4000000 loops | multithreaded (2 threads) time = 4.143
Avgs of 100 trials of 4000000 loops | multithreaded (3 threads) time = 2.803
Avgs of 100 trials of 4000000 loops | multithreaded (4 threads) time = 2.137
Avgs of 100 trials of 4000000 loops | multithreaded (5 threads) time = 1.795
Avgs of 100 trials of 4000000 loops | multithreaded (6 threads) time = 1.511
Avgs of 100 trials of 4000000 loops | multithreaded (7 threads) time = 1.354
Avgs of 100 trials of 4000000 loops | multithreaded (8 threads) time = 1.263
Avgs of 100 trials of 4000000 loops | multithreaded (9 threads) time = 1.192
Avgs of 100 trials of 4000000 loops | multithreaded (10 threads) time = 1.172
Avgs of 100 trials of 4000000 loops | multithreaded (11 threads) time = 1.096
Avgs of 100 trials of 4000000 loops | multithreaded (12 threads) time = 1.109
Avgs of 100 trials of 4000000 loops | multithreaded (13 threads) time = 1.051
Avgs of 100 trials of 4000000 loops | multithreaded (14 threads) time = 1.015
Avgs of 100 trials of 4000000 loops | multithreaded (15 threads) time = 1.027
Avgs of 100 trials of 4000000 loops | multithreaded (16 threads) time = 0.984
--------------------------------------------------
```
