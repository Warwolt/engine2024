#include <gtest/gtest.h>

TEST(dummy_test, hello_world) {
	EXPECT_EQ(2 + 2, 4) << "Hello world!";
}
