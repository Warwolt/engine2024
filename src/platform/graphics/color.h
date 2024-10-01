#pragma once

#include <glm/glm.hpp>

#include <algorithm>

namespace platform {

	namespace Color {
		constexpr glm::vec4 rgba(int r, int g, int b, int a) {
			return glm::vec4 {
				(float)std::clamp(r, 0, 255) / 255.0f,
				(float)std::clamp(g, 0, 255) / 255.0f,
				(float)std::clamp(b, 0, 255) / 255.0f,
				(float)std::clamp(a, 0, 255) / 255.0f,
			};
		}

		// clang-format off
		constexpr glm::vec4 white      { 1.0f, 1.0f, 1.0f, 1.0f };
		constexpr glm::vec4 red        { 1.0f, 0.0f, 0.0f, 1.0f };
		constexpr glm::vec4 green      { 0.0f, 1.0f, 0.0f, 1.0f };
		constexpr glm::vec4 blue       { 0.0f, 0.0f, 1.0f, 1.0f };
		constexpr glm::vec4 black      { 0.0f, 0.0f, 0.0f, 1.0f };
		constexpr glm::vec4 light_grey { 0.75f, 0.75f, 0.75f, 1.0f };
		constexpr glm::vec4 dark_grey  { 0.50f, 0.50f, 0.50f, 1.0f };

		// clang-format on
	}

} // namespace platform
