#include <GL/glew.h>
#include <ft2build.h>

#include <core/container.h>
#include <core/util.h>
#include <engine/engine_api.h>
#include <platform/assert.h>
#include <platform/cli.h>
#include <platform/file.h>
#include <platform/font.h>
#include <platform/image.h>
#include <platform/input/input.h>
#include <platform/input/keyboard.h>
#include <platform/input/timing.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/platform.h>
#include <platform/platform_api.h>
#include <platform/renderer.h>
#include <platform/win32.h>
#include <platform/window.h>
#include <platform/zip.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>
#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>

#include <fstream>

const char* LIBRARY_NAME = "GameEngine2024Engine";

static void set_viewport(GLuint x, GLuint y, GLsizei width, GLsizei height) {
	glViewport(x, y, width, height);
}

static void set_viewport_to_stretch_canvas(int window_width, int window_height, int canvas_width, int canvas_height) {
	int scale = (int)std::min(std::floor((float)window_width / (float)canvas_width), std::floor((float)window_height / (float)canvas_height));
	glm::ivec2 window_size = { window_width, window_height };
	glm::ivec2 scaled_canvas_size = { scale * canvas_width, scale * canvas_height };
	glm::ivec2 top_left = (window_size - scaled_canvas_size) / 2;
	glViewport(top_left.x, top_left.y, scaled_canvas_size.x, scaled_canvas_size.y);
	ASSERT(scaled_canvas_size.x <= window_width, "canvas width %d scaled larger than window width %d!", scaled_canvas_size.x, window_width);
	ASSERT(scaled_canvas_size.y <= window_height, "canvas height %d scaled larger than window height %d!", scaled_canvas_size.y, window_height);
}

static void set_viewport_to_center_canvas(int window_width, int window_height, int canvas_width, int canvas_height) {
	set_viewport((window_width - canvas_width) / 2, (window_height - canvas_height) / 2, canvas_width, canvas_height);
}

