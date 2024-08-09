#pragma once

#include <glm/vec2.hpp>

namespace platform {

	// Origin in upper left corner (e.g. xy-coordinates)
	// top_left o-----------o
	//          |           |
	//          |           |
	//          o-----------o bottom_right
	struct Rect {
		glm::vec2 top_left;
		glm::vec2 bottom_right;

		static Rect with_pos_and_size(glm::vec2 pos, glm::vec2 size);

		void set_position(glm::vec2 pos);
		void set_size(glm::vec2 size);
		glm::vec2 size() const;

		Rect& operator+=(const Rect& rhs);
		Rect& operator-=(const Rect& rhs);
		Rect& operator*=(const Rect& rhs);
		Rect& operator/=(const Rect& rhs);

		Rect& operator+=(const glm::vec2& rhs);
		Rect& operator-=(const glm::vec2& rhs);
		Rect& operator*=(const glm::vec2& rhs);
		Rect& operator/=(const glm::vec2& rhs);

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

} // namespace platform
