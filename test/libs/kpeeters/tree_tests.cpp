#include <gtest/gtest.h>

#include <kpeeters/tree.hpp>

TEST(TreeTests, DefaultTree_IsEmpty) {
	kpeeters::tree<int> tree;

	EXPECT_TRUE(tree.empty());
	EXPECT_EQ(tree.size(), 0);
	EXPECT_EQ(tree.max_depth(), 0);
}

TEST(TreeTests, SingleNodeTree_NonEmpty) {
	kpeeters::tree<int> tree;

	// 1
	tree.insert(tree.begin(), 1);

	EXPECT_FALSE(tree.empty());
	EXPECT_EQ(tree.size(), 1);
	EXPECT_EQ(tree.max_depth(), 1);
}

TEST(TreeTests, NodesIteratedDepthFirst) {
	kpeeters::tree<int> tree;

	//      1
	//    /  \
	//   2    3
	// / | \
	// 4 5 6
	auto root = tree.insert(tree.begin(), 1);
	auto left_child = tree.append_child(root, 2);
	tree.append_child(root, 3);
	tree.append_child(left_child, 4);
	tree.append_child(left_child, 5);
	tree.append_child(left_child, 6);

	std::vector<int> nodes;
	for (int node : tree) {
		nodes.push_back(node);
	}

	const std::vector<int> expected = { 1, 2, 4, 5, 6, 3 };
	EXPECT_EQ(nodes, expected);
}

TEST(TreeTests, EraseSingleNode) {
	kpeeters::tree<int> tree;

	//      1
	//    /  \
	//   2    3
	// / | \
	// 4 5 6
	auto root = tree.insert(tree.begin(), 1);
	auto left_child = tree.append_child(root, 2);
	tree.append_child(root, 3);
	tree.append_child(left_child, 4);
	auto node_5 = tree.append_child(left_child, 5);
	tree.append_child(left_child, 6);

	//      1
	//    /  \
	//   2    3
	// /   \
	// 4   6
	tree.erase(node_5);

	std::vector<int> nodes;
	for (int node : tree) {
		nodes.push_back(node);
	}

	const std::vector<int> expected = { 1, 2, 4, 6, 3 };
	EXPECT_EQ(nodes, expected);
}

TEST(TreeTests, EraseSubTree) {
	kpeeters::tree<int> tree;

	//      1
	//    /  \
	//   2    3
	// / | \
	// 4 5 6
	auto root = tree.insert(tree.begin(), 1);
	auto left_child = tree.append_child(root, 2);
	tree.append_child(root, 3);
	tree.append_child(left_child, 4);
	tree.append_child(left_child, 5);
	tree.append_child(left_child, 6);

	//  1
	//   \
	//    3
	tree.erase(left_child);

	std::vector<int> nodes;
	for (int node : tree) {
		nodes.push_back(node);
	}

	const std::vector<int> expected = { 1, 3 };
	EXPECT_EQ(nodes, expected);
}
