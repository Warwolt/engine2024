cmake_minimum_required(VERSION 3.24.2)
project(GameEngine2024)

# This option is enabled in CI builds
option(WARNINGS_AS_ERRORS "Compile with warnings as errors" OFF)

set(MAIN_BINARY ${CMAKE_PROJECT_NAME})
set(UNIT_TESTS unit_tests)
set(CORE_LIB ${CMAKE_PROJECT_NAME}Core)
set(EDITOR_LIB ${CMAKE_PROJECT_NAME}Editor)
set(ENGINE_LIB ${CMAKE_PROJECT_NAME}Engine)
set(PLATFORM_LIB ${CMAKE_PROJECT_NAME}Platform)
set(SDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2)
set(GLEW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/glew)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)

set(FT_ENABLE_ERROR_STRINGS TRUE)
add_subdirectory(libs/freetype2)
add_subdirectory(libs/google-test)
add_subdirectory(libs/json)
add_subdirectory(libs/miniz)
add_subdirectory(libs/stb_image)

# Enable better error messages from nlomann json
add_compile_definitions(JSON_DIAGNOSTICS=1)

set(INC
    src
    libs/glm
    libs/plog/include
    libs/magic_enum/include
    libs/json/include
    libs/mini
    ${GLEW_DIR}/include
    ${SDL_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/imgui
)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/imgui/imgui_demo.cpp
    ${IMGUI_DIR}/imgui/imgui_draw.cpp
    ${IMGUI_DIR}/imgui/imgui_tables.cpp
    ${IMGUI_DIR}/imgui/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui/imgui.cpp
    ${IMGUI_DIR}/imgui/misc/cpp/imgui_stdlib.cpp
)

add_compile_definitions(IMGUI_USER_CONFIG=<platform/debug/imconfig.h>)

# Configure Plog to use char instead of wchar for better imgui integration
add_compile_definitions(PLOG_CHAR_IS_UTF8)

set(CORE_SRC
    src/core/parse.cpp
    src/core/string.cpp
    src/core/rect.cpp
)

set(EDITOR_SRC
    src/editor/editor_ui.cpp
    src/editor/editor.cpp
    src/editor/ui/log_window.cpp
    src/editor/ui/main_menu_bar.cpp
    src/editor/ui/scene_view.cpp
    src/editor/ui/scene_window.cpp
)

set(ENGINE_SRC
    src/engine/engine_api.cpp
    src/engine/state/game_state.cpp
    src/engine/state/project_state.cpp
    src/engine/system/animation.cpp
    src/engine/system/hot_reloading.cpp
)

set(PLATFORM_SRC
    src/platform/debug/library_loader.cpp
    src/platform/debug/logging.cpp
    src/platform/file/config.cpp
    src/platform/file/file.cpp
    src/platform/file/zip.cpp
    src/platform/graphics/font.cpp
    src/platform/graphics/image.cpp
    src/platform/graphics/renderer.cpp
    src/platform/graphics/texture.cpp
    src/platform/graphics/window.cpp
    src/platform/input/cli.cpp
    src/platform/input/keyboard.cpp
    src/platform/input/timing.cpp
    src/platform/os/imwin32.cpp
    src/platform/os/win32.cpp
    src/platform/platform_api.cpp
)

set(TEST_SRC
    test/core/rect_tests.cpp
    test/core/resource_handler_tests.cpp
    test/core/signal_tests.cpp
    test/core/tagged_variant_tests.cpp
    test/engine/animation_tests.cpp
    test/platform/imwin32_tests.cpp
    test/platform/keyboard_tests.cpp
    test/platform/zip_tests.cpp
)

set(EXTERNAL_LIBS
    freetype
    glu32
    imgui
    miniz
    opengl32
    stb_image
    Comctl32
    ${GLEW_DIR}/lib/glew32.lib
    ${SDL_DIR}/lib/SDL2main.lib
    ${SDL_DIR}/lib/SDL2.lib
    ${SDL_DIR}/lib/SDL2_image.lib
    ${SDL_DIR}/lib/SDL2_mixer.lib
    ${SDL_DIR}/lib/SDL2_ttf.lib
)

