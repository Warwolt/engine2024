#include <gtest/gtest.h>

#include <core/container/vec_map.h>

TEST(VecMapTests, DefaultConstructed_IsEmpty) {
	core::VecMap<std::string, int> vec_map;

	EXPECT_EQ(vec_map.size(), 0);
	EXPECT_TRUE(vec_map.empty());
}

TEST(VecMapTests, ConstructedWithInitializerList_IsNonEmpty) {
	core::VecMap<std::string, int> vec_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};

	EXPECT_EQ(vec_map.size(), 3);
	EXPECT_FALSE(vec_map.empty());
}

TEST(VecMapTests, Equality_DifferentSizes_NotEqual) {
	core::VecMap<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
	};
	core::VecMap<std::string, int> rhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};

	EXPECT_NE(lhs, rhs);
}

TEST(VecMapTests, Equality_EqualElements_DifferentOrder_AreEqual) {
	core::VecMap<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	core::VecMap<std::string, int> rhs = {
		{ "second", 2 },
		{ "third", 3 },
		{ "first", 1 },
	};

	EXPECT_EQ(lhs, rhs);
}
