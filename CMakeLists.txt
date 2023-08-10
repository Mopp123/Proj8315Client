cmake_minimum_required (VERSION 3.5)

set (proj_name "GameClient")

project(${proj_name})

# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")

# add_compile_options()
set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_FREETYPE=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXECUTABLE_SUFFIX .html)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories (
    "${PROJECT_SOURCE_DIR}/game"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu"
    "${PROJECT_SOURCE_DIR}/Proj8315Common/src"
)

file (
        GLOB source_files
        "${PROJECT_SOURCE_DIR}/Proj8315Common/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/Proj8315Common/src/messages/*.cpp"

        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/input/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/input/platform/web/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/platform/web/*.cpp"

        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/components/renderable/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/systems/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/factories/ui/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/systems/animations/*.cpp"

        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/platform/web/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/platform/web/shaders/*.cpp"
        "${PROJECT_SOURCE_DIR}/PortablePesukarhu/utils/*.cpp"

        "${PROJECT_SOURCE_DIR}/game/*.cpp"
        "${PROJECT_SOURCE_DIR}/game/net/*.cpp"
        "${PROJECT_SOURCE_DIR}/game/net/platform/web/*.cpp"
        "${PROJECT_SOURCE_DIR}/game/ui/*.cpp"
        "${PROJECT_SOURCE_DIR}/game/scenes/*.cpp"
        "${PROJECT_SOURCE_DIR}/game/world/*.cpp"
)

add_executable(${proj_name} ${source_files})

set(html_shell_location "${PROJECT_SOURCE_DIR}/shell.html")
set(html_out "Proj8315.html")
set(build_dir "${PROJECT_SOURCE_DIR}/public-html")
set(assets_dir "assets")

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    message(STATUS "___TEST___SYSTEM WAS EMSCRIPTEN!")
    set_target_properties(${proj_name} PROPERTIES LINK_FLAGS "-s FULL_ES2=1 -s WASM=1 --memory-init-file 0 --shell-file ${html_shell_location} -o ${html_out} --preload-file ${assets_dir} --use-preload-plugins -sFETCH -sALLOW_MEMORY_GROWTH -l websocket.js")
endif()

