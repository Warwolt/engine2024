#pragma once

#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>

namespace platform {

	struct Vertex {
		glm::vec2 pos;
		glm::vec4 color;
		glm::vec2 uv;
	};

} // namespace platform
