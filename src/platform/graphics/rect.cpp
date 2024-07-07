#include <platform/graphics/rect.h>

namespace platform {

	void Rect::set_size(glm::vec2 size) {
		this->bottom_right = this->top_left + size;
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

} // namespace platform
