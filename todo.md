Goals:
- Tetris
  - Implement simple game
  - Establish platform + engine + game relation
- Simple level editor
  - Tilemap based environment
  - Place out game objects in environment
  - "Mario maker" style

## TODO
- set VS_DEBUGGER_WORKING_DIRECTORY to project root
- Move all state headers into "engine/state" directory
- Setup ImGui docking branch
- Add TypeScript for game scripting
  - https://v8.dev/docs/embed

## Doing
- Add infrastructure for running engine as standalone game

## Done
- Track if project has unsaved changes
- Move signal.h to core
- Move tagged_variant.h to "core"
- Move util.h header into a "core" namespace (i.e. project standard library)
- Add interface for writing and reading zip files
- Rename engine.cpp to engine_api.cpp
- Add projects, load / save game data
- Use std::variant for PlatformAPI commands
- Switch between editor mode / game mode
- Give feedback in window title if compilation failed
- Fix so that engine unit tests links against DLL statically
- Render level editor canvas (Aseprite style grey checker board)
- Fix debugging of engine DLL (Probably needs to copy PDB when copying DLL)
- Tidy up window animation
- Hot reload engine library with F5
- Rename `add_font` to `add_ttf_font`
- Rename `deinitialize` to `shutdown`
- Let ImGui steal keyboard input
- Pick resolution while running
- Let ImGui steal mouse input
- Render fonts
- Resolution aware mouse input
- Add ImGui
- Full screen support (F11)
- Fixed resolution (integer scaling to fit window)
- Resizable window
- Add alpha blending to renderer
- Render API commands (draw line, shapes etc. as data that doesn't depend on OpenGL)
- Render using pixel coordinates instead of normalized coordinates
- Render textures
- Keyboard input
- Add Google Test
- Update CMakeLists to set VS startup project
- Process input into a struct that's passed to engine::update
- Add render function to engine
- Move init stuff into platform
- Add ASSERT macro that logs error, debug breaks, then exits
- Only use warnings as errors in CI build, not local
- Render APIs
- Add logging
- Add DLL based hot reloading
- Add OpenGL
- Add build github action
- Create SDL2 window
- Add clang format
