#include <platform/os/imwin32.h>

#include <platform/os/lean_mean_windows.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>
#include <WinUser.h>
#include <core/container.h>

namespace ImWin32 {

	static HWND get_window_handle(SDL_Window* window) {
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);
		return wmInfo.info.win.window;
	}

	ImWin32Context* g_im_win32 = nullptr;

	void CreateContext(SDL_Window* window) {
		g_im_win32 = new ImWin32Context();
		g_im_win32->window = window;
	}



	void DestroyContext() {
		delete g_im_win32;
	}

	void SetCurrentContext(ImWin32Context* context) {
		if (context != g_im_win32) {
			DestroyContext();
			g_im_win32 = context;
		}
	}

	ImWin32Context* GetCurrentContext() {
		return g_im_win32;
	}

	void ProcessWindowMessage(WindowMessage msg) {
		ImWin32Context& g = *g_im_win32;
		switch (msg.message) {
			case WM_COMMAND:
				g.interacted_ids.push_back(msg.w_param);

			case WM_QUIT:
				// swallow, we handle quit ourselves

			default:
				DefWindowProc((HWND)msg.hwnd, msg.message, msg.w_param, msg.l_param);
		}
	}

	bool BeginMainMenuBar() {
		return true;
	}

	void EndMainMenuBar() {
	}

	bool BeginMenu(const std::wstring& label) {
		ImWin32Context& g = *g_im_win32;
		g.active_main_menu = label;
		g.main_menus[label]; // create new entry if does not exist
		return true;
	}

	void EndMenu() {
		ImWin32Context& g = *g_im_win32;
		ASSERT(!g.active_main_menu.empty(), "ImWin32::EndMenu() called without a previous ImWin32::BeginMenu() call");
		g.active_main_menu.clear();
	}

	bool MenuItem(const std::wstring& label, bool enabled) {
		ImWin32Context& g = *g_im_win32;
		ASSERT(!g.active_main_menu.empty(), "ImWin32::MenuItem() called without a previous ImWin32::BeginMenu() call");

		const uint64_t id = g.next_item_id;
		unsigned int flags = MF_STRING;
		if (!enabled) {
			flags |= MF_GRAYED;
		}

		g.next_item_id += 1;
		g.main_menus[g.active_main_menu].push_back(MenuItemSpec {
			.flags = flags,
			.id = id,
			.item = label,
		});

		bool is_pressed = core::container::contains(g.interacted_ids, id);
		return is_pressed;
	}

	void Separator() {
		// FIXME: handle cases where we're not in a menu bar
		ImWin32Context& g = *g_im_win32;
		ASSERT(!g.active_main_menu.empty(), "ImWin32::Separator() called without a previous ImWin32::BeginMenu() call");
		g.main_menus[g.active_main_menu].push_back(MenuItemSpec {
			.flags = MF_SEPARATOR,
			.id = 0,
			.item = NULL,
		});
	}

	void NewFrame() {
		ImWin32Context& g = *g_im_win32;
		g.active_main_menu.clear();
		g.main_menus.clear();
		g.next_item_id = 1;
	}

	void Render() {
		ImWin32Context& g = *g_im_win32;
		HWND hwnd = get_window_handle(g.window);

		/* Render main menu bar */
		if (g.main_menus != g.shadow_main_menus) {
			HMENU main_menu_bar = CreateMenu();
			for (const auto& [menu_label, menu_items] : g.main_menus) {
				HMENU menu = CreateMenu();
				for (const MenuItemSpec& item : menu_items) {
					AppendMenuW(menu, item.flags, item.id, item.item.c_str());
				}
				AppendMenuW(main_menu_bar, MF_POPUP, (uint64_t)menu, menu_label.c_str());
			}
			SetMenu(hwnd, main_menu_bar);
			g.shadow_main_menus = g.main_menus;
		}

		g.interacted_ids.clear();
	}

} // namespace ImWin32
