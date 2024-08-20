#include <gtest/gtest.h>

#include <core/container/ring_buffer.h>

#include <numeric>

TEST(RingBufferTests, InitiallyEmpty) {
	core::RingBuffer<int, 4> ring_buffer;

	EXPECT_EQ(ring_buffer.size(), 0);
	EXPECT_TRUE(ring_buffer.empty());
}

TEST(RingBufferTests, InitializerList) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2, 3, 4 };

	EXPECT_EQ(ring_buffer.size(), 4);
	EXPECT_EQ(ring_buffer.front(), 1);
	EXPECT_EQ(ring_buffer.back(), 4);
}

TEST(RingBufferTests, DifferentSizeNotEqual) {
	core::RingBuffer<int, 4> lhs = { 1, 2, 3 };
	core::RingBuffer<int, 4> rhs = { 1, 2, 3, 4 };
	EXPECT_NE(lhs, rhs);
}

TEST(RingBufferTests, PushOnceToEmpty_ReadBack) {
	core::RingBuffer<int, 4> ring_buffer;

	ring_buffer.push_back(10);

	EXPECT_EQ(ring_buffer.back(), 10);
	EXPECT_EQ(ring_buffer.size(), 1);
}

TEST(RingBufferTests, PushToBufferWithInitialElements) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2 };

	ring_buffer.push_back(3);
	ring_buffer.push_back(4);

	const core::RingBuffer<int, 4> expected = { 1, 2, 3, 4 };
	EXPECT_EQ(ring_buffer, expected);
}

TEST(RingBufferTests, CanBeIndexed) {
	core::RingBuffer<int, 4> ring_buffer;

	ring_buffer.push_back(10);
	ring_buffer.push_back(11);
	ring_buffer.push_back(12);

	EXPECT_EQ(ring_buffer.size(), 3);
	EXPECT_EQ(ring_buffer[0], 10);
	EXPECT_EQ(ring_buffer[1], 11);
	EXPECT_EQ(ring_buffer[2], 12);
}

TEST(RingBufferTests, WritingPastCapacity_OverwritesPrevious) {
	core::RingBuffer<int, 4> ring_buffer;

	ring_buffer.push_back(10);
	ring_buffer.push_back(11);
	ring_buffer.push_back(12);
	ring_buffer.push_back(13);
	ring_buffer.push_back(14); // writes to index 0

	const core::RingBuffer<int, 4> expected = { 14, 11, 12, 13 };
	EXPECT_EQ(ring_buffer.back(), 14);
	EXPECT_EQ(ring_buffer.front(), 11);
	EXPECT_EQ(ring_buffer, expected);
}

TEST(RingBufferTests, Iterators_EmptyBuffer_BeginEndEqual) {
	core::RingBuffer<int, 4> ring_buffer;
	EXPECT_EQ(ring_buffer.begin(), ring_buffer.end());
}

TEST(RingBufferTests, Iterators_NonEmptyNotFullBuffer_BeginEndNotEqual) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2 };
	EXPECT_NE(ring_buffer.begin(), ring_buffer.end());
}

TEST(RingBufferTests, Iterators_FullBuffer_BeginEndNotEqual) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2, 3, 4 };
	EXPECT_NE(ring_buffer.begin(), ring_buffer.end());
}

TEST(RingBufferTests, Iterators_ReadElementsByIncrement) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2, 3, 4 };

	std::vector<int> values;
	auto it = ring_buffer.begin();
	values.push_back(*(it++)); // 1
	values.push_back(*it); // 2
	++it;
	values.push_back(*(it++)); // 3
	values.push_back(*it); //  4
	it++; // end

	const std::vector<int> expected_values = { 1, 2, 3, 4 };
	EXPECT_EQ(values, expected_values);
	EXPECT_EQ(it, ring_buffer.end());
}

TEST(RingBufferTests, Iterators_ForEachLoop) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2, 3, 4 };

	std::vector<int> values;
	for (int value : ring_buffer) {
		values.push_back(value);
	}

	const std::vector<int> expected_values = { 1, 2, 3, 4 };
	EXPECT_EQ(values, expected_values);
}

TEST(RingBufferTests, Iterators_AccumulateAlgorithm) {
	core::RingBuffer<int, 4> ring_buffer = { 1, 2, 3, 4 };

	int sum = std::accumulate(ring_buffer.begin(), ring_buffer.end(), 0);

	EXPECT_EQ(sum, 1 + 2 + 3 + 4);
}
