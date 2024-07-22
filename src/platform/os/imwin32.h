#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

struct SDL_Window;

namespace ImWin32 {

	struct WindowMessage {
		void* hwnd;
		unsigned int message;
		uint64_t w_param;
		int64_t l_param;
	};

	struct MenuItemSpec {
		unsigned int flags;
		uint64_t id;
		std::wstring item;

		inline bool operator==(const MenuItemSpec& other) const {
			return flags == other.flags &&
				id == other.id &&
				item == other.item;
		}
	};

	struct ImWin32Context {
		// platform
		SDL_Window* window = nullptr;
		std::vector<uint64_t> interacted_ids;

		// render state
		uint64_t next_item_id = 1;
		std::wstring active_main_menu;
		std::unordered_map<std::wstring, std::vector<MenuItemSpec>> main_menus;
		std::unordered_map<std::wstring, std::vector<MenuItemSpec>> shadow_main_menus;
	};

	void CreateContext(SDL_Window* window);
	void DestroyContext();
	void ProcessWindowMessage(WindowMessage msg);

	bool BeginMainMenuBar();
	void EndMainMenuBar();
	bool BeginMenu(const std::wstring& label);
	void EndMenu();
	bool MenuItem(const std::wstring& label, bool enabled = true);

	void NewFrame();
	void Render();

} // namespace ImWin32
