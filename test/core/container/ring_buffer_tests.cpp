#include <gtest/gtest.h>

#include <core/container/ring_buffer.h>

TEST(RingBufferTests, InitiallyEmpty) {
	core::RingBuffer<int, 4> ring_buffer;

	EXPECT_EQ(ring_buffer.size(), 0);
	EXPECT_TRUE(ring_buffer.empty());
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
