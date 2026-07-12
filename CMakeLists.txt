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

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Headless-safe core: logic + rules (no raylib). Shared by the game and the sim harness.
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/ai/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/logic/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/data/*.cpp")
add_library(game_core STATIC ${CORE_SOURCES})
target_include_directories(game_core PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(game_core PRIVATE nlohmann_json::nlohmann_json)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/app/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/audio/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/view/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/debug/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h")

add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE game_core raylib nlohmann_json::nlohmann_json)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)

if (NOT EMSCRIPTEN)
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.92.8-docking
            GIT_SHALLOW TRUE
            SOURCE_SUBDIR _fetch_only_
    )
    FetchContent_MakeAvailable(imgui)

    target_sources(${PROJECT_NAME} PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${raylib_SOURCE_DIR}/src/external/glfw/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_IMGUI)
endif ()

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

if (EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html" OUTPUT_NAME "index")
    target_link_options(${PROJECT_NAME} PRIVATE
            -sUSE_GLFW=3
            -sASYNCIFY
            -sINITIAL_MEMORY=268435456
            --shell-file "${CMAKE_SOURCE_DIR}/src/shell.html"
            "--preload-file=${ASSETS_DIR}@assets"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${ASSETS_DIR}/icons/ui/icon.png"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icon.png"
            COMMENT "Copying favicon next to index.html")
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

    file(GLOB_RECURSE HARNESS_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tools/sim_harness/*.cpp")
    add_executable(sim_harness ${HARNESS_SOURCES})
    target_link_libraries(sim_harness PRIVATE game_core)

    add_custom_command(TARGET sim_harness POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:sim_harness>/assets"
            COMMENT "Copying assets next to the harness")
endif ()
