# Source Files

The source files are divided up into three namespaces and `main.cpp`:

| Name     | Description                                                       |
| -------- | ----------------------------------------------------------------- |
| core     | Data structure and algorithms to complement the standard library. |
| engine   | The game engine and the associated editor.                        |
| platform | Interface with the operating system and the hardware.             |

The engine is compiled as a DLL file, to allow for hot reloading.

The `main.cpp` file acts as the host for the engine, loads its DLL, and
implements the `platform::PlatformAPI` interface by delegating to SDL2 or Win32.
