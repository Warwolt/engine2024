#include <gtest/gtest.h>

#include <core/future.h>

TEST(CoreFutureTests, FutureIsReady_FutureFromNonResolvedPromise_NotReady) {
	std::promise<int> promise;
	std::future<int> future = promise.get_future();
	EXPECT_FALSE(core::future_is_ready(future));
}

TEST(CoreFutureTests, FutureIsReady_FutureFromResolvedPromise_IsReady) {
	std::promise<int> promise;
	std::future<int> future = promise.get_future();
	promise.set_value(123);
	EXPECT_TRUE(core::future_is_ready(future));
}

TEST(CoreFutureTests, FutureIsReady_InvalidFuture_NotReady) {
	std::future<int> future; // no corresponding promise, not valid
	EXPECT_FALSE(future.valid());
	EXPECT_FALSE(core::future_is_ready(future));
}
