#include <GL/glew.h>
#include <ft2build.h>

#include <core/random.h>
#include <core/unwrap.h>
#include <core/util.h>
#include <library.h>
#include <platform/debug/assert.h>
#include <platform/debug/library_loader.h>
#include <platform/debug/logging.h>
#include <platform/file/config.h>
#include <platform/file/file.h>
#include <platform/file/zip.h>
#include <platform/graphics/font.h>
#include <platform/graphics/gl_context.h>
#include <platform/graphics/image.h>
#include <platform/graphics/renderer.h>
#include <platform/graphics/window.h>
#include <platform/input/cli.h>
#include <platform/input/input.h>
#include <platform/input/keyboard.h>
#include <platform/input/timing.h>
#include <platform/os/imwin32.h>
#include <platform/os/win32.h>
#include <platform/platform_api.h>

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

// prototyping
#include <core/container/vector_map.h>
#include <core/future.h>
#include <core/lerp.h>
#include <nlohmann/json.hpp>
#include <platform/file/resource_manager.h>
#include <platform/file/zip.h>
#include <thread>

// HACK: inline TimelineSystem so we can use it from main without linking
#include <engine/system/timeline_system.cpp>

const char* LIBRARY_NAME = "GameEngine2024Library";

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
	glViewport((window_width - canvas_width) / 2, (window_height - canvas_height) / 2, canvas_width, canvas_height);
}

static void set_normalized_device_coordinate_projection(platform::Renderer* renderer, const platform::ShaderProgram& shader_program) {
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

static void set_imgui_style_win32_like() {
	ImGuiStyle& style = ImGui::GetStyle();

	/* Font */
	// FIXME: This gives a kind of blurry font, maybe we should build the font
	// atlas ourselves and just hand it over to ImGui?
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:/windows/Fonts/tahoma.ttf", 16.0f);

	/* Border sizes */
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.TabBorderSize = 1.0f;
	style.DockingSeparatorSize = 2.0f;

	/* Rounding */
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;

	/* Colors */
	// clang-format off
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.94f, 0.94f, 0.94f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Border]                 = ImVec4(0.42f, 0.42f, 0.42f, 0.38f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.90f, 0.95f, 0.98f, 1.00f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.80f, 0.89f, 0.97f, 1.00f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.94f, 0.94f, 0.94f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.90f, 0.95f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.80f, 0.89f, 0.97f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.90f, 0.95f, 0.98f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.80f, 0.89f, 0.97f, 1.00f);
	colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_TabHovered]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TabActive]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TabUnfocused]           = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_DockingPreview]         = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.58f, 0.76f, 0.91f, 0.57f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	// clang-format on
}

