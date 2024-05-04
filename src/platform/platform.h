#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <expected>

namespace platform {

	enum class CreateGLContextError {
		FailedToCreateContext,
		FailedToInitializeGlew,
		FailedToSetVSync,
	};

	bool initialize();
	void deinitialize(SDL_Window* window);
	SDL_Window* create_window(int widht, int height);
	std::expected<SDL_GLContext, CreateGLContextError> create_gl_context(SDL_Window* window);

} // namespace platform
