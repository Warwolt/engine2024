#include <GL/glew.h>
#include <ft2build.h>

#include <engine.h>
#include <platform/assert.h>
#include <platform/commands.h>
#include <platform/file.h>
#include <platform/image.h>
#include <platform/input/input.h>
#include <platform/input/timing.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/platform.h>
#include <platform/renderer.h>
#include <util.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>
#include <stb_image_write/stb_image_write.h>

#include <expected>
#include <optional>

using Canvas = platform::Canvas;
using CommandAPI = platform::CommandAPI;
using Command = platform::Command;
using CreateGLContextError = platform::CreateGLContextError;
using EngineLibrary = platform::EngineLibrary;
using EngineLibraryHotReloader = platform::EngineLibraryHotReloader;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;

struct FullscreenState {
	bool is_fullscreen = false;
	int last_windowed_x = 0;
	int last_windowed_y = 0;
};

struct Glyph {
	glm::ivec2 atlas_pos;
	glm::ivec2 size;
	glm::ivec2 bearing;
	int advance;
};

const char* LIBRARY_NAME = "GameEngine2024";

void set_viewport(GLuint x, GLuint y, GLsizei width, GLsizei height) {
	glViewport(x, y, width, height);
}

void set_viewport_to_fit_canvas(int window_width, int window_height, int canvas_width, int canvas_height) {
	int scale = (int)std::max(std::round(window_width / canvas_width), std::round(window_height / canvas_height));
	glm::ivec2 window_size = { window_width, window_height };
	glm::ivec2 scaled_canvas_size = { scale * canvas_width, scale * canvas_height };
	glm::ivec2 top_left = (window_size - scaled_canvas_size) / 2;
	glViewport(top_left.x, top_left.y, scaled_canvas_size.x, scaled_canvas_size.y);
}

