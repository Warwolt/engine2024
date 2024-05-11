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
	void shutdown(SDL_GLContext gl_context);

	std::expected<SDL_GLContext, CreateGLContextError> create_gl_context(SDL_Window* window);

} // namespace platform
