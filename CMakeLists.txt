cmake_minimum_required(VERSION 3.16)
project(SWO LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Link the MSVC runtime statically so the release exe is a single, portable
# binary with no external DLL / VC++ Redist dependency. Set before deps are
# fetched so raylib/glfw/imgui inherit it (avoids runtime-library mismatch).
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

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
        "${CMAKE_SOURCE_DIR}/src/assets/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/audio/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/view/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/debug/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h")

add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})
if (WIN32)
    # Embed the application icon into the .exe (Explorer / taskbar icon).
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/app/platform/app.rc")
endif ()
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE game_core raylib nlohmann_json::nlohmann_json)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)

if (MSVC)
    # Release: GUI subsystem so no console window pops up. Keep main() as the
    # entry point (/ENTRY) instead of switching to WinMain. Debug keeps the
    # console so raylib logs stay visible.
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
            $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
endif ()

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

# Pack the assets/ tree into an embeddable, DEFLATE-compressed C header that is
# compiled into the game binary. The pack is mounted at startup (app::assets)
# and serves all raylib file reads, so the shipped binary is self-contained.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(PACK_HEADER "${CMAKE_BINARY_DIR}/generated/assets_pak.h")
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/*")
add_custom_command(
        OUTPUT "${PACK_HEADER}"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tools/pack_assets.py"
                --assets "${ASSETS_DIR}"
                --out-header "${PACK_HEADER}"
                --out-pak "${CMAKE_BINARY_DIR}/assets.pak"
        DEPENDS "${CMAKE_SOURCE_DIR}/tools/pack_assets.py" ${ASSET_FILES}
        COMMENT "Packing assets -> ${PACK_HEADER}"
        VERBATIM)
add_custom_target(assets_pak DEPENDS "${PACK_HEADER}")
add_dependencies(${PROJECT_NAME} assets_pak)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/generated")

if (EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html" OUTPUT_NAME "index")
    target_link_options(${PROJECT_NAME} PRIVATE
            -sUSE_GLFW=3
            -sASYNCIFY
            -sINITIAL_MEMORY=268435456
            --shell-file "${CMAKE_SOURCE_DIR}/src/shell.html"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${ASSETS_DIR}/icons/ui/icon.png"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icon.png"
            COMMENT "Copying favicon next to index.html")
else ()
    # Debug builds fall back to reading assets/ from disk on a pack miss, so ship
    # the folder next to the exe then. Release is fully embedded -> no copy.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            "$<IF:$<CONFIG:Debug>,copy_directory,echo>"
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
            COMMENT "Copying assets next to the executable (Debug only)")

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

    file(GLOB_RECURSE SHOT_VIEW_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/src/view/*.cpp"
            "${CMAKE_SOURCE_DIR}/src/audio/*.cpp"
            "${CMAKE_SOURCE_DIR}/src/assets/*.cpp")
    list(FILTER SHOT_VIEW_SOURCES EXCLUDE REGEX "render/renderer\\.cpp$")
    file(GLOB_RECURSE SHOT_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tools/scene_shot/*.cpp")
    add_executable(scene_shot ${SHOT_SOURCES} ${SHOT_VIEW_SOURCES})
    add_dependencies(scene_shot assets_pak)
    target_include_directories(scene_shot PRIVATE
            "${CMAKE_SOURCE_DIR}/src"
            "${CMAKE_BINARY_DIR}/generated")
    target_link_libraries(scene_shot PRIVATE game_core raylib nlohmann_json::nlohmann_json)

    add_custom_command(TARGET scene_shot POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:scene_shot>/assets"
            COMMENT "Copying assets next to scene_shot")
endif ()
