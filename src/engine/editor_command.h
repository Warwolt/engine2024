#pragma once

namespace engine {

	enum class EditorCommand {
		// file
		NewProject,
		OpenProject,
		SaveProject,
		SaveProjectAs,
		// game
		RunGame,
		ResetGameState,
		// app
		Quit,
	};

} // namespace engine
