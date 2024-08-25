#include <gtest/gtest.h>

#include <kpeeters/tree.hpp>

TEST(TreeTests, DefaultTree_IsEmpty) {
	kpeeters::tree<int> tree;

	EXPECT_TRUE(tree.empty());
	EXPECT_EQ(tree.size(), 0);
	EXPECT_EQ(tree.max_depth(), 0);
}

TEST(TreeTests, SingleNodeTree) {
	kpeeters::tree<int> tree;

	tree.insert(tree.begin(), 10);

	EXPECT_FALSE(tree.empty());
	EXPECT_EQ(tree.size(), 1);
	EXPECT_EQ(tree.max_depth(), 1);
}
