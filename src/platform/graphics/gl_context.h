#pragma once

#include <platform/graphics/canvas.h>
#include <platform/graphics/shader_program.h>
#include <platform/graphics/texture.h>

#include <expected>

typedef void* SDL_GLContext; // from SDL_video.h

namespace platform {

	enum class ShaderProgramError {
		VertexShaderFailedToCompile,
		FragmentShaderFailedToCompile,
		ShaderProgramFailedToLink,
	};

	// Wrapper around OpenGL functions, to make it easier to track what has a
	// dependency on the OpenGL context and avoid e.g. using from wrong thread.
	class OpenGLContext {
	public:
		explicit OpenGLContext(SDL_GLContext) {}

		virtual Texture add_texture(
			const unsigned char* data,
			int width,
			int height,
			TextureWrapping wrapping = TextureWrapping::ClampToEdge,
			TextureFilter filter = TextureFilter::Nearest
		);
		virtual void set_texture_wrapping(Texture texture, TextureWrapping wrapping);
		virtual void set_texture_filter(Texture texture, TextureFilter filter);
		virtual void free_texture(Texture texture);

		virtual Canvas add_canvas(int width, int height, TextureWrapping wrapping = TextureWrapping::ClampToEdge, TextureFilter filter = TextureFilter::Nearest);
		virtual void free_canvas(Canvas canvas);

		virtual std::expected<ShaderProgram, ShaderProgramError> add_shader_program(const char* vertex_src, const char* fragment_src);
		virtual void free_shader_program(const ShaderProgram& shader_program);
	};

} // namespace platform
