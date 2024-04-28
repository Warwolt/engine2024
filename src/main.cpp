#include <engine.h>

#include <GL/glew.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <functional>
#include <optional>
#include <stdio.h>
#include <stdlib.h>

using EngineUpdateFn = void(engine::EngineState*);

struct EngineLibrary {
	std::function<EngineUpdateFn> update = [](engine::EngineState*) {};
};

// load engine library
// reload engine library

namespace DebugConfig {
	constexpr bool PRINT_TIMESTAMP_ON_DLL_LOAD = true;
} // namespace DebugConfig

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

std::string get_dll_full_path(HMODULE dll_module) {
	char dll_full_path[MAX_PATH];
	if (GetModuleFileName(dll_module, dll_full_path, sizeof(dll_full_path)) == 0) {
		fprintf(stderr, "error: read_dll_timestamp failed: ");
		print_last_winapi_error();
		return std::string();
	}
	return std::string(dll_full_path);
}

std::optional<FILETIME> file_last_modified(const char* file_path) {
	/* Open file */
	HANDLE file = CreateFileA(
		file_path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	/* Read time modified */
	FILETIME create_time, access_time, write_time;
	if (!GetFileTime(file, &create_time, &access_time, &write_time)) {
		fprintf(stderr, "error: GetFileTime failed: ");
		print_last_winapi_error();
		goto file_last_modified_error;
	}

	FILETIME localized_write_time;
	if (!FileTimeToLocalFileTime(&write_time, &localized_write_time)) {
		fprintf(stderr, "error: FileTimeToLocalFileTime failed: ");
		print_last_winapi_error();
		goto file_last_modified_error;
	}

	CloseHandle(file);
	return localized_write_time;

file_last_modified_error:
	CloseHandle(file);
	return {};
}

std::string filetime_to_string(const FILETIME* filetime) {
	SYSTEMTIME st;
	char buffer[128];
	FileTimeToSystemTime(filetime, &st);
	sprintf_s(buffer, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return std::string(buffer);
}

int main(int /* argc */, char** /* args */) {
	printf("Game Engine 2024 Initializing\n");

	/* Initialize SDL + OpenGL*/
	SDL_Window* window;
	SDL_GLContext gl_context;
	{
		/* Initialize SDL */
		if (SDL_Init(SDL_INIT_VIDEO)) {
			fprintf(stderr, "error: SDL_Init failed: %s\n", SDL_GetError());
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
			fprintf(stderr, "error: SDL_CreateWindow failed: %s\n", SDL_GetError());
			exit(1);
		}

		/* Create GL Context */
		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			fprintf(stderr, "error: SDL_GL_CreateContext failed: %s\n", SDL_GetError());
			exit(1);
		}

		/* Initialize GLEW */
		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			fprintf(stderr, "error: glewInit failed: %s\n", glewGetErrorString(glewError));
			exit(1);
		}

		/* Set VSync */
		if (SDL_GL_SetSwapInterval(1)) {
			fprintf(stderr, "error: SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
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
	const char* copied_dll_name = "GameEngine2024-copy.dll";
	std::string original_dll_path = {};
	std::string copied_dll_path = {};
	HMODULE engine_dll = {};
	FILETIME prev_dll_write_timestamp = {};
	EngineLibrary engine;
	{
		/* Load original DLL */
		const char* original_dll_name = "GameEngine2024.dll";
		HMODULE original_engine_dll = LoadLibrary(original_dll_name);
		if (!original_engine_dll) {
			fprintf(stderr, "error: LoadLibrary(\"%s\") returned null. Does the DLL exist?\n", original_dll_name);
			exit(1);
		}

		/* Get path of original DLL */
		original_dll_path = get_dll_full_path(original_engine_dll);
		if (original_dll_path.empty()) {
			fprintf(stderr, "error: failed to get full path of engine DLL");
			exit(1);
		}
		copied_dll_path = original_dll_path.substr(0, original_dll_path.size() - 4) + "-copy.dll";

		/* Create a copy of the DLL */
		const bool fail_if_already_exists = false;
		if (!CopyFile(original_dll_path.c_str(), copied_dll_path.c_str(), fail_if_already_exists)) {
			fprintf(stderr, "error: CopyFile(\"%s\", \"%s\", %s) failed:", original_dll_path.c_str(), copied_dll_path.c_str(), fail_if_already_exists ? "true" : "false");
			print_last_winapi_error();
			exit(1);
		}

		/* Load copied DLL*/
		engine_dll = LoadLibrary(copied_dll_name);
		if (!engine_dll) {
			fprintf(stderr, "error: LoadLibrary(\"%s\") returned null. Does the DLL exist?\n", copied_dll_name);
			exit(1);
		}

		/* Read last write timestamp */
		std::optional<FILETIME> timestamp = file_last_modified(original_dll_path.c_str());
		if (!timestamp.has_value()) {
			fprintf(stderr, "error: file_last_modified() failed for \"%s\"\n", original_dll_path.c_str());
			exit(1);
		}
		prev_dll_write_timestamp = timestamp.value();

		if (DebugConfig::PRINT_TIMESTAMP_ON_DLL_LOAD) {
			std::string filetime_str = filetime_to_string(&prev_dll_write_timestamp);
			printf("Last file write to %s: %s\n", original_dll_name, filetime_str.c_str());
		}

		/* Read functions */
		{
			const char* fn_name = "engine_update";
			EngineUpdateFn* fn = (EngineUpdateFn*)(GetProcAddress(engine_dll, fn_name));
			if (!fn) {
				fprintf(stderr, "error: GetProcAddress(\"%s\") returned null. Does the function exist?\n", fn_name);
				exit(1);
			}
			engine.update = fn;
		}

		/* Unload original DLL */
		if (!FreeLibrary(original_engine_dll)) {
			fprintf(stderr, "error: FreeLibrary(original_engine_dll) failed: ");
			print_last_winapi_error();
			exit(1);
		}
	}
	printf("Engine DLL loaded\n");

	/* Main loop */
	timing::Timer timer;
	engine::EngineState engine_state;
	bool quit = false;
	while (!quit) {
		/* Hot reloading */
		{
			if (timer.elapsed_ms() >= 1000) {
				timer.reset();

				// try to read timestamp, if we fail try again later
				if (std::optional<FILETIME> dll_write_timestamp = file_last_modified(original_dll_path.c_str())) {
					std::string filetime_str = filetime_to_string(&dll_write_timestamp.value());
					std::string prev_filetime_str = filetime_to_string(&prev_dll_write_timestamp);

					if (CompareFileTime(&dll_write_timestamp.value(), &prev_dll_write_timestamp)) {
						printf("Detected updated engine DLL\n");
						prev_dll_write_timestamp = dll_write_timestamp.value();

						/* Free now old engine DLL copy */
						if (!FreeLibrary(engine_dll)) {
							fprintf(stderr, "error: FreeLibrary(engine_dll) failed: ");
							print_last_winapi_error();
						}

						/* Create a copy of the updated DLL */
						const bool fail_if_already_exists = false;
						if (!CopyFile(original_dll_path.c_str(), copied_dll_path.c_str(), fail_if_already_exists)) {
							fprintf(stderr, "error: CopyFile(\"%s\", \"%s\", %s) failed:", original_dll_path.c_str(), copied_dll_path.c_str(), fail_if_already_exists ? "true" : "false");
							print_last_winapi_error();
							exit(1);
						}

						/* Reload updated DLL copy */
						engine_dll = LoadLibrary(copied_dll_name);
						if (!engine_dll) {
							fprintf(stderr, "error: LoadLibrary(\"%s\") returned null. Does the DLL exist?\n", copied_dll_name);
							exit(1);
						}

						/* Read functions */
						{
							const char* fn_name = "engine_update";
							EngineUpdateFn* fn = (EngineUpdateFn*)(GetProcAddress(engine_dll, fn_name));
							if (!fn) {
								fprintf(stderr, "error: GetProcAddress(\"%s\") returned null. Does the function exist?\n", fn_name);
								exit(1);
							}
							engine.update = fn;
						}

						printf("Engine DLL succesfully reloaded\n");
					}
				}
			}
		}

		/* Input */
		{
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						quit = true;
				}
			}
		}

		/* Update */
		engine.update(&engine_state);

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

	/* Unload and delete copied engine DLL */
	{
		/* Free copied engine DLL */
		if (!FreeLibrary(engine_dll)) {
			fprintf(stderr, "error: FreeLibrary(engine_dll) failed: ");
			print_last_winapi_error();
		}

		/* Delete copied engine DLL */
		if (!DeleteFile(copied_dll_path.c_str())) {
			fprintf(stderr, "error: DeleteFile(\"%s\") failed: ", copied_dll_path.c_str());
			print_last_winapi_error();
		}
	}

	/* Shutdown SDL + OpenGL */
	{
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(shader_program);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	return 0;
}
