#pragma once

#include <optional>
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
		void register_event(int keycode, ButtonEvent event);
		void update();

		bool key_pressed(int keycode) const;
		bool key_pressed_now(int keycode) const;
		bool key_released(int keycode) const;
		bool key_released_now(int keycode) const;

	private:
		std::optional<ButtonEvent> _get_event(int keycode) const;
		std::optional<Button> _get_key(int keycode) const;

		std::unordered_map<int, ButtonEvent> m_events;
		std::unordered_map<int, Button> m_keys;
	};

} // namespace platform