void set_pixel_coordinate_projection(Renderer* renderer, ShaderProgram shader_program, int width, int height) {
	float grid_offset = 0.375f; // used to avoid missing pixels
	glm::mat4 projection = glm::ortho(grid_offset, grid_offset + width, grid_offset + height, grid_offset, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

void set_normalized_device_coordinate_projection(Renderer* renderer, ShaderProgram shader_program) {
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

void toggle_fullscreen(FullscreenState* state, SDL_Window* window, glm::ivec2* resolution, glm::ivec2* window_size) {
	int display_index = SDL_GetWindowDisplayIndex(window);

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(display_index, &display_mode);

	SDL_Rect display_bound;
	SDL_GetDisplayBounds(display_index, &display_bound);

	if (state->is_fullscreen) {
		state->is_fullscreen = false;

		/* Toggle windowed */
		SDL_SetWindowBordered(window, SDL_TRUE);
		SDL_SetWindowPosition(window, state->last_windowed_x, state->last_windowed_y);
		SDL_SetWindowSize(window, resolution->x, resolution->y);

		/* Update window size */
		*window_size = *resolution;
	}
	else {
		state->is_fullscreen = true;

		/* Save current windowed position */
		SDL_GetWindowPosition(window, &state->last_windowed_x, &state->last_windowed_y);

		/* Toggle fullscreen */
		SDL_SetWindowBordered(window, SDL_FALSE);
		SDL_SetWindowPosition(window, display_bound.x, display_bound.y);
		SDL_SetWindowSize(window, display_mode.w, display_mode.h);

		/* Update windowed size */
		*window_size = glm::ivec2 { display_mode.w, display_mode.h };
	}
}

void init_imgui(SDL_Window* window, SDL_GLContext gl_context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 450");
}

void clear_screen() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void render_imgui() {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void swap_buffer(SDL_Window* window) {
	SDL_GL_SwapWindow(window);
}

void deinit_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void start_imgui_frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

int main(int /* argc */, char** /* args */) {
	glm::ivec2 resolution = { 800, 600 };

	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	SDL_Window* window = platform::create_window(resolution.x, resolution.y);
	ASSERT(window, "platform::create_window() returned null");

	/* Create OpenGL context */
	SDL_GLContext gl_context = util::unwrap(platform::create_gl_context(window), [](CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", util::enum_to_string(error));
	});
	init_imgui(window, gl_context);

	/* Initialize FreeType */
	FT_Library ft;
	if (FT_Error error = FT_Init_FreeType(&ft); error != FT_Err_Ok) {
		ABORT("FT_Init_FreeType failed: %s", FT_Error_String(error));
	}

	FT_Face face;
	const char* font_path = "C:/windows/Fonts/Arial.ttf";
	if (FT_Error error = FT_New_Face(ft, font_path, 0, &face); error != FT_Err_Ok) {
		ABORT("FT_New_Face(\"%s\") failed: %s", font_path, FT_Error_String(error));
	}
	int font_size = 48;
	FT_Set_Char_Size(face, 0, font_size * 64, 96, 96);

	// generate texture atlas
	constexpr int NUM_GLYPHS = 120;
	Glyph glyphs[NUM_GLYPHS];
	// https://gist.github.com/baines/b0f9e4be04ba4e6f56cab82eef5008ff
	{
		// quick and dirty max texture size estimate
		unsigned int max_dim = (1 + (face->size->metrics.height / 64)) * (unsigned int)ceilf(sqrtf(NUM_GLYPHS));
		unsigned int tex_width = 1;
		while (tex_width < max_dim) {
			tex_width *= 2;
		}
		unsigned int tex_height = tex_width;
		LOG_DEBUG("(tex_width, tex_height) = (%d, %d)", tex_width, tex_height);

		uint8_t* pixels = (uint8_t*)calloc(tex_width * tex_height, sizeof(uint8_t));
		glm::ivec2 pen = { 0, 0 };

		for (int i = 0; i < NUM_GLYPHS; i++) {
			// load character
			FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
			FT_Bitmap* bmp = &face->glyph->bitmap;

			// move pen forward
			if (pen.x + bmp->width >= tex_width) {
				pen.x = 0;
				pen.y += face->size->metrics.height / 64 + 1;
			}

			// render current glyph
			for (unsigned int row = 0; row < bmp->rows; row++) {
				for (unsigned int col = 0; col < bmp->width; col++) {
					unsigned int x = pen.x + col;
					unsigned int y = pen.y + row;
					pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
				}
			}

			// save glyph info
			glyphs[i].atlas_pos = pen;
			glyphs[i].size = { bmp->width, bmp->rows };
			glyphs[i].bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
			glyphs[i].advance = face->glyph->advance.x / 64;

			// move pen
			pen.x += bmp->width + 1;
		}

		// write png
		uint8_t* png_data = (uint8_t*)calloc(tex_width * tex_height * 4, sizeof(uint8_t));
		for (unsigned int i = 0; i < (tex_width * tex_height); ++i) {
			png_data[i * 4 + 0] |= pixels[i];
			png_data[i * 4 + 1] |= pixels[i];
			png_data[i * 4 + 2] |= pixels[i];
			png_data[i * 4 + 3] = 0xFF;
		}

		stbi_write_png("font_output.png", tex_width, tex_height, 4, png_data, tex_width * 4);

		free(png_data);
		free(pixels);
	}

	/* Read shader sources */
	const char* vertex_shader_path = "resources/shaders/shader.vert";
	const char* fragment_shader_path = "resources/shaders/shader.frag";
	std::string vertex_shader_src = util::unwrap(platform::read_file(vertex_shader_path), [&] {
		ABORT("Failed to open vertex shader \"%s\"", vertex_shader_path);
	});
	std::string fragment_shader_src = util::unwrap(platform::read_file(fragment_shader_path), [&] {
		ABORT("Failed to open fragment shader \"%s\"", fragment_shader_path);
	});

	/* Initialize Renderer */
	Renderer renderer = Renderer(gl_context, resolution.x, resolution.y);
	ShaderProgram shader_program = util::unwrap(platform::add_shader_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", util::enum_to_string(error));
	});

	/* Load engine DLL */
	EngineLibraryLoader library_loader;
	EngineLibraryHotReloader hot_reloader = EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, util::enum_to_string(error));
	});
	engine.set_logger(plog::verbose, plog::get());
	engine.set_imgui_context(ImGui::GetCurrentContext());
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input;
	platform::CommandAPI commands;
	engine::State state;

	bool quit = false;
	glm::ivec2 window_size = { resolution.x, resolution.y };
	Canvas canvas = platform::add_canvas(resolution.x, resolution.y);
	FullscreenState fullscreen_state;

	engine.initialize(&state);
	while (!quit) {
		start_imgui_frame();

		/* Hot reloading */
		hot_reloader.check_hot_reloading(&engine);

		/* Input */
		platform::read_input(&input, &frame_timer, window_size, resolution);

		/* Update */
		{
			engine.update(&state, &input, &commands);
			for (const Command& command : commands.commands()) {
				switch (command) {
					case Command::Quit:
						quit = true;
						break;
					case Command::ToggleFullscreen:
						toggle_fullscreen(&fullscreen_state, window, &resolution, &window_size);
						break;
				}
			}
			commands.clear();
		}

		/* Render */
		{
			clear_screen();
			{
				// Render to canvas
				set_viewport(0, 0, resolution.x, resolution.y);
				set_pixel_coordinate_projection(&renderer, shader_program, resolution.x, resolution.y);
				engine.render(&renderer, &state);
				renderer.render_to_canvas(shader_program, canvas);
			}
			{
				// Render canvas to window
				set_viewport_to_fit_canvas(window_size.x, window_size.y, resolution.x, resolution.y);
				set_normalized_device_coordinate_projection(&renderer, shader_program);
				renderer.draw_texture({ -1.0f, 1.0f }, { 1.0f, -1.0f }, canvas.texture);
				renderer.render(shader_program);
			}
			render_imgui();
			swap_buffer(window);
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	deinit_imgui();
	engine.deinitialize(&state);
	platform::free_shader_program(shader_program);
	platform::deinitialize(window, gl_context);
	return 0;
}