static void init_imgui(SDL_Window* window, SDL_GLContext gl_context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable window docking

	set_imgui_style_win32_like();

	// Workaround for clipboard setup
	// https://github.com/ocornut/imgui/issues/7455#issuecomment-2027855876
	auto defaultSetClipboardTextFn = io.SetClipboardTextFn;
	auto defaultGetClipboardTextFn = io.GetClipboardTextFn;
	auto defaultClipboardUserData = io.ClipboardUserData;

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 450");

	io.SetClipboardTextFn = defaultSetClipboardTextFn;
	io.GetClipboardTextFn = defaultGetClipboardTextFn;
	io.ClipboardUserData = defaultClipboardUserData;
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

static HWND get_window_handle(SDL_Window* window) {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
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

struct ScriptState {
	std::string texture_ids[3];
	std::string captions[3];
	core::Lerped<glm::vec2> positions[3]; // relative center of screen
	glm::vec2 target_positions[3];
	engine::TimelineID timelines[3];
	int index[3];
};

static ScriptState init_script() {
	const glm::vec2 image_size = 2.0f * glm::vec2 { 128, 128 }; // hack variable
	glm::vec2 target_positions[3] = {
		glm::vec2 { -image_size.x / 2.0f, 0.0f },
		glm::vec2 { 0.0f, 0.0f },
		glm::vec2 { image_size.x / 2.0f, 0.0f },
	};
	return ScriptState {
		.texture_ids = {
			"alice",
			"bob",
			"charlie",
		},
		.captions = {
			"Alice",
			"Bob",
			"Charlie",
		},
		.positions = {
			target_positions[0],
			target_positions[1],
			target_positions[2],
		},
		.target_positions = {
			target_positions[0],
			target_positions[1],
			target_positions[2],
		},
		.index = { 0, 1, 2 },
	};
}

static void run_script(
	ScriptState* state,
	engine::TimelineSystem* timeline_system,
	const platform::Input& input
) {
	for (size_t i = 0; i < 3; i++) {
		if (input.keyboard.key_pressed_now(SDLK_LEFT)) {
			state->index[i] = (3 + state->index[i] - 1) % 3;
			state->positions[i].set_target(state->target_positions[state->index[i]]);
			state->timelines[i] = timeline_system->add_one_shot_timeline(input.global_time_ms, 500);
		}

		if (input.keyboard.key_pressed_now(SDLK_RIGHT)) {
			state->index[i] = (3 + state->index[i] + 1) % 3;
			state->positions[i].set_target(state->target_positions[state->index[i]]);
			state->timelines[i] = timeline_system->add_one_shot_timeline(input.global_time_ms, 500);
		}

		float local_time = timeline_system->local_time(state->timelines[i], input.global_time_ms);
		state->positions[i].current = core::lerp(state->positions[i].start, state->positions[i].end, local_time);
	}
}

static void render_script(
	platform::Renderer* renderer,
	const ScriptState& state,
	const platform::Input& input,
	const platform::ResourceManager& resource_manager
) {
	renderer->draw_rect_fill(core::Rect { { 0.0f, 0.0f }, input.window_resolution }, platform::Color::rgba(74, 57, 32, 255)); // clear

	const glm::vec2 window_center = input.window_resolution / 2.0f;
	for (size_t i = 0; i < 3; i++) {
		const platform::Texture& texture = resource_manager.textures().at(state.texture_ids[i]);
		const glm::vec2 image_size = texture.size * 2.0f;
		core::Rect quad = core::Rect::with_center_and_size(window_center + state.positions[i].current, image_size);
		renderer->draw_texture_with_color(texture, quad, glm::vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

int main(int argc, char** argv) {
	/* Parse args */
	platform::CommandLineArgs cmd_args = core::unwrap(platform::parse_arguments(argc, argv), [](std::string error) {
		fprintf(stderr, "parse error: %s\n", error.c_str());
		printf("%s\n", platform::usage_string().c_str());
		exit(1);
	});
	const bool is_editor_mode = cmd_args.start_in_editor_mode;
	platform::RunMode run_mode = is_editor_mode ? platform::RunMode::Editor : platform::RunMode::Game;

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

	/* Load configuration */
	platform::Configuration config;
	const std::string config_name = is_editor_mode ? "Editor.ini" : (platform::application_name() + ".ini");
	const std::filesystem::path config_path = platform::application_path().parent_path() / config_name;
	std::optional<platform::Configuration> loaded_config = platform::load_configuration(config_path);
	if (loaded_config.has_value()) {
		LOG_INFO("Loaded configuration from \"%s\"", config_path.string().c_str());
		config = loaded_config.value();
	}

	// use `loaded_config` so that we only act when a config file has been loaded
	const bool window_should_be_fullscreen = !cmd_args.start_game_windowed && (run_mode == platform::RunMode::Game || config.window.fullscreen);
	const bool window_should_be_maximized = !window_should_be_fullscreen && config.window.maximized;
	const bool window_should_be_positioned = !window_should_be_maximized && !window_should_be_fullscreen;

	/* Create window */
	const glm::ivec2 initial_window_size = { 960, 600 };
	int window_flags = 0;
	if (is_editor_mode) {
		window_flags |= SDL_WINDOW_RESIZABLE;
	}
	if (window_should_be_maximized) {
		window_flags |= SDL_WINDOW_MAXIMIZED;
	}
	platform::Window window = core::unwrap(platform::Window::create(initial_window_size.x, initial_window_size.y, window_flags, "Untitled Project"), [] {
		ABORT("platform::create_window failed");
	});
	if (window_should_be_fullscreen) {
		window.toggle_fullscreen();
	}
	if (window_should_be_positioned) {
		const int win32_menu_bar_height = 32;
		window.set_position(config.window.position + glm::ivec2 { 0, win32_menu_bar_height });
	}

	/* Create OpenGL context */
	SDL_GLContext sdl_gl_context = core::unwrap(platform::create_gl_context(window.sdl_window()), [](platform::CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", core::util::enum_to_string(error));
	});
	platform::OpenGLContext gl_context = platform::OpenGLContext(sdl_gl_context);

	/* Initialize ImGui and ImWin32 */
	init_imgui(window.sdl_window(), sdl_gl_context);
	ImWin32::CreateContext(window.sdl_window());
	std::vector<ImWin32::WindowMessage> win32_window_messages;
	auto on_window_message = [](void* userdata, void* hwnd, unsigned int message, Uint64 w_param, Sint64 l_param) {
		std::vector<ImWin32::WindowMessage>* win32_window_messages = (std::vector<ImWin32::WindowMessage>*)userdata;
		win32_window_messages->push_back(ImWin32::WindowMessage {
			.hwnd = hwnd,
			.message = message,
			.w_param = w_param,
			.l_param = l_param,
		});
	};
	SDL_SetWindowsMessageHook(on_window_message, &win32_window_messages);

	/* Read shader sources */
	const char* vertex_shader_path = "resources/shaders/shader.vert";
	const char* fragment_shader_path = "resources/shaders/shader.frag";
	std::string vertex_shader_src = core::unwrap(platform::read_file_to_string(vertex_shader_path), [&] {
		ABORT("Failed to open vertex shader \"%s\"", vertex_shader_path);
	});
	std::string fragment_shader_src = core::unwrap(platform::read_file_to_string(fragment_shader_path), [&] {
		ABORT("Failed to open fragment shader \"%s\"", fragment_shader_path);
	});

	/* Initialize Renderer */
	platform::Renderer renderer = platform::Renderer(&gl_context);
	platform::ShaderProgram shader_program = core::unwrap(gl_context.add_shader_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](platform::ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", core::util::enum_to_string(error));
	});

	/* Load engine DLL */
	platform::EngineLibraryLoader library_loader;
	platform::EngineLibraryHotReloader hot_reloader = platform::EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	platform::EngineLibrary library = core::unwrap(library_loader.load_library(LIBRARY_NAME), [](platform::LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, core::util::enum_to_string(error));
	});
	platform::on_engine_library_loaded(&library);
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input;
	platform::PlatformAPI platform;

	/* Initialize engine */
	engine::Engine* engine = nullptr;
	{
		platform::Timer init_timer;
		start_imgui_frame(); // this allows engine to initialize imgui state
		engine = library.initialize_engine(&gl_context);
		ImGui::EndFrame();
		LOG_INFO("Engine initialized (after %zu milliseconds)", init_timer.elapsed_ms());
	}

	/* Initialize editor */
	editor::Editor* editor = nullptr;
	if (is_editor_mode) {
		editor = library.initialize_editor(engine, &gl_context, config);
	}

	bool quit = false;
	platform::Canvas window_canvas = gl_context.add_canvas(initial_window_size.x, initial_window_size.y);
	SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

	/* Initialize engine */
	if (run_mode == platform::RunMode::Game) {
		// load game pak
		std::filesystem::path path = std::filesystem::path(platform::application_path()).replace_extension("pak");
		library.load_engine_data(engine, path.string().c_str());
	}

	// GOAL: Represent a scene on disk, so that we can then compose the game out
	// of scenes and store the game on disk by storing its scenes.
	//
	// Each scene consists of a scene graph (node tree) and the resources that
	// those nodes use.
	//
	// To initialize a scene, we need all the resources loaded and to construct
	// the scene graph.
	//
	// TODO:
	// - [] Write a prototype scenario where resources are loaded from disk and a scene graph is construted.
	// 		- [] Load the resources from disk
	// 		- [] Load the resources from a .pak zip archive
	// - [] Update a counter value, then serialize back to disk
	// - (Probably some kind of scenario that includes resources stored both externally on disk and internally in the .pak)

	engine::TimelineSystem timeline_system;
	ScriptState script_state = init_script();

	if (0) {
		/* Read */
		platform::FileArchive archive;
		nlohmann::json payload_json;
		{
			archive = core::unwrap(platform::FileArchive::open_from_file("test.zip"), [](std::string error) {
				ABORT("FileArchive::open_from_file(\"test.zip\") failed with: %s", error.c_str());
			});
			std::vector<uint8_t> payload_bytes = core::unwrap(archive.read_from_archive("data.json"), [](platform::FileArchiveError error) {
				LOG_DEBUG("archive.read_from_archive(\"data.json\") failed with %s", core::util::enum_to_string(error));
			});
			payload_json = nlohmann::json::parse(payload_bytes);
		}

		/* Update */
		struct Payload {
			int counter;
		};
		Payload payload;
		{
			payload = Payload {
				.counter = payload_json["counter"],
			};

			payload.counter += 1;
		}

		/* Write */
		{
			nlohmann::json payload_json2;
			payload_json2["counter"] = payload.counter;
			std::string payload_string2 = payload_json2.dump();
			std::vector<uint8_t> payload_bytes2 = std::vector<uint8_t>(payload_string2.begin(), payload_string2.end());
			archive.write_to_archive("data.json", payload_bytes2.data(), payload_bytes2.size());

			std::expected<void, platform::FileArchiveError> result = archive.write_archive_to_disk("test.zip");
			if (!result.has_value()) {
				LOG_DEBUG("write to archive failed with %s", core::util::enum_to_string(result.error()));
			}
		}
	}

	platform::ResourceManifest scene_manifest = {
		.fonts = {
			{ "arial16", "C:/windows/Fonts/Arial.ttf", 16 },
		},
		.images = {
			{ "alice", "alice.png" },
			{ "bob", "bob.png" },
			{ "charlie", "charlie.png" },
		}
	};
	platform::ResourceManager resource_manager;
	std::shared_ptr<const platform::ResourceLoadProgress> load_scene_progress = resource_manager.load_manifest(scene_manifest);

	/* Main loop */
	while (!quit) {
		/* Input */
		{
			/* ImWin32 */
			ImWin32::NewFrame();
			for (const ImWin32::WindowMessage& msg : win32_window_messages) {
				ImWin32::ProcessWindowMessage(msg);
			}
			win32_window_messages.clear();

			/* Reset input states */
			using ButtonEvent = platform::ButtonEvent;
			constexpr size_t NUM_MOUSE_BUTTONS = 5;
			std::array<ButtonEvent, NUM_MOUSE_BUTTONS> mouse_button_events = { ButtonEvent::None };
			input.mouse.scroll_delta = 0;
			input.mouse.pos_delta = glm::vec2 { 0, 0 };
			input.quit_signal_received = false;
			input.window = &window;

			/* Poll all SDL events */
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL2_ProcessEvent(&event);

				switch (event.type) {
					case SDL_QUIT:
						input.quit_signal_received = true;
						break;

					case SDL_KEYDOWN: {
						int modifiers = 0;
						modifiers |= (event.key.keysym.mod & KMOD_CTRL) ? platform::KEY_MOD_CTRL : 0;
						modifiers |= (event.key.keysym.mod & KMOD_SHIFT) ? platform::KEY_MOD_SHIFT : 0;
						modifiers |= (event.key.keysym.mod & KMOD_ALT) ? platform::KEY_MOD_ALT : 0;
						input.keyboard.register_event_with_modifier(event.key.keysym.sym, ButtonEvent::Down, modifiers);
					} break;

					case SDL_KEYUP:
						// Note: We never let ImGui hog up events to avoid
						// "stuck" keys when switching to an ImGui window
						input.keyboard.register_event(event.key.keysym.sym, ButtonEvent::Up);
						break;

					case SDL_MOUSEBUTTONDOWN:
						if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
							mouse_button_events[event.button.button - 1] = ButtonEvent::Down;
						}
						break;

					case SDL_MOUSEBUTTONUP:
						if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
							mouse_button_events[event.button.button - 1] = ButtonEvent::Up;
						}
						break;

					case SDL_MOUSEWHEEL:
						input.mouse.scroll_delta += event.wheel.y;
						break;

					case SDL_WINDOWEVENT:
						if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
							window.on_resize(event.window.data1, event.window.data2);
						}
						if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
							window.on_maximized();
						}
						if (event.window.event == SDL_WINDOWEVENT_MOVED) {
							window.on_moved(event.window.data1, event.window.data2);
						}
						break;
				}
			}

			// mouse position
			{
				int window_x, window_y = 0;
				SDL_GetWindowPosition(window.sdl_window(), &window_x, &window_y);

				int mouse_x, mouse_y = 0;
				SDL_GetGlobalMouseState(&mouse_x, &mouse_y);

				glm::vec2 new_mouse_pos = glm::vec2 { mouse_x - window_x, mouse_y - window_y };
				input.mouse.pos_delta = new_mouse_pos - input.mouse.pos;
				input.mouse.pos = new_mouse_pos;
			}

			/* Update input states */
			input.keyboard.update();
			input.delta_ms = frame_timer.elapsed_ms();
			input.global_time_ms += input.delta_ms;
			frame_timer.reset();
			input.engine_is_rebuilding = hot_reloader.rebuild_command_is_running();
			input.engine_rebuild_exit_code = hot_reloader.last_exit_code();
			input.window_resolution = window_canvas.texture.size;
			input.mode = run_mode;
			input.mouse.left_button.update(mouse_button_events[SDL_BUTTON_LEFT - 1]);
			input.mouse.middle_button.update(mouse_button_events[SDL_BUTTON_MIDDLE - 1]);
			input.mouse.right_button.update(mouse_button_events[SDL_BUTTON_RIGHT - 1]);
			input.mouse.x1_button.update(mouse_button_events[SDL_BUTTON_X1 - 1]);
			input.mouse.x2_button.update(mouse_button_events[SDL_BUTTON_X2 - 1]);
			input.is_editor_mode = is_editor_mode;
			input.log = &platform::get_in_memory_log();

			SDL_DisplayMode display_mode;
			SDL_GetCurrentDisplayMode(0, &display_mode);
			input.monitor_size.x = (float)display_mode.w;
			input.monitor_size.y = (float)display_mode.h;
		}

		/* Update */
		bool scene_has_loaded = false;
		{
			/* Hot reloading */
			hot_reloader.update(&library);

			/* Engine update */
			start_imgui_frame();
			// DISABLED WHILE PROTOTYPING
			// if (editor) {
			// library.update_editor(editor, config, input, engine, &platform, &gl_context);
			// }
			// library.update_engine(engine, input, &platform, &gl_context);

			/* PROTOTYPE CODE */
			{
				if (input.keyboard.key_pressed(SDLK_ESCAPE) || input.quit_signal_received) {
					quit = true;
				}

				resource_manager.update(&gl_context);
				scene_has_loaded = load_scene_progress->is_done();

				if (scene_has_loaded) {
					run_script(&script_state, &timeline_system, input);
				}
			}

			/* Platform update */
			while (platform.has_commands()) {
				for (platform::PlatformCommand& cmd : platform.drain_commands()) {
					using PlatformCommandType = platform::PlatformCommandType;
					switch (cmd.tag()) {
						case PlatformCommandType::ChangeResolution: {
							auto& [width, height] = std::get<platform::cmd::window::ChangeResolution>(cmd);
							gl_context.free_canvas(window_canvas);
							window_canvas = gl_context.add_canvas(width, height);
						} break;

						case PlatformCommandType::Quit:
							quit = true;
							break;

						case PlatformCommandType::ClearLog:
							platform::clear_in_memory_log();
							break;

						case PlatformCommandType::RebuildEngineLibrary:
							hot_reloader.trigger_rebuild_command();
							break;

						case PlatformCommandType::SetCursor: {
							auto& [new_cursor] = std::get<platform::cmd::cursor::SetCursor>(cmd);
							SDL_FreeCursor(cursor);
							switch (new_cursor) {
								case platform::Cursor::Arrow:
									cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
									break;

								case platform::Cursor::SizeAll:
									cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
									break;
							}
						} break;

						case PlatformCommandType::SetRunMode: {
							auto& [new_run_mode] = std::get<platform::cmd::app::SetRunMode>(cmd);
							run_mode = new_run_mode;
						} break;

						case PlatformCommandType::SetWindowMode: {
							auto& [new_window_mode] = std::get<platform::cmd::window::SetWindowMode>(cmd);
							window.set_window_mode(new_window_mode);
						} break;

						case PlatformCommandType::SetWindowTitle: {
							auto& [window_title] = std::get<platform::cmd::window::SetWindowTitle>(cmd);
							SDL_SetWindowTitle(window.sdl_window(), window_title.c_str());
						} break;

						case PlatformCommandType::ToggleFullscreen:
							window.toggle_fullscreen();
							break;

						case PlatformCommandType::LoadFileWithDialog: {
							auto& [on_file_loaded, dialog] = std::get<platform::cmd::file::LoadFileWithDialog>(cmd);
							HWND hwnd = get_window_handle(window.sdl_window());
							if (std::optional<std::filesystem::path> path = platform::show_load_dialog(hwnd, &dialog)) {
								std::vector<uint8_t> data = read_file_to_string(path.value());
								on_file_loaded(data, path.value());
							}
						} break;

						case PlatformCommandType::SaveFile: {
							auto& [on_file_saved, path, data] = std::get<platform::cmd::file::SaveFile>(cmd);
							std::ofstream file;
							file.open(path);
							if (file.is_open()) {
								file.write((char*)data.data(), data.size());
								on_file_saved();
							}
						} break;

						case PlatformCommandType::SaveFileWithDialog: {
							auto& [on_file_saved, data, dialog] = std::get<platform::cmd::file::SaveFileWithDialog>(cmd);
							HWND hwnd = get_window_handle(window.sdl_window());
							if (std::optional<std::filesystem::path> path = platform::show_save_dialog(hwnd, &dialog)) {
								std::ofstream file;
								file.open(path.value().string().c_str());
								if (file.is_open()) {
									file.write((char*)data.data(), data.size());
									on_file_saved(path.value());
								}
							}
						} break;

						case PlatformCommandType::ShowUnsavedChangesDialog: {
							auto& [on_dialog_choice, document_name] = std::get<platform::cmd::file::ShowUnsavedChangesDialog>(cmd);
							platform::UnsavedChangesDialogChoice choice = platform::show_unsaved_changes_dialog(document_name);
							on_dialog_choice(choice);
						} break;
					}
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
				// PROTOTYPE RENDERING
				{
					if (scene_has_loaded) {
						render_script(&renderer, script_state, input, resource_manager);
					}
					else {
						// loading bar
						float load_progress = (float)(load_scene_progress->num_loaded_resources()) / (float)(load_scene_progress->total_num_resources());

						glm::vec2 window_center = input.window_resolution / 2.0f;
						float loading_bar_max_width = 100.0f;
						float loading_bar_width = load_progress * loading_bar_max_width;
						float loading_bar_height = 10.0f;
						glm::vec2 loading_bar_max_size = { loading_bar_max_width, loading_bar_height };
						glm::vec2 loading_bar_size = { loading_bar_width, loading_bar_height };
						core::Rect fill_rect = core::Rect::with_pos_and_size(
							window_center - loading_bar_max_size / 2.0f,
							loading_bar_size
						);
						core::Rect outline = core::Rect::with_pos_and_size(
							fill_rect.position() - glm::vec2 { 1.0f, 1.0f },
							glm::vec2 { loading_bar_max_width, loading_bar_height } + glm::vec2 { 3.0f, 3.0f }
						);
						renderer.draw_rect_fill(fill_rect, platform::Color::white);
						renderer.draw_rect(outline, platform::Color::white);
					}
				}

				// if (editor && run_mode == platform::RunMode::Editor) {
				// 	library.render_editor(*editor, *engine, &gl_context, &renderer);
				// }
				// else {
				// 	library.render_engine(*engine, &renderer);
				// }

				renderer.set_render_canvas(window_canvas);
				renderer.render(shader_program);
				renderer.reset_render_canvas();

				input.renderer_debug_data = renderer.debug_data();
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
				renderer.draw_texture(window_canvas.texture, core::Rect { { -1.0f, 1.0f }, { 1.0f, -1.0f } });
				renderer.render(shader_program);
			}

			ImWin32::Render();
			render_imgui();
			swap_buffer(window.sdl_window());
		}
	}

	LOG_INFO("Shutting down");

	/* Save configuration */
	config.window.docking_initialized = true;
	config.window.fullscreen = window.is_fullscreen();
	config.window.maximized = window.is_maximized();
	config.window.position = window.last_windowed_position();
	config.window.size = window.size();
	platform::save_configuration(config, config_path);

	/* Deinitialize */
	ImWin32::DestroyContext();
	deinit_imgui();
	library.shutdown_editor(editor, &gl_context);
	library.shutdown_engine(engine, &gl_context);
	gl_context.free_shader_program(shader_program);
	platform::shutdown(sdl_gl_context);
	window.destroy();

	return 0;
}
