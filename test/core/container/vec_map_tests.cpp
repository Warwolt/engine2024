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
	EXPECT_EQ(rhs, lhs);
}

TEST(VecMapTests, CopyConstructor) {
	core::VecMap<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	core::VecMap<std::string, int> rhs = lhs;

	EXPECT_EQ(lhs, rhs);
	EXPECT_EQ(rhs, lhs);
}

TEST(VecMapTests, CopyAssignment) {
	core::VecMap<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	core::VecMap<std::string, int> rhs;
	rhs = lhs;

	EXPECT_EQ(lhs, rhs);
	EXPECT_EQ(rhs, lhs);
}

TEST(VecMapTests, BracketOperator_NonExistingKey_DefaultConstructsElement) {
	core::VecMap<std::string, int> vec_map;

	int value = vec_map["key"];

	core::VecMap<std::string, int> expected = { { "key", 0 } };
	EXPECT_EQ(value, 0);
	EXPECT_EQ(vec_map, expected);
}

TEST(VecMapTests, BracketOperator_ExistingKey_ReturnsThatValue) {
	core::VecMap<std::string, int> vec_map = { { "first", 1 } };

	int value = vec_map["first"];

	EXPECT_EQ(value, 1);
}

TEST(VecMapTests, AtOperator_NonExistingKey_ThrowsException) {
	core::VecMap<std::string, int> vec_map;
	EXPECT_THROW(vec_map.at("key"), std::out_of_range);

	const core::VecMap<std::string, int> const_vec_map;
	EXPECT_THROW(const_vec_map.at("key"), std::out_of_range);
}

TEST(VecMapTests, AtOperator_ExistingKey_ReturnsThatValue) {
	core::VecMap<std::string, int> vec_map = { { "key", 1234 } };
	EXPECT_EQ(vec_map.at("key"), 1234);

	const core::VecMap<std::string, int> const_vec_map = { { "key", 1234 } };
	EXPECT_EQ(const_vec_map.at("key"), 1234);
}
