#include <gtest/gtest.h>

#include <platform/os/imwin32.h>
#include <platform/os/lean_mean_windows.h>

class ImWin32Tests : public testing::Test {
public:
	void SetUp() {
		SDL_Window* window = nullptr;
		ImWin32::CreateContext(window);
	}

	void TearDown() {
		ImWin32::DestroyContext();
	}
};

TEST_F(ImWin32Tests, MenuItem_NoEvents_NotPressed) {
	bool menu_item_pressed = false;
	if (ImWin32::BeginMainMenuBar()) {
		if (ImWin32::BeginMenu(L"Test")) {
			if (ImWin32::MenuItem(L"Item")) {
				menu_item_pressed = true;
			}
			ImWin32::EndMenu();
		}
		ImWin32::EndMainMenuBar();
	}

	EXPECT_FALSE(menu_item_pressed);
}

TEST_F(ImWin32Tests, MenuItem_WmCommandEvent_ItemIsPressed) {
	const uint64_t item_id = 1;
	ImWin32::WindowMessage message = {
		.hwnd = nullptr,
		.message = WM_COMMAND,
		.w_param = item_id,
		.l_param = 0,
	};
	ProcessWindowMessage(message);

	bool menu_item_pressed = false;
	if (ImWin32::BeginMainMenuBar()) {
		if (ImWin32::BeginMenu(L"Test")) {
			if (ImWin32::MenuItem(L"Item")) /* should have id 1 */ {
				menu_item_pressed = true;
			}
			ImWin32::EndMenu();
		}
		ImWin32::EndMainMenuBar();
	}

	EXPECT_TRUE(menu_item_pressed);
}
