cmake_minimum_required(VERSION 3.24.2)
project(GameEngine2024)

option(WARNINGS_AS_ERRORS "Compile with warnings as errors" OFF)

set(MAIN_BINARY ${CMAKE_PROJECT_NAME})
set(ENGINE_LIB ${CMAKE_PROJECT_NAME}Engine)
set(PLATFORM_LIB ${CMAKE_PROJECT_NAME}Platform)
set(SDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2)
set(GLEW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/glew)

set(INC
    src
    libs/glm
    libs/plog/include
    ${GLEW_DIR}/include
    ${SDL_DIR}/include
)

set(ENGINE_SRC
    src/engine.cpp
)

set(PLATFORM_SRC
    src/platform/library_loader.cpp
    src/platform/logging.cpp
    src/platform/platform.cpp
    src/platform/renderer.cpp
    src/platform/timing.cpp
)

set(EXTERNAL_LIBS
    opengl32
    glu32
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

# Platform
add_library(${PLATFORM_LIB} STATIC ${PLATFORM_SRC})
target_include_directories(${PLATFORM_LIB} PRIVATE ${INC})

# Engine
add_library(${ENGINE_LIB} SHARED ${ENGINE_SRC})
set_property(TARGET ${ENGINE_LIB} PROPERTY OUTPUT_NAME ${MAIN_BINARY})
target_include_directories(${ENGINE_LIB} PRIVATE ${INC})
target_link_libraries(${ENGINE_LIB} PRIVATE ${PLATFORM_LIB} ${EXTERNAL_LIBS})

# Main
add_executable(${MAIN_BINARY} src/main.cpp)
target_include_directories(${MAIN_BINARY} PRIVATE ${INC})
target_link_libraries(${MAIN_BINARY} PRIVATE ${PLATFORM_LIB} ${EXTERNAL_LIBS})

# Set compile options for targets
foreach(TARGET IN ITEMS ${MAIN_BINARY} ${PLATFORM_LIB} ${ENGINE_LIB})
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 23)
    target_compile_options(${TARGET} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>: /W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic>)
    if (WARNINGS_AS_ERRORS)
        target_compile_options(${TARGET} PRIVATE
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
