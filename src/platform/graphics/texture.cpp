#include <platform/graphics/texture.h>

namespace platform {

	int wrapping_mode_to_gl_int(TextureWrapping wrapping) {
		switch (wrapping) {
			case TextureWrapping::Repeat:
				return GL_REPEAT;

			case TextureWrapping::MirroredRepeat:
				return GL_MIRRORED_REPEAT;

			case TextureWrapping::ClampToEdge:
				return GL_CLAMP_TO_EDGE;

			case TextureWrapping::ClampToBorder:
				return GL_CLAMP_TO_BORDER;
		}
		return 0;
	}

	int filter_mode_to_gl_int(TextureFilter filter) {
		switch (filter) {
			case TextureFilter::Linear:
				return GL_LINEAR;

			case TextureFilter::Nearest:
				return GL_NEAREST;
		}
		return 0;
	}

	Texture add_texture(const unsigned char* data, int width, int height, TextureWrapping wrapping, TextureFilter filter) {
		GLuint texture_id;
		glGenTextures(1, &texture_id);

		Texture texture = Texture { texture_id, glm::vec2 { width, height } };
		glBindTexture(GL_TEXTURE_2D, texture_id);

		set_texture_filter(texture, filter);
		set_texture_wrapping(texture, wrapping);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, NULL);
		return texture;
	}

	void set_texture_wrapping(Texture texture, TextureWrapping wrapping) {
		int wrapping_int = wrapping_mode_to_gl_int(wrapping);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping_int);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping_int);
	}

	void set_texture_filter(Texture texture, TextureFilter filter) {
		int filter_int = filter_mode_to_gl_int(filter);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_int);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_int);
	}

	void free_texture(Texture texture) {
		glDeleteTextures(1, &texture.id);
	}

} // namespace platform
