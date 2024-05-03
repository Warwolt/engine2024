#include <platform/input/keyboard.h>

namespace util {

	template <typename K, typename V>
	std::optional<V> map_get(const std::unordered_map<K, V>& map, const K& key) {
		auto it = map.find(key);
		return it == map.cend() ? std::nullopt : std::make_optional(it->second);
	}

}

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
			ButtonEvent event = util::map_get(m_events, keycode).value_or(ButtonEvent::None);
			state = update_button(state, event);
		}
	}

	bool Keyboard::key_pressed(int keycode) const {
		Button key = util::map_get(m_keys, keycode).value_or(Button { 0 });
		return key.pressed;
	}

	bool Keyboard::key_pressed_now(int keycode) const {
		Button key = util::map_get(m_keys, keycode).value_or(Button { 0 });
		return key.pressed && key.changed;
	}

	bool Keyboard::key_released(int keycode) const {
		Button key = util::map_get(m_keys, keycode).value_or(Button { 0 });
		return !key.pressed;
	}

	bool Keyboard::key_released_now(int keycode) const {
		Button key = util::map_get(m_keys, keycode).value_or(Button { 0 });
		return !key.pressed && key.changed;
	}

} // namespace platform
