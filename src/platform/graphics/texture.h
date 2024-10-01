#pragma once

#include <SDL2/SDL_opengl.h>
#include <glm/vec2.hpp>

namespace platform {

	struct Texture {
		GLuint id;
		glm::vec2 size;
	};

	enum class TextureWrapping {
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
	};

	enum class TextureFilter {
		Nearest,
		Linear,
	};

} // namespace platform
