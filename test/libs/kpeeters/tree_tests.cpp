#include <gtest/gtest.h>

#include <kpeeters/tree.hpp>

TEST(TreeTests, DefaultTree_IsEmpty) {
	kpeeters::tree<int> tree;
	EXPECT_TRUE(tree.empty());
	EXPECT_EQ(tree.size(), 0);
}
