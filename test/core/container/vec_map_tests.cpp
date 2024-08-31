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

TEST(VecMapTests, ConstructedWithInitializerList_DuplicateEntriesIgnored) {
	core::VecMap<std::string, int> vec_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "second", 22 },
	};
	std::unordered_map<std::string, int> unordered_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "second", 22 },
	};

	EXPECT_EQ(vec_map["first"], 1);
	EXPECT_EQ(vec_map["second"], 2);
	EXPECT_EQ(unordered_map["first"], 1);
	EXPECT_EQ(unordered_map["second"], 2);
	EXPECT_EQ(vec_map.size(), 2);
}

TEST(VecMapTests, Iterator_IteratesInOrderOfInsertion) {
	core::VecMap<std::string, int> vec_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 33 },
	};
	const core::VecMap<std::string, int> const_vec_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 33 },
	};

	std::vector<int> values;
	for (int value : vec_map) {
		values.push_back(value);
	}
	std::vector<int> const_values;
	for (int value : const_vec_map) {
		const_values.push_back(value);
	}

	ASSERT_EQ(values.size(), 3);
	EXPECT_EQ(values[0], 1);
	EXPECT_EQ(values[1], 2);
	EXPECT_EQ(values[2], 33);

	ASSERT_EQ(const_values.size(), 3);
	EXPECT_EQ(const_values[0], 1);
	EXPECT_EQ(const_values[1], 2);
	EXPECT_EQ(const_values[2], 33);
}

TEST(VecMapTests, Insert_NewKey_AddsElementToEnd) {
	core::VecMap<std::string, int> vec_map {
		{ "first", 1 },
	};

	const auto& [iterator, was_inserted] = vec_map.insert({ "second", 22 });
	std::vector<int> values;
	for (int value : vec_map) {
		values.push_back(value);
	}

	const std::vector<int> expected = { 1, 22 };
	EXPECT_TRUE(was_inserted);
	ASSERT_NE(iterator, vec_map.end());
	EXPECT_EQ(*iterator, 22);
	EXPECT_EQ(vec_map["first"], 1);
	EXPECT_EQ(vec_map["second"], 22);
	EXPECT_EQ(values, expected);
}

TEST(VecMapTests, Insert_ExistingKey_OverwritesValue) {
	core::VecMap<std::string, int> vec_map {
		{ "first", 1 },
	};

	const auto& [iterator, was_inserted] = vec_map.insert({ "first", 12 });
	std::vector<int> values;
	for (int value : vec_map) {
		values.push_back(value);
	}

	const std::vector<int> expected = { 12 };
	EXPECT_FALSE(was_inserted);
	ASSERT_NE(iterator, vec_map.end());
	EXPECT_EQ(*iterator, 12);
	EXPECT_EQ(vec_map["first"], 12);
	EXPECT_EQ(values, expected);
}

TEST(VecMapTests, Clear_RemovesAllElements) {
	core::VecMap<std::string, int> vec_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 33 },
	};

	vec_map.clear();
	std::vector<int> values;
	for (int value : vec_map) {
		values.push_back(value);
	}

	EXPECT_TRUE(vec_map.empty());
	EXPECT_EQ(vec_map.size(), 0);
	EXPECT_TRUE(values.empty());
}

// erase returns iterator to next element after erased element
TEST(VecMapTests, Erase_WithMatchingKey_RemovesElement) {
	core::VecMap<std::string, int> vec_map = {
		{ "first", 10 },
		{ "second", 20 },
		{ "third", 30 },
	};

	vec_map.erase("second");

	const core::VecMap<std::string, int> expected = {
		{ "first", 10 },
		{ "third", 30 },
	};
	EXPECT_EQ(vec_map, expected);
}

// erase with non-existing key does nothing

// erase with std::remove_if
