find_package(GTest CONFIG REQUIRED)

set(TESTS 
    Tests
)

add_executable(Tests main.ixx QueueTests.ixx ParallelProcessorTests.ixx)

foreach(target IN LISTS TESTS)
    target_link_libraries(${target} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

    target_link_libraries(${target} PRIVATE lib)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/lib)

    target_link_libraries(${target} PRIVATE bench_utils)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/bench_utils)

    add_test(${target} ${target})
endforeach()