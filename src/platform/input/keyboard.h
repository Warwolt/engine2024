#pragma once

#include <optional>
#include <unordered_map>

namespace platform {

	enum class ButtonEvent {
		None,
		Down,
		Up,
	};

	enum KeyboardModifier {
		KEY_MOD_CTRL = 1 << 0,
		KEY_MOD_SHIFT = 1 << 1,
		KEY_MOD_ALT = 1 << 2,
	};

	class Button {
	public:
		void update(ButtonEvent event);

		bool is_pressed() const;
		bool pressed_now() const;
		bool is_released() const;
		bool released_now() const;

	private:
		bool m_pressed = false;
		bool m_changed = false;
	};

	class Keyboard {
	public:
		void register_event(int keycode, ButtonEvent event);
		void register_event_with_modifier(int keycode, ButtonEvent event, int modifier);
		void update();

		bool key_pressed(int keycode) const;
		bool key_pressed_now(int keycode) const;
		bool key_released(int keycode) const;
		bool key_released_now(int keycode) const;

		bool key_pressed_with_modifier(int keycode, int modifier) const;
		bool key_pressed_now_with_modifier(int keycode, int modifier) const;

	private:
		Button _key(int keycode) const;
		int _modifier(int keycode) const;

		std::unordered_map<int, ButtonEvent> m_events;
		std::unordered_map<int, int> m_modifiers;
		std::unordered_map<int, Button> m_keys;
	};

} // namespace platform
