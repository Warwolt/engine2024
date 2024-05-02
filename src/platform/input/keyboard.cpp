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

} // namespace platform
