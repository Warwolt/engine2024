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

	Texture add_texture(const unsigned char* data, int width, int height, TextureWrapping wrapping) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		int param = wrapping_mode_to_gl_int(wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, NULL);
		return Texture { texture, glm::vec2 { width, height } };
	}

	void free_texture(Texture texture) {
		glDeleteTextures(1, &texture.id);
	}

} // namespace platform
