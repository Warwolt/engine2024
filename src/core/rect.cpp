#include <core/rect.h>

namespace core {

	Rect Rect::with_pos_and_size(glm::vec2 pos, glm::vec2 size) {
		return Rect {
			.top_left = pos,
			.bottom_right = pos + size,
		};
	}

	void Rect::set_position(glm::vec2 pos) {
		const glm::vec2 size = this->size();
		this->top_left = pos;
		this->bottom_right = this->top_left + size;
	}

	void Rect::set_size(glm::vec2 size) {
		this->bottom_right = this->top_left + size;
	}

	glm::vec2 Rect::size() const {
		return this->bottom_right - this->top_left;
	}

	glm::vec2 Rect::position() const {
		return this->top_left;
	}

	bool Rect::overlaps_point(glm::vec2 point) const {
		const bool overlaps_horizontally = this->top_left.x <= point.x && point.x <= this->bottom_right.x;
		const bool overlaps_vertically = this->top_left.y <= point.y && point.y <= this->bottom_right.y;
		return overlaps_horizontally && overlaps_vertically;
	}

	Rect& Rect::operator+=(const Rect& rhs) {
		this->top_left += rhs.top_left;
		this->bottom_right += rhs.bottom_right;
		return *this;
	}

	Rect& Rect::operator-=(const Rect& rhs) {
		this->top_left -= rhs.top_left;
		this->bottom_right -= rhs.bottom_right;
		return *this;
	}

	Rect& Rect::operator*=(const Rect& rhs) {
		this->top_left *= rhs.top_left;
		this->bottom_right *= rhs.bottom_right;
		return *this;
	}

	Rect& Rect::operator/=(const Rect& rhs) {
		this->top_left /= rhs.top_left;
		this->bottom_right /= rhs.bottom_right;
		return *this;
	}

	Rect& Rect::operator+=(const glm::vec2& rhs) {
		this->top_left += rhs;
		this->bottom_right += rhs;
		return *this;
	}

	Rect& Rect::operator-=(const glm::vec2& rhs) {
		this->top_left -= rhs;
		this->bottom_right -= rhs;
		return *this;
	}

	Rect& Rect::operator*=(const glm::vec2& rhs) {
		this->top_left *= rhs;
		this->bottom_right *= rhs;
		return *this;
	}

	Rect& Rect::operator/=(const glm::vec2& rhs) {
		this->top_left /= rhs;
		this->bottom_right /= rhs;
		return *this;
	}

	Rect operator+(Rect lhs, const Rect& rhs) {
		lhs.top_left += rhs.top_left;
		lhs.bottom_right += rhs.bottom_right;
		return lhs;
	}

	Rect operator-(Rect lhs, const Rect& rhs) {
		lhs.top_left -= rhs.top_left;
		lhs.bottom_right -= rhs.bottom_right;
		return lhs;
	}

	Rect operator*(Rect lhs, const Rect& rhs) {
		lhs.top_left *= rhs.top_left;
		lhs.bottom_right *= rhs.bottom_right;
		return lhs;
	}

	Rect operator/(Rect lhs, const Rect& rhs) {
		lhs.top_left /= rhs.top_left;
		lhs.bottom_right /= rhs.bottom_right;
		return lhs;
	}

	Rect operator+(Rect lhs, const glm::vec2& rhs) {
		lhs.top_left += rhs;
		lhs.bottom_right += rhs;
		return lhs;
	}

	Rect operator-(Rect lhs, const glm::vec2& rhs) {
		lhs.top_left -= rhs;
		lhs.bottom_right -= rhs;
		return lhs;
	}

	Rect operator*(Rect lhs, const glm::vec2& rhs) {
		lhs.top_left *= rhs;
		lhs.bottom_right *= rhs;
		return lhs;
	}

	Rect operator/(Rect lhs, const glm::vec2& rhs) {
		lhs.top_left /= rhs;
		lhs.bottom_right /= rhs;
		return lhs;
	}

} // namespace core
