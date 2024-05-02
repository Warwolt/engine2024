#include <gtest/gtest.h>

#include <platform/input/keyboard.h>

TEST(keyboard, button_initially_released) {
	platform::Keyboard keyboard;
	int key = 1234;

	EXPECT_FALSE(keyboard.key_pressed(key));
	EXPECT_TRUE(keyboard.key_released(key));
}
