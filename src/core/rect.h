#pragma once

#include <glm/vec2.hpp>

namespace core {

	// Origin in upper left corner (e.g. xy-coordinates)
	// top_left o-----------o
	//          |           |
	//          |           |
	//          o-----------o bottom_right
	struct Rect {
		glm::vec2 top_left = { 0.0f, 0.0f };
		glm::vec2 bottom_right = { 0.0f, 0.0f };

		static Rect with_pos_and_size(glm::vec2 pos, glm::vec2 size);
		static Rect with_center_and_size(glm::vec2 center, glm::vec2 size);

		void set_position(glm::vec2 pos);
		void set_size(glm::vec2 size);

		glm::vec2 size() const;
		glm::vec2 position() const;
		glm::vec2 center() const;
		glm::vec2 bottom_left() const;
		glm::vec2 top_right() const;

		bool overlaps_point(glm::vec2 point) const;

		Rect& operator+=(const Rect& rhs);
		Rect& operator-=(const Rect& rhs);
		Rect& operator*=(const Rect& rhs);
		Rect& operator/=(const Rect& rhs);

		Rect& operator+=(const glm::vec2& rhs);
		Rect& operator-=(const glm::vec2& rhs);
		Rect& operator*=(const glm::vec2& rhs);
		Rect& operator/=(const glm::vec2& rhs);

		// scale centered around top_left
		Rect& operator*=(const float& rhs);
		Rect& operator/=(const float& rhs);
		friend Rect operator*(Rect lhs, const float& rhs);
		friend Rect operator/(Rect lhs, const float& rhs);

		friend Rect operator+(Rect lhs, const Rect& rhs);
		friend Rect operator-(Rect lhs, const Rect& rhs);
		friend Rect operator*(Rect lhs, const Rect& rhs);
		friend Rect operator/(Rect lhs, const Rect& rhs);

		friend Rect operator+(Rect lhs, const glm::vec2& rhs);
		friend Rect operator-(Rect lhs, const glm::vec2& rhs);
		friend Rect operator*(Rect lhs, const glm::vec2& rhs);
		friend Rect operator/(Rect lhs, const glm::vec2& rhs);
	};

	// Origin in bottom left corner (e.g. uv-coordinates)
	//             o-----------o top_right
	//             |           |
	//             |           |
	// bottom_left o-----------o
	struct FlipRect {
		glm::vec2 bottom_left;
		glm::vec2 top_right;
	};

} // namespace core
