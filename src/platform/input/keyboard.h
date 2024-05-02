#pragma once

#include <unordered_map>

namespace platform {

	struct Button {
		bool pressed;
		bool changed;
	};

	enum class ButtonEvent {
		None,
		Down,
		Up,
	};

	Button update_button(Button button, ButtonEvent event);

	class Keyboard {
	public:
		void register_event(int key, ButtonEvent event);

		bool key_pressed(int key) const;
		bool key_pressed_now(int key) const;

	private:
		std::unordered_map<int, Button> m_keys;
	};

} // namespace platform
