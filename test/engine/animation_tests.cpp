#include <gtest/gtest.h>

#include <engine/system/animation.h>

constexpr const char ANIMATION_KEY[] = "animation-key";

TEST(AnimationTests, AnimationSystem_Initially_HoldsZeroAnimations_ForGivenKey) {
	engine::AnimationSystem animation_system;
	EXPECT_TRUE(animation_system.animations(ANIMATION_KEY).empty());
}

TEST(AnimationTests, StartedAnimation_CanLaterBeRetreived) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 1;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	EXPECT_EQ(animation.id.value, id.value);
	EXPECT_EQ(animation.id.key, id.key);
	EXPECT_EQ(animation.start, start_time);
	EXPECT_EQ(animation.length, animation_length);
	EXPECT_EQ(animation.repeats, true);
}

TEST(AnimationTests, StoppedAnimation_BecomesRemoved) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 1;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	animation_system.stop_animation(id);

	EXPECT_TRUE(animation_system.animations(ANIMATION_KEY).empty());
}

TEST(AnimationTests, StartedAnimation_GlobalTimeLessThanStartTime_AnimationNotPlaying) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = 2;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	ASSERT_TRUE(global_time < start_time);
	EXPECT_FALSE(animation.is_playing(global_time));
}

TEST(AnimationTests, StartedAnimation_AnimationHalfWayDone_LocalTime05) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 0;
	const uint64_t global_time = start_time + animation_length / 2;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	EXPECT_EQ(animation.local_time(global_time), 0.5f);
}

TEST(AnimationTests, RepeatingAnimation_GlobalTimeEqualsStartTime_AnimationIsPlaying) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = 3;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	ASSERT_TRUE(global_time == start_time);
	EXPECT_TRUE(animation.is_playing(global_time));
}

TEST(AnimationTests, RepeatingAnimation_GlobalTimePastEndTime_AnimationStillPlaying) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = start_time + animation_length;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	ASSERT_TRUE(global_time > start_time);
	EXPECT_TRUE(animation.is_playing(global_time));
}

TEST(AnimationTests, SingleShotAnimation_GlobalTimeEqualsStartTime_AnimationIsPlaying) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = 3;

	engine::AnimationID id = animation_system.start_single_shot_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	ASSERT_TRUE(global_time == start_time);
	EXPECT_TRUE(animation.is_playing(global_time));
}

TEST(AnimationTests, SingleShotAnimation_GlobalTimePastEndTime_AnimationStopped) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = start_time + animation_length;

	engine::AnimationID id = animation_system.start_single_shot_animation(ANIMATION_KEY, animation_length, start_time);
	engine::Animation animation = animation_system.animations(ANIMATION_KEY).back();

	ASSERT_TRUE(global_time > start_time);
	EXPECT_FALSE(animation.is_playing(global_time));
}

TEST(AnimationTests, SingleShotAnimation_GlobalTimePastEndTime_CanBeClearedOut) {
	engine::AnimationSystem animation_system;
	const uint64_t animation_length = 10;
	const uint64_t start_time = 3;
	const uint64_t global_time = start_time + animation_length + 1;

	engine::AnimationID id = animation_system.start_single_shot_animation(ANIMATION_KEY, animation_length, start_time);
	animation_system.clear_old_animations(global_time);
	std::optional<engine::Animation> animation = animation_system.most_recent_animation(ANIMATION_KEY);

	ASSERT_TRUE(global_time > start_time + animation_length);
	EXPECT_FALSE(animation.has_value());
}
