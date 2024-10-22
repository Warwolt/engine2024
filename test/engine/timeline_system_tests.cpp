#include <gtest/gtest.h>

#include <engine/system/timeline_system.h>

constexpr const char TIMELINE_KEY[] = "timeline-key";

// TEST(TimelineTests, TimelineSystem_Initially_HoldsZeroTimelines_ForGivenKey) {
// 	engine::TimelineSystem timeline_system;
// 	EXPECT_TRUE(timeline_system.timelines_DEPRECATED(TIMELINE_KEY).empty());
// }

// TEST(TimelineTests, StartedTimeline_CanLaterBeRetreived) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 1;

// 	engine::TimelineID id = timeline_system.add_repeating_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	EXPECT_EQ(timeline.id.value, id.value);
// 	EXPECT_EQ(timeline.id.key, id.key);
// 	EXPECT_EQ(timeline.start, start_time);
// 	EXPECT_EQ(timeline.length, length);
// 	EXPECT_EQ(timeline.repeats, true);
// }

// TEST(TimelineTests, StoppedTimeline_BecomesRemoved) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 1;

// 	engine::TimelineID id = timeline_system.add_repeating_timeline(TIMELINE_KEY, start_time, length);
// 	timeline_system.remove_timeline(id);

// 	EXPECT_TRUE(timeline_system.timelines_DEPRECATED(TIMELINE_KEY).empty());
// }

// TEST(TimelineTests, StartedTimeline_GlobalTimeLessThanStartTime_TimelineNotActive) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 3;
// 	const uint64_t global_time = 2;

// 	engine::TimelineID id = timeline_system.add_repeating_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	ASSERT_TRUE(global_time < start_time);
// 	EXPECT_FALSE(timeline.is_active(global_time));
// }

// TEST(TimelineTests, StartedTimeline_TimelineHalfWayDone_LocalTime05) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 0;
// 	const uint64_t global_time = start_time + length / 2;

// 	engine::TimelineID id = timeline_system.add_repeating_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	EXPECT_EQ(timeline.local_time(global_time), 0.5f);
// }

// TEST(TimelineTests, RepeatingTimeline_GlobalTimeEqualsStartTime_TimelineIsActive) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 3;
// 	const uint64_t global_time = 3;

// 	engine::TimelineID id = timeline_system.add_repeating_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	ASSERT_TRUE(global_time == start_time);
// 	EXPECT_TRUE(timeline.is_active(global_time));
// }

// TEST(TimelineTests, RepeatingTimeline_GlobalTimePastEndTime_TimelineStillActive) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 3;
// 	const uint64_t global_time = start_time + length;

// 	engine::TimelineID id = timeline_system.add_repeating_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	ASSERT_TRUE(global_time > start_time);
// 	EXPECT_TRUE(timeline.is_active(global_time));
// }

// TEST(TimelineTests, OneShotTimeline_GlobalTimeEqualsStartTime_TimelineIsActive) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 3;
// 	const uint64_t global_time = 3;

// 	engine::TimelineID id = timeline_system.add_one_shot_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	ASSERT_TRUE(global_time == start_time);
// 	EXPECT_TRUE(timeline.is_active(global_time));
// }

// TEST(TimelineTests, OneShotTimeline_GlobalTimePastEndTime_TimelineStopped) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 3;
// 	const uint64_t global_time = start_time + length;

// 	engine::TimelineID id = timeline_system.add_one_shot_timeline(TIMELINE_KEY, start_time, length);
// 	engine::Timeline timeline = timeline_system.timelines_DEPRECATED(TIMELINE_KEY).back();

// 	ASSERT_TRUE(global_time > start_time);
// 	EXPECT_FALSE(timeline.is_active(global_time));
// }

// TEST(TimelineTests, OneShotTimeline_GlobalTimePastEndTime_CanBeClearedOut) {
// 	engine::TimelineSystem timeline_system;
// 	const uint64_t length = 10;
// 	const uint64_t start_time = 3;
// 	const uint64_t global_time = start_time + length + 1;

// 	engine::TimelineID id = timeline_system.add_one_shot_timeline(TIMELINE_KEY, start_time, length);
// 	timeline_system.remove_expired_timelines(global_time);
// 	std::optional<engine::Timeline> timeline = timeline_system.most_recent_timeline_DEPRECATED(TIMELINE_KEY);

// 	ASSERT_TRUE(global_time > start_time + length);
// 	EXPECT_FALSE(timeline.has_value());
// }
