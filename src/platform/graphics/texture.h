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

	int wrapping_mode_to_gl_int(TextureWrapping wrapping);
	Texture add_texture(const unsigned char* data, int width, int height, TextureWrapping wrapping = TextureWrapping::ClampToEdge);
	void free_texture(Texture texture);

} // namespace platform
