Goal:
- Simple level editor
  - Tilemap based environment
  - Place out game objects in environment
  - "Mario maker" style

## TODO
- Render fonts
- Add ImGui
- Add TypeScript for game scripting
  - https://v8.dev/docs/embed

## Doing
- Fixed resolution (integer scaling to fit window)
  - https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/

## Done
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
