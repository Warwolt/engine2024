#include <gtest/gtest.h>

#include <core/future.h>

#include <algorithm>

TEST(FutureTests, FutureIsReady_FutureFromNonResolvedPromise_NotReady) {
	std::promise<int> promise;
	std::future<int> future = promise.get_future();
	EXPECT_FALSE(core::future_is_ready(future));
}

TEST(FutureTests, FutureIsReady_FutureFromResolvedPromise_IsReady) {
	std::promise<int> promise;
	std::future<int> future = promise.get_future();
	promise.set_value(123);
	EXPECT_TRUE(core::future_is_ready(future));
}

TEST(FutureTests, FutureIsReady_InvalidFuture_NotReady) {
	std::future<int> future; // no corresponding promise, not valid
	EXPECT_FALSE(future.valid());
	EXPECT_FALSE(core::future_is_ready(future));
}

TEST(FutureTests, AsyncBatch_LaunchedFutures_AreValid) {
	std::vector<int> input = { 1, 2, 3 };

	std::vector<std::future<int>> futures = core::batch_async(std::launch::async, input.begin(), input.end(), [](int x) { return x + 1; });
	size_t valid_futures = std::count_if(futures.begin(), futures.end(), [](const std::future<int>& future) { return future.valid(); });

	EXPECT_EQ(valid_futures, input.size());
}

TEST(FutureTests, AsyncBatch_CollectValues) {
	std::vector<int> input = { 1, 2, 3 };

	std::vector<std::future<int>> futures = core::batch_async(std::launch::async, input.begin(), input.end(), [](int x) { return x + 1; });
	std::vector<int> results;
	core::get_all_batch_values(futures, std::back_inserter(results));

	const std::vector<int> expected_results = { 2, 3, 4 };
	EXPECT_EQ(results, expected_results);
}

TEST(FutureTests, AsyncBatch_CollectValues_RethrowsFirstException) {
	std::promise<int> promise1;
	std::promise<int> promise2;
	std::vector<std::future<int>> futures;
	futures.push_back(promise1.get_future());
	futures.push_back(promise2.get_future());

	/* Set first exception */
	try {
		throw std::runtime_error("First");
	}
	catch (...) {
		promise1.set_exception(std::current_exception());
	}

	/* Set second exception */
	try {
		throw std::runtime_error("Second");
	}
	catch (...) {
		promise2.set_exception(std::current_exception());
	}

	/* Expect first exception to be rethrown */
	try {
		std::vector<int> results = core::get_all_batch_values(futures);
		FAIL() << "Expected error to be thrown";
	}
	catch (const std::runtime_error& expected) {
		EXPECT_STREQ(expected.what(), "First");
	}
}

TEST(FutureTests, AsyncBatch_CollectReadyValues) {
	std::promise<int> promises[3];
	std::vector<std::future<int>> futures;
	futures.push_back(promises[0].get_future());
	futures.push_back(promises[1].get_future());
	futures.push_back(promises[2].get_future());
	std::vector<int> ready_values;

	/* Make first and last future ready */
	promises[0].set_value(10);
	promises[2].set_value(12);
	core::get_ready_batch_values(futures, std::back_inserter(ready_values));
	const std::vector<int> expected_ready_values = { 10, 12 };
	EXPECT_EQ(ready_values, expected_ready_values);

	/* Make remaining future ready */
	promises[1].set_value(11);
	core::get_ready_batch_values(futures, std::back_inserter(ready_values));
	const std::vector<int> expected_ready_values2 = { 10, 12, 11 };
	EXPECT_EQ(ready_values, expected_ready_values2);
}
