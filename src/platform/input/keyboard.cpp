#include <platform/input/keyboard.h>

namespace platform {

	Button update_button(Button button, ButtonEvent event) {
		if (event == ButtonEvent::Down) {
			return Button {
				.pressed = true,
				.changed = !button.pressed
			};
		}
		if (event == ButtonEvent::Up) {
			return Button {
				.pressed = false,
				.changed = button.pressed
			};
		}
		else {
			return Button {
				.pressed = button.pressed,
				.changed = false,
			};
		}
	}

	void Keyboard::register_event(int keycode, ButtonEvent event) {
		m_events[keycode] = event;
	}

	void Keyboard::update() {
		/* Add new keys */
		for (auto& [keycode, _] : m_events) {
			m_keys.insert({ keycode, Button { 0 } });
		}

		/* Process events */
		for (auto& [keycode, state] : m_keys) {
			ButtonEvent event = _get_event(keycode).value_or(ButtonEvent::None);
			state = update_button(state, event);
		}
	}

	bool Keyboard::key_pressed(int keycode) const {
		Button key = _get_key(keycode).value_or(Button { 0 });
		return key.pressed;
	}

	bool Keyboard::key_pressed_now(int keycode) const {
		Button key = _get_key(keycode).value_or(Button { 0 });
		return key.pressed && key.changed;
	}

	bool Keyboard::key_released(int keycode) const {
		Button key = _get_key(keycode).value_or(Button { 0 });
		return !key.pressed;
	}

	bool Keyboard::key_released_now(int keycode) const {
		Button key = _get_key(keycode).value_or(Button { 0 });
		return !key.pressed && key.changed;
	}

	std::optional<ButtonEvent> Keyboard::_get_event(int keycode) const {
		auto it = m_events.find(keycode);
		return it == m_events.cend() ? std::nullopt : std::make_optional(it->second);
	}

	std::optional<Button> Keyboard::_get_key(int keycode) const {
		auto it = m_keys.find(keycode);
		return it == m_keys.cend() ? std::nullopt : std::make_optional(it->second);
	}

} // namespace platform
