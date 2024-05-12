#include <platform/input/keyboard.h>

namespace util {

	template <typename K, typename V>
	std::optional<V> map_get(const std::unordered_map<K, V>& map, const K& key) {
		auto it = map.find(key);
		return it == map.cend() ? std::nullopt : std::make_optional(it->second);
	}

}

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

	void Keyboard::update() {
		/* Add new keys */
		for (auto& [keycode, _] : m_events) {
			m_keys.insert({ keycode, Button() });
		}

		/* Process events */
		for (auto& [keycode, button] : m_keys) {
			ButtonEvent event = util::map_get(m_events, keycode).value_or(ButtonEvent::None);
			button.update(event);
		}
	}

	bool Keyboard::key_pressed(int keycode) const {
		return _key(keycode).is_pressed();
	}

	bool Keyboard::key_pressed_now(int keycode) const {
		return _key(keycode).pressed_now();
	}

	bool Keyboard::key_released(int keycode) const {
		return _key(keycode).is_released();
	}

	bool Keyboard::key_released_now(int keycode) const {
		return _key(keycode).released_now();
	}

	Button Keyboard::_key(int keycode) const {
		return util::map_get(m_keys, keycode).value_or(Button());
	}

} // namespace platform
