#include <gtest/gtest.h>

#include <core/async_batch.h>

TEST(AsyncBatchTests, DefaultBatch_IsEmptyAndDone) {
	core::AsyncBatch<int(int)> batch;

	EXPECT_TRUE(batch.is_done());
	EXPECT_TRUE(batch.values().empty());
}

TEST(AsyncBatchTests, BatchWithItems_NotUpdated_NotDone) {
	std::vector<int> items = { 1, 2, 3 };
	core::AsyncBatch<int(int)> batch(items, [](int x) { return x + 1; });

	EXPECT_FALSE(batch.is_done());
	EXPECT_TRUE(batch.values().empty());
}

TEST(AsyncBatchTests, BatchWithItems_UpdatedUntilDone_HasValues) {
	std::vector<int> items = { 1, 2, 3 };
	core::AsyncBatch<int(int)> batch(items, [](int x) { return x + 1; });

	while (!batch.is_done()) {
		batch.update();
	}

	std::vector<int> expected_values = { 2, 3, 4 };
	EXPECT_TRUE(batch.is_done());
	EXPECT_EQ(batch.values(), expected_values);
}

TEST(AsyncBatchTests, Batch_CanBeAssigned) {
	std::vector<int> items = { 1, 2, 3 };
	core::AsyncBatch<int(int)> batch;
	batch = core::AsyncBatch<int(int)>(items, [](int x) { return x + 1; });

	EXPECT_FALSE(batch.is_done());
}

TEST(AsyncBatchTests, BatchSizeIsFixed) {
	std::vector<int> items = { 1, 2, 3 };
	core::AsyncBatch<int(int)> empty_batch;
	core::AsyncBatch<int(int)> nonempty_batch = core::AsyncBatch<int(int)>(items, [](int x) { return x + 1; });

	EXPECT_EQ(empty_batch.size(), 0);
	EXPECT_EQ(nonempty_batch.size(), items.size());
}
