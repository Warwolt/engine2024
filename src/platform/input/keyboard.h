#pragma once

#include <optional>
#include <unordered_map>

namespace platform {

	struct Button {
		bool pressed;
		bool changed;

		bool is_pressed() const;
		bool pressed_now() const;
		bool is_released() const;
		bool released_now() const;
	};

	enum class ButtonEvent {
		None,
		Down,
		Up,
	};

	Button update_button(Button button, ButtonEvent event);

	class Keyboard {
	public:
		void register_event(int keycode, ButtonEvent event);
		void update();

		bool key_pressed(int keycode) const;
		bool key_pressed_now(int keycode) const;
		bool key_released(int keycode) const;
		bool key_released_now(int keycode) const;

	private:
		Button _key(int keycode) const;

		std::unordered_map<int, ButtonEvent> m_events;
		std::unordered_map<int, Button> m_keys;
	};

} // namespace platform
