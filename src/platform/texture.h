#pragma once
#include <SDL2/SDL_opengl.h>

namespace platform {

	struct Texture {
		GLuint id;
		int width;
		int height;
	};

	Texture add_texture(const unsigned char* data, int width, int height);
	void free_texture(Texture texture);

} // namespace platform
