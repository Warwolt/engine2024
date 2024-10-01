#pragma once

#include <SDL2/SDL_opengl.h>

namespace platform {

	struct ShaderProgram {
		GLuint id;
		GLuint vao;
		GLuint vbo;
		struct {
			GLint projection;
		} uniforms;
	};

} // namespace platform
