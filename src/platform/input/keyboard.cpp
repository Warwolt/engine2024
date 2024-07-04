#include <platform/input/keyboard.h>

#include <core/container.h>

namespace platform {

	void Button::update(ButtonEvent event) {
		const bool was_pressed = m_pressed;
		switch (event) {
			case ButtonEvent::Down:
				m_changed = !was_pressed;
				m_pressed = true;
				break;

			case ButtonEvent::Up:
				m_changed = was_pressed;
				m_pressed = false;
				break;

			case ButtonEvent::None:
				m_changed = false;
		}
	}

	bool Button::is_pressed() const {
		return m_pressed;
	}

	bool Button::pressed_now() const {
		return m_pressed && m_changed;
	}

	bool Button::is_released() const {
		return !m_pressed;
	}

	bool Button::released_now() const {
		return !m_pressed && m_changed;
	}

	void Keyboard::register_event(int keycode, ButtonEvent event) {
		m_events[keycode] = event;
	}

	void Keyboard::register_event_with_modifier(int keycode, ButtonEvent event, int modifier) {
		m_events[keycode] = event;
		m_modifiers[keycode] = modifier;
	}

	void Keyboard::update() {
		/* Add new keys */
		for (auto& [keycode, _] : m_events) {
			m_keys.insert({ keycode, Button() });
		}

		/* Process events */
		for (auto& [keycode, button] : m_keys) {
			ButtonEvent event = core::container::map_get(m_events, keycode).value_or(ButtonEvent::None);
			button.update(event);
		}
	}

	bool Keyboard::key_pressed(int keycode) const {
		return key_pressed_with_modifier(keycode, 0);
	}

	bool Keyboard::key_pressed_now(int keycode) const {
		return key_pressed_now_with_modifier(keycode, 0);
	}

	bool Keyboard::key_released(int keycode) const {
		return _key(keycode).is_released();
	}

	bool Keyboard::key_released_now(int keycode) const {
		return _key(keycode).released_now();
	}

	bool Keyboard::key_pressed_with_modifier(int keycode, int modifier) const {
		return _key(keycode).is_pressed() && _modifier(keycode) == modifier;
	}

	bool Keyboard::key_pressed_now_with_modifier(int keycode, int modifier) const {
		return _key(keycode).pressed_now() && _modifier(keycode) == modifier;
	}

	Button Keyboard::_key(int keycode) const {
		return core::container::map_get(m_keys, keycode).value_or(Button());
	}

	int Keyboard::_modifier(int keycode) const {
		return core::container::map_get(m_modifiers, keycode).value_or(0);
	}

} // namespace platform
