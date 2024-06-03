#include <gtest/gtest.h>

#include <engine/animation.h>

constexpr const char ANIMATION_KEY[] = "animation-key";

TEST(AnimationTests, AnimationSystem_Initially_HoldsZeroAnimations_ForGivenKey) {
	engine::AnimationSystem animation_system;
	EXPECT_TRUE(animation_system.animations(ANIMATION_KEY).empty());
}

// TODO write more tests !
