#pragma once

namespace platform {

	struct Commands {
		void quit() {
			m_quit = true;
		}

		bool m_quit;
	};

} // namespace platform
