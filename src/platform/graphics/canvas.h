#pragma once

#include <SDL2/SDL_opengl.h>
#include <platform/graphics/texture.h>

namespace platform {

	struct Canvas {
		GLuint framebuffer;
		Texture texture;
	};

} // namespace platform
