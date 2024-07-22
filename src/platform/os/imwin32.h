#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

struct SDL_Window;

namespace ImWin32 {

	struct ImWin32Context;

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

	void CreateContext(SDL_Window* window);
	void DestroyContext();
	void SetCurrentContext(ImWin32Context* context);
	ImWin32Context* GetCurrentContext();
	void ProcessWindowMessage(WindowMessage msg);

	bool BeginMainMenuBar();
	void EndMainMenuBar();
	bool BeginMenu(const std::wstring& label);
	void EndMenu();
	bool MenuItem(const std::wstring& label, bool enabled = true);

	void Separator();

	void NewFrame();
	void Render();

} // namespace ImWin32
