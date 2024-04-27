cmake_minimum_required(VERSION 3.24.2)
project(GameEngine2024)

set(MAIN_BINARY ${CMAKE_PROJECT_NAME})
set(SDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2)
set(GLEW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/glew)

set(INC
    src
    ${GLEW_DIR}/include
    ${SDL_DIR}/include)

set(LIB
    opengl32
    glu32
    ${GLEW_DIR}/lib/glew32.lib
    ${SDL_DIR}/lib/SDL2main.lib
    ${SDL_DIR}/lib/SDL2.lib
    ${SDL_DIR}/lib/SDL2_image.lib
    ${SDL_DIR}/lib/SDL2_mixer.lib
    ${SDL_DIR}/lib/SDL2_ttf.lib)

set(DLLS
    ${GLEW_DIR}/lib/glew32.dll
    ${SDL_DIR}/lib/SDL2.dll
    ${SDL_DIR}/lib/SDL2_image.dll
    ${SDL_DIR}/lib/SDL2_mixer.dll
    ${SDL_DIR}/lib/SDL2_ttf.dll)

add_executable(${MAIN_BINARY} src/main.cpp)
target_include_directories(${MAIN_BINARY} PRIVATE ${INC})
target_link_libraries(${MAIN_BINARY} PRIVATE ${LIB})

# Set compile options for targets
foreach(TARGET IN ITEMS ${MAIN_BINARY})
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
    target_compile_options(${TARGET} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)
endforeach()

# Copy SDL2 dlls
foreach(DLL IN ITEMS ${DLLS})
    add_custom_command(TARGET ${MAIN_BINARY} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        $<TARGET_FILE_DIR:${MAIN_BINARY}>)
endforeach()
