#include <gtest/gtest.h>

#include <core/rect.h>

TEST(RectTests, PosAndSizeConstructor) {
	core::Rect rect = core::Rect::with_pos_and_size(glm::vec2 { 1, 2 }, glm::vec2 { 3, 4 });

	glm::vec2 expected_position = glm::vec2 { 1, 2 };
	glm::vec2 expected_size = glm::vec2 { 3, 4 };
	EXPECT_EQ(rect.position(), expected_position);
	EXPECT_EQ(rect.size(), expected_size);
	EXPECT_EQ(rect.top_left, expected_position);
	EXPECT_EQ(rect.bottom_right, expected_position + expected_size);
}

TEST(RectTests, PointOverlap) {
	core::Rect rect = core::Rect::with_pos_and_size(glm::vec2 { 0, 0 }, glm::vec2 { 4, 4 });
	glm::vec2 inside_point = { 2, 2 };
	glm::vec2 left_point = { -1, 2 };
	glm::vec2 right_point = { 5, 2 };
	glm::vec2 up_point = { 2, -1 };
	glm::vec2 down_point = { 2, 5 };

	EXPECT_TRUE(rect.overlaps_point(inside_point));
	EXPECT_FALSE(rect.overlaps_point(left_point));
	EXPECT_FALSE(rect.overlaps_point(right_point));
	EXPECT_FALSE(rect.overlaps_point(up_point));
	EXPECT_FALSE(rect.overlaps_point(down_point));
}
