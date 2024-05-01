#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <platform/assert.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/renderer.h>
#include <platform/timing.h>

#include <expected>

namespace platform {

	enum class CreateGLContextError {
		FailedToCreateContext,
		FailedToInitializeGlew,
		FailedToSetVSync,
	};

	const char* create_gl_context_error_to_string(CreateGLContextError error);

	bool initialize();
	void deinitialize(SDL_Window* window);
	SDL_Window* create_window();
	std::expected<SDL_GLContext, CreateGLContextError> create_gl_context(SDL_Window* window);

} // namespace platform
