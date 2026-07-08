cmake_minimum_required(VERSION 3.16)
project(raylib-game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

if (EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
endif ()

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h")

add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if (NOT EMSCRIPTEN)
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.92.8-docking
            GIT_SHALLOW TRUE
    )
    FetchContent_Declare(
            rlimgui
            GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
            GIT_TAG Raylib_6_0
            GIT_SHALLOW TRUE
    )
    FetchContent_GetProperties(imgui)
    if (NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif ()
    FetchContent_GetProperties(rlimgui)
    if (NOT rlimgui_POPULATED)
        FetchContent_Populate(rlimgui)
    endif ()

    target_sources(${PROJECT_NAME} PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR} ${rlimgui_SOURCE_DIR})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_IMGUI)
endif ()

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

if (EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
            -sUSE_GLFW=3
            -sASYNCIFY
            -sALLOW_MEMORY_GROWTH=1
            --shell-file "${CMAKE_SOURCE_DIR}/src/shell.html"
            "--preload-file=${ASSETS_DIR}@assets"
    )
else ()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
            COMMENT "Copying assets next to the executable")

    if (APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE
                "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    endif ()
endif ()