set(DLLS
    ${GLEW_DIR}/lib/glew32.dll
    ${SDL_DIR}/lib/SDL2.dll
    ${SDL_DIR}/lib/SDL2_image.dll
    ${SDL_DIR}/lib/SDL2_mixer.dll
    ${SDL_DIR}/lib/SDL2_ttf.dll
)

# Imgui
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${IMGUI_DIR}/imgui ${SDL_DIR}/include/SDL2 libs/glm src)
target_link_libraries(imgui PUBLIC ${SDL_DIR}/lib/SDL2.lib)

# Core
add_library(${CORE_LIB} STATIC ${CORE_SRC})
target_include_directories(${CORE_LIB} PUBLIC ${INC})
target_link_libraries(${CORE_LIB} PUBLIC ${EXTERNAL_LIBS})

# Platform
add_library(${PLATFORM_LIB} STATIC ${PLATFORM_SRC})
target_include_directories(${PLATFORM_LIB} PUBLIC ${INC})
target_link_libraries(${PLATFORM_LIB} PUBLIC ${EXTERNAL_LIBS})

# Editor
add_library(${EDITOR_LIB} STATIC ${EDITOR_SRC})
target_include_directories(${EDITOR_LIB} PUBLIC ${INC})
target_link_libraries(${EDITOR_LIB} PUBLIC ${EXTERNAL_LIBS})

# Engine
add_library(${ENGINE_LIB} SHARED ${ENGINE_SRC})
target_include_directories(${ENGINE_LIB} PUBLIC ${INC})
target_link_libraries(${ENGINE_LIB} PUBLIC ${PLATFORM_LIB} ${EDITOR_LIB} ${CORE_LIB} ${EXTERNAL_LIBS})
set_target_properties(${ENGINE_LIB} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS YES)

# Main
add_executable(${MAIN_BINARY} src/main.cpp)
target_include_directories(${MAIN_BINARY} PUBLIC ${INC})
target_link_libraries(${MAIN_BINARY} PUBLIC ${PLATFORM_LIB} ${EDITOR_LIB} ${CORE_LIB} ${EXTERNAL_LIBS})

# Tests
add_executable(${UNIT_TESTS} test/main.cpp ${TEST_SRC})
target_include_directories(${UNIT_TESTS} PUBLIC test)
target_link_libraries(${UNIT_TESTS} PUBLIC gtest gmock ${PLATFORM_LIB} ${EDITOR_LIB} ${ENGINE_LIB})

# Set compile options for targets
foreach(TARGET IN ITEMS ${MAIN_BINARY} ${CORE_LIB} ${PLATFORM_LIB} ${EDITOR_LIB} ${ENGINE_LIB} ${UNIT_TESTS})
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 23)
    target_compile_options(${TARGET} PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>: /W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic>)
    if (WARNINGS_AS_ERRORS)
        target_compile_options(${TARGET} PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>: /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Werror>)
    endif()
endforeach()

# Copy DLLs
foreach(DLL IN ITEMS ${DLLS})
    add_custom_command(TARGET ${MAIN_BINARY} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        $<TARGET_FILE_DIR:${MAIN_BINARY}>)
endforeach()

# Windows workaround for gtest
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
endif()

# Configure Visual Studio solution
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MAIN_BINARY}) # Set main binary to be default target for Visual Studio
set_target_properties(${MAIN_BINARY} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${MAIN_BINARY}>) # Set working directory to be same as binary
set_target_properties(${MAIN_BINARY} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS  "--editor") # Set default command line

# Copy resource files
add_custom_target(CopyResourceFiles ALL
    COMMENT "Copying resource files"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${MAIN_BINARY}>/resources)

# Set DLL as dependency for binary so Visual Studio knows to rebuild the DLL
add_dependencies(${MAIN_BINARY} ${EDITOR_LIB} ${ENGINE_LIB} ${PLATFORM_LIB})
