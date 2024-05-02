## TODO
- Update CMakeLists to set VS startup project
- Keyboard input
- Opaque reloadable engine state
  - Serialize before DLL reload, deserialize after
  - Keep state inside of DLL
- Render textures
- Add ImGui
- Add Google Test
- Render API commands (draw line, shapes etc. as data that doesn't depend on OpenGL)
- Add TypeScript (maybe)
  - https://v8.dev/docs/embed

## Doing
- Process input into a struct that's passed to engine::update

## Done
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
