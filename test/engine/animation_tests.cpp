#include <gtest/gtest.h>

#include <engine/animation.h>

constexpr const char ANIMATION_KEY[] = "animation-key";

TEST(AnimationTests, AnimationSystem_Initially_HoldsZeroAnimations_ForGivenKey) {
	engine::AnimationSystem animation_system;
	EXPECT_TRUE(animation_system.animations(ANIMATION_KEY).empty());
}

TEST(AnimationTests, StartedAnimation_CanLaterBeRetreived) {
	engine::AnimationSystem animation_system;
	const float animation_length = 10.0;
	const float start_time = 1.0;

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
	const float animation_length = 10.0;
	const float start_time = 1.0;

	engine::AnimationID id = animation_system.start_animation(ANIMATION_KEY, animation_length, start_time);
	animation_system.stop_animation(id);

	EXPECT_TRUE(animation_system.animations(ANIMATION_KEY).empty());
}

// TODO: global_time < animation_start => animation is not playing

// TODO: animation_start < global_time => animation is playing

// TODO: animation_start < global_time && global_time <= animation_end => animation is playing

// TODO: global time -> local time tests