static void set_pixel_coordinate_projection(platform::Renderer* renderer, platform::ShaderProgram shader_program, int width, int height) {
	float grid_offset = 0.375f; // used to avoid missing pixels
	glm::mat4 projection = glm::ortho(grid_offset, grid_offset + width, grid_offset + height, grid_offset, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

static void set_normalized_device_coordinate_projection(platform::Renderer* renderer, platform::ShaderProgram shader_program) {
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

static void init_imgui(SDL_Window* window, SDL_GLContext gl_context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 450");
}

static void clear_screen() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void render_imgui() {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void swap_buffer(SDL_Window* window) {
	SDL_GL_SwapWindow(window);
}

static void deinit_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

static void start_imgui_frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

static HWND get_window_handle(const platform::Window* window) {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window->sdl_window(), &wmInfo);
	return wmInfo.info.win.window;
}

static std::vector<uint8_t> read_file_to_string(const std::filesystem::path& path) {
	auto length = std::filesystem::file_size(path);
	if (length == 0) {
		return {}; // empty vector
	}
	std::vector<uint8_t> buffer(length);
	std::ifstream file(path, std::ios_base::binary);
	file.read((char*)buffer.data(), length);
	file.close();
	return buffer;
}

int main(int argc, char** argv) {
	/* Parse args */
	platform::CommandLineArgs cmd_args = core::container::unwrap(platform::parse_arguments(argc, argv), [](std::string error) {
		fprintf(stderr, "parse error: %s\n", error.c_str());
		printf("%s\n", platform::usage_string().c_str());
		exit(1);
	});

	if (cmd_args.print_usage) {
		printf("%s\n", platform::usage_string().c_str());
		return 0;
	}

	/* Initialize logging */
	platform::init_logging();
	LOG_INFO("R2K Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	const glm::ivec2 initial_window_size = { 960, 600 };
	platform::Window window = core::container::unwrap(platform::Window::create(initial_window_size.x, initial_window_size.y, SDL_WINDOW_RESIZABLE, "Untitled Project"), [] {
		ABORT("platform::create_window failed");
	});

	/* Create OpenGL context */
	SDL_GLContext gl_context = core::container::unwrap(platform::create_gl_context(window.sdl_window()), [](platform::CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", core::util::enum_to_string(error));
	});
	init_imgui(window.sdl_window(), gl_context);

	/* Read shader sources */
	const char* vertex_shader_path = "resources/shaders/shader.vert";
	const char* fragment_shader_path = "resources/shaders/shader.frag";
	std::string vertex_shader_src = core::container::unwrap(platform::read_file_to_string(vertex_shader_path), [&] {
		ABORT("Failed to open vertex shader \"%s\"", vertex_shader_path);
	});
	std::string fragment_shader_src = core::container::unwrap(platform::read_file_to_string(fragment_shader_path), [&] {
		ABORT("Failed to open fragment shader \"%s\"", fragment_shader_path);
	});

	/* Initialize Renderer */
	platform::Renderer renderer = platform::Renderer(gl_context);
	platform::ShaderProgram shader_program = core::container::unwrap(platform::add_shader_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](platform::ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", core::util::enum_to_string(error));
	});

	/* Load engine DLL */
	platform::EngineLibraryLoader library_loader;
	platform::EngineLibraryHotReloader hot_reloader = platform::EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	platform::EngineLibrary engine = core::container::unwrap(library_loader.load_library(LIBRARY_NAME), [](platform::LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, core::util::enum_to_string(error));
	});
	platform::on_engine_library_loaded(&engine);
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input;
	platform::PlatformAPI platform;
	engine::State state;

	bool quit = false;
	platform::RunMode mode = cmd_args.start_in_editor_mode ? platform::RunMode::Editor : platform::RunMode::Game;
	platform::Canvas window_canvas = platform::add_canvas(initial_window_size.x, initial_window_size.y);
	SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

	/* Initialize engine */
	// load game pak
	if (mode == platform::RunMode::Game) {
		std::filesystem::path path = std::filesystem::path(platform::application_path()).replace_extension("pak");
		if (std::filesystem::is_regular_file(path)) {
			LOG_DEBUG("Loading %s", path.filename().string().c_str());
			std::ifstream pak_file(path);
			if (pak_file.is_open()) {
				std::vector<uint8_t> data = platform::read_file_bytes(path).value();
				state.project = core::container::unwrap(engine::ProjectState::from_json_string(data, path), [&](const std::string& error) {
					ABORT("Could not parse json file \"%s\": %s", path.string().c_str(), error.c_str());
				});
			}
		}
	}

	// Start in full screen if running game
	if (mode == platform::RunMode::Game) {
		// FIXME: Update `Window::create` to allow creating a full screen window
		// and use that instead of setting window mode here, since we get a
		// little flicker now when the game starts.
		window.set_window_mode(platform::WindowMode::FullScreen);
	}

	engine.initialize(&state);

	/* Main loop */
	while (!quit) {
		/* Input */
		{
			/* Reset input states */
			using ButtonEvent = platform::ButtonEvent;
			constexpr size_t NUM_MOUSE_BUTTONS = 5;
			std::array<ButtonEvent, NUM_MOUSE_BUTTONS> mouse_button_events = { ButtonEvent::None };
			input.mouse.scroll_delta = 0;
			input.mouse.pos_delta = glm::vec2 { 0, 0 };
			input.quit_signal_received = false;

			/* Poll all SDL events */
			ImGuiIO& imgui_io = ImGui::GetIO();
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL2_ProcessEvent(&event);

				switch (event.type) {
					case SDL_QUIT:
						input.quit_signal_received = true;
						break;

					case SDL_KEYDOWN:
						if (!imgui_io.WantCaptureKeyboard) {
							int modifiers = 0;
							modifiers |= (event.key.keysym.mod & KMOD_CTRL) ? platform::KEY_MOD_CTRL : 0;
							modifiers |= (event.key.keysym.mod & KMOD_SHIFT) ? platform::KEY_MOD_SHIFT : 0;
							modifiers |= (event.key.keysym.mod & KMOD_ALT) ? platform::KEY_MOD_ALT : 0;
							input.keyboard.register_event_with_modifier(event.key.keysym.sym, ButtonEvent::Down, modifiers);
						}
						break;

					case SDL_KEYUP:
						// Note: We never let ImGui to hog up events to avoid
						// "stuck" keys when switching to an ImGui window
						input.keyboard.register_event(event.key.keysym.sym, ButtonEvent::Up);
						break;

					case SDL_MOUSEMOTION: {
						if (!imgui_io.WantCaptureMouse) {
							glm::vec2 new_mouse_pos = glm::vec2 { event.motion.x, event.motion.y };
							input.mouse.pos_delta = new_mouse_pos - input.mouse.pos;
							input.mouse.pos = new_mouse_pos;
						}
						break;
					}

					case SDL_MOUSEBUTTONDOWN:
						if (!imgui_io.WantCaptureMouse) {
							if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
								mouse_button_events[event.button.button - 1] = ButtonEvent::Down;
							}
							break;
						}

					case SDL_MOUSEBUTTONUP:
						if (!imgui_io.WantCaptureMouse) {
							if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
								mouse_button_events[event.button.button - 1] = ButtonEvent::Up;
							}
						}
						break;

					case SDL_MOUSEWHEEL:
						if (!imgui_io.WantCaptureMouse) {
							input.mouse.scroll_delta += event.wheel.y;
						}
						break;

					case SDL_WINDOWEVENT:
						if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
							window.on_resize(event.window.data1, event.window.data2);
						}
						break;
				}
			}

			/* Update input states */
			input.keyboard.update();
			input.delta_ms = frame_timer.elapsed_ms();
			input.global_time_ms += input.delta_ms;
			frame_timer.reset();
			input.engine_is_rebuilding = hot_reloader.rebuild_command_is_running();
			input.engine_rebuild_exit_code = hot_reloader.last_exit_code();
			input.window_resolution = window_canvas.texture.size;
			input.mode = mode;
			input.mouse.left_button.update(mouse_button_events[SDL_BUTTON_LEFT - 1]);
			input.mouse.middle_button.update(mouse_button_events[SDL_BUTTON_MIDDLE - 1]);
			input.mouse.right_button.update(mouse_button_events[SDL_BUTTON_RIGHT - 1]);
			input.mouse.x1_button.update(mouse_button_events[SDL_BUTTON_X1 - 1]);
			input.mouse.x2_button.update(mouse_button_events[SDL_BUTTON_X2 - 1]);
			input.is_editor_mode = cmd_args.start_in_editor_mode;
		}

		/* Update */
		{
			/* Hot reloading */
			hot_reloader.update(&engine);

			/* Engine update */
			start_imgui_frame();
			engine.update(&state, &input, &platform);

			/* Platform update */
			for (platform::PlatformCommand& cmd : platform.drain_commands()) {
				using PlatformCommandType = platform::PlatformCommandType;
				switch (cmd.tag()) {
					case PlatformCommandType::ChangeResolution: {
						auto& change_resolution = std::get<platform::cmd::window::ChangeResolution>(cmd);
						const int width = change_resolution.width;
						const int height = change_resolution.height;
						platform::free_canvas(window_canvas);
						window_canvas = platform::add_canvas(width, height);
					} break;

					case PlatformCommandType::Quit:
						quit = true;
						break;

					case PlatformCommandType::RebuildEngineLibrary:
						hot_reloader.trigger_rebuild_command();
						break;

					case PlatformCommandType::SetCursor: {
						auto& set_cursor = std::get<platform::cmd::cursor::SetCursor>(cmd);
						SDL_FreeCursor(cursor);
						switch (set_cursor.cursor) {
							case platform::Cursor::Arrow:
								cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
								break;

							case platform::Cursor::SizeAll:
								cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
								break;
						}
					} break;

					case PlatformCommandType::SetRunMode: {
						auto& set_run_mode = std::get<platform::cmd::app::SetRunMode>(cmd);
						mode = set_run_mode.mode;
					} break;

					case PlatformCommandType::SetWindowMode: {
						auto& set_window_mode = std::get<platform::cmd::window::SetWindowMode>(cmd);
						window.set_window_mode(set_window_mode.mode);
					} break;

					case PlatformCommandType::SetWindowTitle: {
						auto& set_window_title = std::get<platform::cmd::window::SetWindowTitle>(cmd);
						SDL_SetWindowTitle(window.sdl_window(), set_window_title.title.c_str());
					} break;

					case PlatformCommandType::ToggleFullscreen:
						window.toggle_fullscreen();
						break;

					case PlatformCommandType::LoadFileWithDialog: {
						auto& load_file_with_dialog = std::get<platform::cmd::file::LoadFileWithDialog>(cmd);
						HWND hwnd = get_window_handle(&window);
						if (std::optional<std::filesystem::path> path = platform::show_load_dialog(hwnd, &load_file_with_dialog.dialog)) {
							std::vector<uint8_t> data = read_file_to_string(path.value());
							load_file_with_dialog.on_file_loaded(data, path.value());
						}
					} break;

					case PlatformCommandType::SaveFile: {
						auto& save_file = std::get<platform::cmd::file::SaveFile>(cmd);
						std::ofstream file;
						file.open(save_file.path);
						if (file.is_open()) {
							file.write((char*)save_file.data.data(), save_file.data.size());
							save_file.on_file_saved();
						}
					} break;

					case PlatformCommandType::SaveFileWithDialog: {
						auto& save_file_with_dialog = std::get<platform::cmd::file::SaveFileWithDialog>(cmd);
						HWND hwnd = get_window_handle(&window);
						if (std::optional<std::filesystem::path> path = platform::show_save_dialog(hwnd, &save_file_with_dialog.dialog)) {
							std::ofstream file;
							file.open(path.value().string().c_str());
							if (file.is_open()) {
								file.write((char*)save_file_with_dialog.data.data(), save_file_with_dialog.data.size());
								save_file_with_dialog.on_file_saved(path.value());
							}
						}
					} break;

					case PlatformCommandType::ShowUnsavedChangesDialog: {
						auto& show_unsaved_changes_dialog = std::get<platform::cmd::file::ShowUnsavedChangesDialog>(cmd);
						platform::UnsavedChangesDialogChoice choice = platform::show_unsaved_changes_dialog(show_unsaved_changes_dialog.document_name);
						show_unsaved_changes_dialog.on_dialog_choice(choice);
					} break;
				}
			}

			/* Set Cursor */
			SDL_SetCursor(cursor);
		}

		/* Render */
		{
			clear_screen();

			/* Render to canvas */
			{
				set_viewport(0, 0, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				set_pixel_coordinate_projection(&renderer, shader_program, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);

				engine.render(&renderer, &state);

				renderer.set_render_canvas(window_canvas);
				renderer.render(shader_program);
				renderer.reset_render_canvas();
			}

			/* Render canvas to window */
			{
				if (window.is_fullscreen() || window.is_maximized()) {
					set_viewport_to_stretch_canvas(window.size().x, window.size().y, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				}
				else {
					set_viewport_to_center_canvas(window.size().x, window.size().y, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				}
				set_normalized_device_coordinate_projection(&renderer, shader_program);
				renderer.draw_texture(window_canvas.texture, platform::Rect { { -1.0f, 1.0f }, { 1.0f, -1.0f } });
				renderer.render(shader_program);
			}

			render_imgui();
			swap_buffer(window.sdl_window());
		}
	}

	deinit_imgui();
	engine.shutdown(&state);
	platform::free_shader_program(shader_program);
	platform::shutdown(gl_context);
	window.destroy();
	return 0;
}
