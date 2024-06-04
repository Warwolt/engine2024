Goals:
- Tetris
  - Implement simple game
  - Establish platform + engine + game relation
- Simple level editor
  - Tilemap based environment
  - Place out game objects in environment
  - "Mario maker" style

## TODO
- Fix so that engine unit tests links against DLL statically
- Render level editor canvas (Aseprite style grey checker board)
- Add TypeScript for game scripting
  - https://v8.dev/docs/embed

## Doing
- Tidy up window animation

## Done
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
