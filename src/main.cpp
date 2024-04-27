#include <engine.h>

#include <GL/glew.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define WIN32_LEAN_AND_MEAN
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

using EngineUpdateFn = void(engine::EngineState*);

constexpr bool PRINT_DLL_TIMESTAMPS = false;

const char* vertex_shader_src =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec4 vertexColor;\n"
	"void main() {\n"
	"    gl_Position = vec4(aPos, 1.0);\n"
	"    vertexColor = vec4(aColor, 1.0);\n"
	"}";

const char* fragment_shader_src =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec4 vertexColor;\n"
	"void main() {\n"
	"    FragColor = vertexColor;\n"
	"}";

void GLAPIENTRY on_opengl_error(
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

void print_last_winapi_error() {
	DWORD err_code = GetLastError();
	char* err_msg;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
					   (LPTSTR)&err_msg,
					   0,
					   NULL))
		return;

	static char buffer[1024];
	_snprintf_s(buffer, sizeof(buffer), "%s", err_msg);
	fprintf(stderr, "%s", buffer);
	LocalFree(err_msg);
}

int main(int /*argc*/, char** /*args*/) {
	printf("Game Engine 2024 Start\n");

	/* Initialize SDL + OpenGL*/
	SDL_Window* window;
	SDL_GLContext gl_context;
	{
		/* Initialize SDL */
		if (SDL_Init(SDL_INIT_VIDEO)) {
			fprintf(stderr, "error: SDL_Init failed with: %s\n", SDL_GetError());
			exit(1);
		}

		/* Initialize OpenGL */
		glEnable(GL_DEBUG_OUTPUT);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		/* Create window */
		window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			680,
			480,
			SDL_WINDOW_OPENGL
		);
		if (!window) {
			fprintf(stderr, "error: SDL_CreateWindow failed with: %s\n", SDL_GetError());
			exit(1);
		}

		/* Create GL Context */
		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			fprintf(stderr, "error: SDL_GL_CreateContext failed with: %s\n", SDL_GetError());
			exit(1);
		}

		/* Initialize GLEW */
		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			fprintf(stderr, "error: glewInit failed with: %s\n", glewGetErrorString(glewError));
			exit(1);
		}

		/* Set VSync */
		if (SDL_GL_SetSwapInterval(1)) {
			fprintf(stderr, "error: SDL_GL_SetSwapInterval failed with: %s\n", SDL_GetError());
		}

		/* Set OpenGL error callback */
		glDebugMessageCallback(on_opengl_error, 0);
	}

	/* Initialize shader */
	GLuint shader_program = 0;
	{
		shader_program = glCreateProgram();
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

		/* Vertex shader */
		glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
		glCompileShader(vertex_shader);
		GLint vertex_shader_compiled = GL_FALSE;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
		if (vertex_shader_compiled != GL_TRUE) {
			char info_log[512] = { 0 };
			glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
			fprintf(stderr, "error: Vertex shader failed to compile:\n%s\n", info_log);
			exit(1);
		}
		glAttachShader(shader_program, vertex_shader);

		/* Fragment shader */
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
		glCompileShader(fragment_shader);
		GLint fragment_shader_compiled = GL_FALSE;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
		if (fragment_shader_compiled != GL_TRUE) {
			char info_log[512] = { 0 };
			glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
			fprintf(stderr, "error: Fragment shader failed to compile:\n%s\n", info_log);
			exit(1);
		}
		glAttachShader(shader_program, fragment_shader);

		/* Shader program */
		glLinkProgram(shader_program);
		GLint shader_program_linked = GL_FALSE;
		glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_program_linked);
		if (shader_program_linked != GL_TRUE) {
			char info_log[512] = { 0 };
			glGetProgramInfoLog(shader_program, 512, NULL, info_log);
			fprintf(stderr, "error: Shader program failed to link:\n%s\n", info_log);
			exit(1);
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	/* Setup render data */
	GLuint vao = 0;
	GLuint vbo = 0;
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		/* Set buffer data */
		// clang-format off
		float vertices[] = {
			// positions         // colors
			0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // bottom right
			-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
			0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f    // top
		};
		// clang-format on
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		/* Configure vertex attributes */
		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
		glEnableVertexAttribArray(0);
		// color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		/* Unbind */
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	/* Load engine DLL */
	// 1. make a copy of DLL (so original can be rebuilt)
	// 2. load DLL copy
	// 3. save DLL copy timestamp
	// 4. while running, if DLL timestamp newer than DLL copy timestamp, goto 1
	FILETIME last_dll_write = {};
	std::function<EngineUpdateFn> engine_update = [](engine::EngineState*) {};
	{
		/* Load DLL */
		const char* dll_name = "GameEngine2024.dll";
		HMODULE engine_dll = LoadLibrary(dll_name);
		if (!engine_dll) {
			fprintf(stderr, "error: LoadLibraryA(\"%s\") returned null. Does the DLL exist?\n", dll_name);
			exit(1);
		}

		// DEBUG print out dll_name full path
		char dll_full_path[MAX_PATH];
		if (GetModuleFileName(engine_dll, dll_full_path, sizeof(dll_full_path)) == 0) {
			fprintf(stderr, "error: GetModuleFileName(\"%s\") failed:", dll_name);
			print_last_winapi_error();
		}

		/* Read last write timestamp */
		{
			WIN32_FILE_ATTRIBUTE_DATA data;
			if (GetFileAttributesEx(dll_full_path, GetFileExInfoStandard, &data)) {
				last_dll_write = data.ftLastWriteTime;
				if (PRINT_DLL_TIMESTAMPS) {
					SYSTEMTIME st;
					FileTimeToSystemTime(&last_dll_write, &st);
					printf("Last file write to %s: %04d-%02d-%02d %02d:%02d:%02d\n", dll_name, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				}
			} else {
				fprintf(stderr, "error: GetFileAttributesEx(\"dll_full_path\") failed with: ");
				print_last_winapi_error();
			}
		}

		/* Read functions */
		{
			const char* fn_name = "engine_update";
			EngineUpdateFn* fn = (EngineUpdateFn*)(GetProcAddress(engine_dll, fn_name));
			if (!fn) {
				fprintf(stderr, "error: GetProcAddress(\"%s\") returned null. Does the function exist?\n", fn_name);
				exit(1);
			}
			engine_update = fn;
		}
	}

	/* Main loop */
	engine::EngineState engine_state;
	bool quit = false;
	while (!quit) {
		/* Input */
		{
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
					case SDL_KEYDOWN:
						quit = true;
				}
			}
		}

		/* Update */
		engine_update(&engine_state);

		/* Render */
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(shader_program);
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);
			glBindVertexArray(NULL);

			SDL_GL_SwapWindow(window);
		}
	}

	/* Shutdown */
	{
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(shader_program);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	return 0;
}
