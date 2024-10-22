#include <gtest/gtest.h>

#include <core/container/vector_map.h>

TEST(VectorMapTests, DefaultConstructed_IsEmpty) {
	core::vector_map<std::string, int> vector_map;

	EXPECT_EQ(vector_map.size(), 0);
	EXPECT_TRUE(vector_map.empty());
}

TEST(VectorMapTests, ConstructedWithInitializerList_IsNonEmpty) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};

	EXPECT_EQ(vector_map.size(), 3);
	EXPECT_FALSE(vector_map.empty());
}

TEST(VectorMapTests, Equality_DifferentSizes_NotEqual) {
	core::vector_map<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
	};
	core::vector_map<std::string, int> rhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};

	EXPECT_NE(lhs, rhs);
}

TEST(VectorMapTests, Equality_EqualElements_DifferentOrder_AreEqual) {
	core::vector_map<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	core::vector_map<std::string, int> rhs = {
		{ "second", 2 },
		{ "third", 3 },
		{ "first", 1 },
	};

	EXPECT_EQ(lhs, rhs);
	EXPECT_EQ(rhs, lhs);
}

TEST(VectorMapTests, CopyConstructor) {
	core::vector_map<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	core::vector_map<std::string, int> rhs = lhs;

	EXPECT_EQ(lhs, rhs);
	EXPECT_EQ(rhs, lhs);
}

TEST(VectorMapTests, CopyAssignment) {
	core::vector_map<std::string, int> lhs = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	core::vector_map<std::string, int> rhs;
	rhs = lhs;

	EXPECT_EQ(lhs, rhs);
	EXPECT_EQ(rhs, lhs);
}

TEST(VectorMapTests, BracketOperator_NonExistingKey_DefaultConstructsElement) {
	core::vector_map<std::string, int> vector_map;

	int value = vector_map["key"];

	core::vector_map<std::string, int> expected = { { "key", 0 } };
	EXPECT_EQ(value, 0);
	EXPECT_EQ(vector_map, expected);
}

TEST(VectorMapTests, BracketOperator_ExistingKey_ReturnsThatValue) {
	core::vector_map<std::string, int> vector_map = { { "first", 1 } };

	int value = vector_map["first"];

	EXPECT_EQ(value, 1);
}

TEST(VectorMapTests, AtOperator_NonExistingKey_ThrowsException) {
	core::vector_map<std::string, int> vector_map;
	EXPECT_THROW(vector_map.at("key"), std::out_of_range);

	const core::vector_map<std::string, int> const_vec_map;
	EXPECT_THROW(const_vec_map.at("key"), std::out_of_range);
}

TEST(VectorMapTests, AtOperator_ExistingKey_ReturnsThatValue) {
	core::vector_map<std::string, int> vector_map = { { "key", 1234 } };
	EXPECT_EQ(vector_map.at("key"), 1234);

	const core::vector_map<std::string, int> const_vec_map = { { "key", 1234 } };
	EXPECT_EQ(const_vec_map.at("key"), 1234);
}

TEST(VectorMapTests, AccessUnderlyingData) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};

	std::vector<std::pair<std::string, int>> values = vector_map.data();

	const std::vector<std::pair<std::string, int>> expected = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 3 },
	};
	EXPECT_EQ(values, expected);
}

TEST(VectorMapTests, ConstructedWithInitializerList_DuplicateEntriesIgnored) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "second", 22 },
	};
	std::unordered_map<std::string, int> unordered_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "second", 22 },
	};

	EXPECT_EQ(vector_map["first"], 1);
	EXPECT_EQ(vector_map["second"], 2);
	EXPECT_EQ(unordered_map["first"], 1);
	EXPECT_EQ(unordered_map["second"], 2);
	EXPECT_EQ(vector_map.size(), 2);
}

TEST(VectorMapTests, Iterator_IteratesInOrderOfInsertion) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 33 },
	};
	const core::vector_map<std::string, int> const_vec_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 33 },
	};

	std::vector<int> values;
	for (const auto& [key, value] : vector_map) {
		values.push_back(value);
	}
	std::vector<int> const_values;
	for (const auto& [key, value] : const_vec_map) {
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

TEST(VectorMapTests, Insert_NewKey_AddsElementToEnd) {
	core::vector_map<std::string, int> vector_map {
		{ "first", 1 },
	};

	const auto& [iterator, was_inserted] = vector_map.insert({ "second", 22 });
	std::vector<std::pair<std::string, int>> values;
	for (auto& [key, value] : vector_map) {
		values.push_back({ key, value });
	}

	const std::vector<std::pair<std::string, int>> expected = {
		{ "first", 1 },
		{ "second", 22 },
	};
	EXPECT_TRUE(was_inserted);
	ASSERT_NE(iterator, vector_map.end());
	EXPECT_EQ(iterator->second, 22);
	EXPECT_EQ(vector_map["first"], 1);
	EXPECT_EQ(vector_map["second"], 22);
	EXPECT_EQ(values, expected);
}

TEST(VectorMapTests, Insert_ExistingKey_OverwritesValue) {
	core::vector_map<std::string, int> vector_map {
		{ "first", 1 },
	};

	const auto& [iterator, was_inserted] = vector_map.insert({ "first", 12 });
	std::vector<int> values;
	for (auto& [key, value] : vector_map) {
		values.push_back(value);
	}

	const std::vector<int> expected = { 12 };
	EXPECT_FALSE(was_inserted);
	ASSERT_NE(iterator, vector_map.end());
	EXPECT_EQ(iterator->second, 12);
	EXPECT_EQ(vector_map["first"], 12);
	EXPECT_EQ(values, expected);
}

TEST(VectorMapTests, Clear_RemovesAllElements) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 1 },
		{ "second", 2 },
		{ "third", 33 },
	};

	vector_map.clear();
	std::vector<int> values;
	for (auto& [key, value] : vector_map) {
		values.push_back(value);
	}

	EXPECT_TRUE(vector_map.empty());
	EXPECT_EQ(vector_map.size(), 0);
	EXPECT_TRUE(values.empty());
}

TEST(VectorMapTests, Erase_WithMatchingKey_RemovesElement) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 10 },
		{ "second", 20 },
		{ "third", 30 },
	};

	vector_map.erase("second");

	const core::vector_map<std::string, int> expected = {
		{ "first", 10 },
		{ "third", 30 },
	};
	EXPECT_EQ(vector_map, expected);
}

TEST(VectorMapTests, Erase_ReturnsIteratorToNextElement) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 10 },
		{ "second", 20 },
		{ "third", 30 },
	};

	auto it = vector_map.erase("second");

	ASSERT_NE(it, vector_map.end());
	EXPECT_EQ(it->second, 30);
}

TEST(VectorMapTests, Erase_LastElemenet_RemovesElement) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 10 },
		{ "second", 20 },
		{ "third", 30 },
	};

	vector_map.erase("third");

	const core::vector_map<std::string, int> expected = {
		{ "first", 10 },
		{ "second", 20 },
	};
	EXPECT_EQ(vector_map, expected);
}

TEST(VectorMapTests, Erase_NonExistingKey_DoesNothing) {
	core::vector_map<std::string, int> vector_map = {
		{ "first", 10 },
		{ "second", 20 },
		{ "third", 30 },
	};

	vector_map.erase("non-existing");

	const core::vector_map<std::string, int> expected = {
		{ "first", 10 },
		{ "second", 20 },
		{ "third", 30 },
	};
	EXPECT_EQ(vector_map, expected);
	EXPECT_EQ(vector_map.size(), 3);
	EXPECT_FALSE(vector_map.empty());
}
