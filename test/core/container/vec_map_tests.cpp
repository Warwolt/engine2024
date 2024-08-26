#include <gtest/gtest.h>

#include <core/container/vec_map.h>

TEST(VecMapTests, InitiallyEmpty) {
	core::VecMap<std::string, int> vec_map;

	EXPECT_EQ(vec_map.size(), 0);
	EXPECT_TRUE(vec_map.empty());
}
