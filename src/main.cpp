#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <GL/glu.h>

#include <cstdlib>
#include <stdio.h>

const char* vertex_shader_src[] = {
	"#version 140\n"
	"in vec2 LVertexPos2D;"
	"void main() {"
	"  gl_Position = vec4(LVertexPos2D.x, LVertexPos2D.y, 0, 1);"
	"}"
};

const char* fragment_shader_src[] = {
	"#version 140\n"
	"out vec4 LFragment;"
	"void main() {"
	"  LFragment = vec4(1.0, 1.0, 1.0, 1.0);"
	"}"
};

void GLAPIENTRY
on_opengl_error(
	GLenum /*source*/,
	GLenum type,
	GLuint /*id*/,
	GLenum severity,
	GLsizei /*length*/,
	const GLchar* message,
	const void* /*userParam*/
) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

int main(int /*argc*/, char** /*args*/) {
	printf("Hello Game Engine 2024!\n");

	/* Initialize SDL + OpenGL*/
	SDL_Window* window;
	SDL_GLContext gl_context;
	{
		if (SDL_Init(SDL_INIT_VIDEO)) {
			fprintf(stderr, "SDL_Init failed with error: %s\n", SDL_GetError());
			exit(1);
		}

		glEnable(GL_DEBUG_OUTPUT);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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
			exit(1);
		}

		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			fprintf(stderr, "SDL_GL_CreateContext failed with error: %s\n", SDL_GetError());
			exit(1);
		}

		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			fprintf(stderr, "glewInit failed with error: %s\n", glewGetErrorString(glewError));
		}

		glDebugMessageCallback(on_opengl_error, 0);
	}

	/* Initialize shader */
	GLuint shader_program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	{
		shader_program = glCreateProgram();
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);

		// vertex shader
		glShaderSource(vertex_shader, 1, vertex_shader_src, NULL);
		glCompileShader(vertex_shader);
		GLint vertex_shader_compiled = GL_FALSE;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
		if (vertex_shader_compiled != GL_TRUE) {
			fprintf(stderr, "Vertex shader failed to compile\n");
			exit(1);
		}
		glAttachShader(shader_program, vertex_shader);

		// fragment shader
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_src, NULL);
		glCompileShader(fragment_shader);
		GLint fragment_shader_compiled = GL_FALSE;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
		if (fragment_shader_compiled != GL_TRUE) {
			fprintf(stderr, "Fragment shader failed to compile\n");
			exit(1);
		}
		glAttachShader(shader_program, fragment_shader);

		// shader program
		glLinkProgram(shader_program);
		GLint shader_program_linked = GL_FALSE;
		glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_program_linked);
		if (shader_program_linked != GL_TRUE) {
			fprintf(stderr, "Shader program failed to link\n");
			exit(1);
		}
	}

	SDL_Delay(1000);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
