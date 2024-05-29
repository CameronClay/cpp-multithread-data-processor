// import test_mod;
// module;

// import <gtest/gtest.h>;

#include <gtest/gtest.h>

// module main;

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}