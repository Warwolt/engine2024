#include <gtest/gtest.h>

#include <platform/input/keyboard.h>

using ButtonEvent = platform::ButtonEvent;

constexpr int KEY = 1234;

TEST(KeyboardTests, ButtonInitiallyReleased) {
	platform::Keyboard keyboard;

	EXPECT_EQ(keyboard.key_pressed(KEY), false);
	EXPECT_EQ(keyboard.key_pressed_now(KEY), false);
	EXPECT_EQ(keyboard.key_released(KEY), true);
	EXPECT_EQ(keyboard.key_released_now(KEY), false);
}

TEST(KeyboardTests, ButtonReleased_DownEvent_IsPressedNow) {
	platform::Keyboard keyboard;

	keyboard.register_event(KEY, ButtonEvent::Down);
	keyboard.update();

	EXPECT_EQ(keyboard.key_pressed(KEY), true);
	EXPECT_EQ(keyboard.key_pressed_now(KEY), true);
	EXPECT_EQ(keyboard.key_released(KEY), false);
	EXPECT_EQ(keyboard.key_released_now(KEY), false);
}

TEST(KeyboardTests, ButtonPressed_UpEvent_IsReleasedNow) {
	platform::Keyboard keyboard;

	keyboard.register_event(KEY, ButtonEvent::Down);
	keyboard.update();
	keyboard.register_event(KEY, ButtonEvent::Up);
	keyboard.update();

	EXPECT_EQ(keyboard.key_pressed(KEY), false);
	EXPECT_EQ(keyboard.key_pressed_now(KEY), false);
	EXPECT_EQ(keyboard.key_released(KEY), true);
	EXPECT_EQ(keyboard.key_released_now(KEY), true);
}

TEST(KeyboardTests, ButtonPressedNow_DownEvent_IsPressed) {
	platform::Keyboard keyboard;

	keyboard.register_event(KEY, ButtonEvent::Down);
	keyboard.update();
	keyboard.register_event(KEY, ButtonEvent::Down);
	keyboard.update();

	EXPECT_EQ(keyboard.key_pressed(KEY), true);
	EXPECT_EQ(keyboard.key_pressed_now(KEY), false);
	EXPECT_EQ(keyboard.key_released(KEY), false);
	EXPECT_EQ(keyboard.key_released_now(KEY), false);
}

TEST(KeyboardTests, ButtonPressedNow_WithModifier_IsPressedWithThatModifier) {
	platform::Keyboard keyboard;

	keyboard.register_event_with_modifier(KEY, ButtonEvent::Down, 1234);
	keyboard.update();

	EXPECT_EQ(keyboard.key_pressed(KEY), true);
	EXPECT_EQ(keyboard.key_pressed_with_modifier(KEY, 1234), true);
}

TEST(KeyboardTests, ButtonPressedNow_WithModifier_IsNotPressedWithOtherModifier) {
	platform::Keyboard keyboard;

	keyboard.register_event_with_modifier(KEY, ButtonEvent::Down, 1234);
	keyboard.update();

	EXPECT_EQ(keyboard.key_pressed(KEY), true);
	EXPECT_EQ(keyboard.key_pressed_now(KEY), true);
	EXPECT_EQ(keyboard.key_pressed_with_modifier(KEY, 2345), false);
	EXPECT_EQ(keyboard.key_pressed_now_with_modifier(KEY, 2345), false);
}
