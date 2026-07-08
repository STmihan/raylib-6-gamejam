cmake_minimum_required(VERSION 3.16)
project(raylib-game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SCREEN_WIDTH  720 CACHE STRING "Window width")
set(SCREEN_HEIGHT 720 CACHE STRING "Window height")

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
endif()

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h")

add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SCREEN_WIDTH=${SCREEN_WIDTH}
    SCREEN_HEIGHT=${SCREEN_HEIGHT}
)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sUSE_GLFW=3
        -sASYNCIFY
        -sALLOW_MEMORY_GROWTH=1
        --shell-file "${CMAKE_SOURCE_DIR}/src/shell.html"
        "--preload-file=${ASSETS_DIR}@assets"
    )
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets next to the executable")

    if(APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    endif()
endif()
