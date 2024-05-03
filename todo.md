## TODO
- Render textures
- Add ImGui
- Render API commands (draw line, shapes etc. as data that doesn't depend on OpenGL)
- Add TypeScript (maybe)
  - https://v8.dev/docs/embed
- Allow data to change between hot reloading (Serialize state before DLL reload, deserialize after)
  - Tried this, would be too error prone to update serialization definition manually

## Doing

## Done
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
