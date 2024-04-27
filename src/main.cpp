#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <GL/GLU.h>

#include <stdio.h>

// void GLAPIENTRY on_opengl_error(
// 	GLenum source,
// 	GLenum type,
// 	GLuint id,
// 	GLenum severity,
// 	GLsizei length,
// 	const GLchar* message,
// 	const void* userParam
// ) {
// 	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
// }

int main(int argc, char* args[]) {
	printf("Hello Game Engine 2024!\n");

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_Init failed with error: %s\n", SDL_GetError());
		return 1;
	}

	glEnable(GL_DEBUG_OUTPUT);
	// glDebugMessageCallback(on_opengl_error, 0); // requires GLEW

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_Window* window;
	SDL_GLContext gl_context;
	{
		window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			680,
			480,
			SDL_WINDOW_OPENGL
		);
		if (!window) {
			fprintf(stderr, "SDL_CreateWindow failed with error: %s\n", SDL_GetError());
			return 1;
		}

		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			fprintf(stderr, "SDL_GL_CreateContext failed with error: %s\n", SDL_GetError());
			return 1;
		}
	}

	// Requires GLEW
	// GLuint shader_program;
	// GLuint vertex_shader;
	// {
	// 	shader_program = glCreateProgram();
	// 	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	// }

	SDL_Delay(1000);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
