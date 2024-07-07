#pragma once

#include <SDL2/SDL_opengl.h>
#include <glm/vec2.hpp>

namespace platform {

	struct Texture {
		GLuint id;
		glm::vec2 size;
	};

	Texture add_texture(const unsigned char* data, int width, int height);
	void free_texture(Texture texture);

} // namespace platform
