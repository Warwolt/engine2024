#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <platform/resource_handle.h>

#include <functional>
#include <type_traits>
#include <utility>

using namespace platform;

static bool g_was_called;

namespace {
	void _delete_resource(int /* resource */) {
		g_was_called = true;
	}
}

TEST(resource_handle, handle_calls_deleter_when_destructed) {
	bool was_called = false;
	std::function<void(int)> deleter = [&was_called](int) { was_called = true; };
	{
		ResourceHandle<int, void(int)> handle(0, deleter);
	}
	EXPECT_TRUE(was_called);
}

TEST(resource_handle, free_functions_can_be_deleters) {
	g_was_called = false;
	{
		ResourceHandle<int, void(int)> handle(0, _delete_resource);
	}
	EXPECT_TRUE(g_was_called);
}

TEST(resource_handle, resource_handles_cannot_be_copied_or_assigned) {
	bool is_copy_constructible = std::is_copy_constructible<ResourceHandle<int, void(int)>>::value;
	bool is_copy_assignable = std::is_copy_assignable<ResourceHandle<int, void(int)>>::value;
	EXPECT_FALSE(is_copy_constructible);
	EXPECT_FALSE(is_copy_assignable);
}

TEST(resource_handle, moved_resources_do_not_call_the_deleter) {
	int num_calls = 0;
	std::function<void(int)> deleter = [&num_calls](int) { num_calls++; };
	{
		ResourceHandle<int, void(int)> handle1(10, deleter);
		ResourceHandle<int, void(int)> handle2 = std::move(handle1);
	}
	EXPECT_EQ(num_calls, 1);
}

TEST(resource_handle, resources_can_be_retreived_from_handle) {
	const std::pair<int, int> pair = { 12, 34 };

	ResourceHandle<std::pair<int, int>, void(std::pair<int, int>)> handle(pair, nullptr);
	const ResourceHandle<std::pair<int, int>, void(std::pair<int, int>)>& const_handle = handle;

	EXPECT_EQ(handle.get(), pair);
	EXPECT_EQ(*handle, pair);

	EXPECT_EQ(const_handle.get(), pair);
	EXPECT_EQ(*const_handle, pair);

	EXPECT_EQ(handle->first, 12);
	EXPECT_EQ(const_handle->first, 12);
}
