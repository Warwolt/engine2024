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

	int wrapping_mode_to_gl_int(TextureWrapping wrapping);
	int filter_mode_to_gl_int(TextureFilter filter);

	Texture add_texture(
		const unsigned char* data,
		int width,
		int height,
		TextureWrapping wrapping = TextureWrapping::ClampToEdge,
		TextureFilter filter = TextureFilter::Nearest
	);
	void set_texture_wrapping(Texture texture, TextureWrapping wrapping);
	void set_texture_filter(Texture texture, TextureFilter filter);
	void free_texture(Texture texture);

} // namespace platform
