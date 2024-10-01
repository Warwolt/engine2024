#pragma once

#include <platform/graphics/texture.h>
#include <platform/graphics/canvas.h>

typedef void* SDL_GLContext; // from SDL_video.h

namespace platform {

	// Wrapper around OpenGL functions, to make it easier to track what has a
	// dependency on the OpenGL context and avoid e.g. using from wrong thread.
	class GraphicsContext {
	public:
		explicit GraphicsContext(SDL_GLContext) {}

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

		Canvas add_canvas(int width, int height, TextureWrapping wrapping = TextureWrapping::ClampToEdge, TextureFilter filter = TextureFilter::Nearest);
		void free_canvas(Canvas canvas);
	};

} // namespace platform
