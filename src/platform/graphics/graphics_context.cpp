#include <GL/glew.h> // must be included before <SDL2/SDL_opengl.h>

#include <platform/graphics/graphics_context.h>

namespace platform {

	static int _wrapping_mode_to_gl_int(TextureWrapping wrapping) {
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

	static int _filter_mode_to_gl_int(TextureFilter filter) {
		switch (filter) {
			case TextureFilter::Linear:
				return GL_LINEAR;

			case TextureFilter::Nearest:
				return GL_NEAREST;
		}
		return 0;
	}

	Texture GraphicsContext::add_texture(
		const unsigned char* data,
		int width,
		int height,
		TextureWrapping wrapping,
		TextureFilter filter
	) {
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

	void GraphicsContext::set_texture_wrapping(Texture texture, TextureWrapping wrapping) {
		const int wrapping_int = _wrapping_mode_to_gl_int(wrapping);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping_int);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping_int);
	}

	void GraphicsContext::set_texture_filter(Texture texture, TextureFilter filter) {
		const int filter_int = _filter_mode_to_gl_int(filter);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_int);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_int);
	}

	void GraphicsContext::free_texture(Texture texture) {
		glDeleteTextures(1, &texture.id);
	}

	Canvas GraphicsContext::add_canvas(int width, int height, TextureWrapping wrapping, TextureFilter filter) {
		// create texture
		GLuint texture_id;
		glGenTextures(1, &texture_id);
		Texture texture = Texture { texture_id, glm::vec2 { width, height } };
		glBindTexture(GL_TEXTURE_2D, texture_id);

		// create buffer
		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		set_texture_filter(texture, filter);
		set_texture_wrapping(texture, wrapping);

		// attach texture to buffer and draw buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

		GLuint render_buffer;
		glGenRenderbuffers(1, &render_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
		glRenderbufferStorage(
			GL_RENDERBUFFER,
			GL_DEPTH24_STENCIL8,
			width,
			height
		);

		glBindTexture(GL_TEXTURE_2D, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		return Canvas { framebuffer, texture };
	}

	void GraphicsContext::free_canvas(Canvas canvas) {
		glDeleteFramebuffers(1, &canvas.framebuffer);
		glDeleteTextures(1, &canvas.texture.id);
	}

} // namespace platform
