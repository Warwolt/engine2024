#include <gtest/gtest.h>

#include <engine/system/timeline_system.h>

TEST(TimelineTests, InvalidID_HasNoCorrespondingTimeline) {
	engine::TimelineSystem timeline_system;
	engine::TimelineID id = engine::INVALID_TIMELINE_ID;

	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	EXPECT_FALSE(timeline.has_value());
}

TEST(TimelineTests, DefaultID_HasNoCorrespondingTimeline) {
	engine::TimelineSystem timeline_system;
	engine::TimelineID id = engine::TimelineID();

	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	EXPECT_FALSE(timeline.has_value());
}

TEST(TimelineTests, StartedTimeline_CanLaterBeRetreived) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 1;

	engine::TimelineID id = timeline_system.add_repeating_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(timeline.has_value());
	EXPECT_EQ(timeline->start, start_time);
	EXPECT_EQ(timeline->length, length);
	EXPECT_EQ(timeline->repeats, true);
}

TEST(TimelineTests, TimelineCanBeRemoved) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 1;

	engine::TimelineID id = timeline_system.add_repeating_timeline(start_time, length);
	timeline_system.remove_timeline(id);

	EXPECT_FALSE(timeline_system.timeline(id).has_value());
}

TEST(TimelineTests, StartedTimeline_GlobalTimeLessThanStartTime_TimelineNotActive) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = 2;

	engine::TimelineID id = timeline_system.add_repeating_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(global_time < start_time);
	ASSERT_TRUE(timeline.has_value());
	EXPECT_FALSE(timeline->is_active(global_time));
	EXPECT_EQ(timeline->local_time(global_time), 0.0f);
	EXPECT_EQ(timeline_system.local_time(id, global_time), timeline->local_time(global_time));
}

TEST(TimelineTests, RepeatingTimeline_GlobalTimeHalfWayPoint_LocalTime05) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 0;
	const uint64_t global_time = start_time + length / 2;

	engine::TimelineID id = timeline_system.add_repeating_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(timeline.has_value());
	EXPECT_TRUE(timeline->is_active(global_time));
	EXPECT_EQ(timeline->local_time(global_time), 0.5f);
	EXPECT_EQ(timeline_system.local_time(id, global_time), timeline->local_time(global_time));
}

TEST(TimelineTests, RepeatingTimeline_GlobalTimeEqualsStartTime_IsActive) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = 3;

	engine::TimelineID id = timeline_system.add_repeating_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(global_time == start_time);
	ASSERT_TRUE(timeline.has_value());
	EXPECT_TRUE(timeline->is_active(global_time));
	EXPECT_EQ(timeline->local_time(global_time), 0.0f);
	EXPECT_EQ(timeline_system.local_time(id, global_time), timeline->local_time(global_time));
}

TEST(TimelineTests, RepeatingTimeline_GlobalTimePastEndTime_IsActive) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = start_time + length;

	engine::TimelineID id = timeline_system.add_repeating_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(global_time > start_time);
	ASSERT_TRUE(timeline.has_value());
	EXPECT_TRUE(timeline->is_active(global_time));
	EXPECT_NE(timeline->local_time(global_time), 1.0f);
	EXPECT_EQ(timeline_system.local_time(id, global_time), timeline->local_time(global_time));
}

TEST(TimelineTests, OneShotTimeline_GlobalTimeEqualsStartTime_IsActive) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = 3;

	engine::TimelineID id = timeline_system.add_one_shot_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(global_time == start_time);
	ASSERT_TRUE(timeline.has_value());
	EXPECT_TRUE(timeline->is_active(global_time));
	EXPECT_EQ(timeline->local_time(global_time), 0.0f);
	EXPECT_EQ(timeline_system.local_time(id, global_time), timeline->local_time(global_time));
}

TEST(TimelineTests, OneShotTimeline_GlobalTimePastEndTime_NotActive) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = start_time + length;

	engine::TimelineID id = timeline_system.add_one_shot_timeline(start_time, length);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(global_time > start_time);
	ASSERT_TRUE(timeline.has_value());
	EXPECT_FALSE(timeline->is_active(global_time));
	EXPECT_EQ(timeline->local_time(global_time), 1.0f);
	EXPECT_EQ(timeline_system.local_time(id, global_time), timeline->local_time(global_time));
}

TEST(TimelineTests, OneShotTimeline_GlobalTimePastEndTime_Removed) {
	engine::TimelineSystem timeline_system;
	const uint64_t length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = start_time + length + 1;

	engine::TimelineID id = timeline_system.add_one_shot_timeline(start_time, length);
	timeline_system.remove_expired_timelines(global_time);
	std::optional<engine::Timeline> timeline = timeline_system.timeline(id);

	ASSERT_TRUE(global_time > start_time + length);
	EXPECT_FALSE(timeline.has_value());
}
