cmake_minimum_required (VERSION 3.5)

set (proj_name "Proj8315Client")

project(${proj_name})

# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -fexceptions")


# add_compile_options()
set(USE_FLAGS "-s USE_SDL=2 -s USE_FREETYPE=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXECUTABLE_SUFFIX .html)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(PK_DEBUG_FULL=1)
add_compile_definitions(PK_PLATFORM_WEB=1)

include_directories (
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu"
    "${PROJECT_SOURCE_DIR}/Proj8315Common/src"
)

file (
    GLOB source_files
    # Engine files ---
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/input/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/input/platform/web/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/core/platform/web/*.cpp"

    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/components/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/components/ui/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/components/renderable/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/components/lighting/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/systems/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/systems/ui/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/factories/ui/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/ecs/utils/ui/*.cpp"

    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/shaders/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/renderers/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/platform/web/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/platform/web/shaders/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/platform/opengl/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/graphics/platform/opengl/shaders/*.cpp"

    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/resources/*.cpp"
    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/resources/platform/opengl/*.cpp"

    "${PROJECT_SOURCE_DIR}/PortablePesukarhu/utils/*.cpp"

    # Game Common files ---
    "${PROJECT_SOURCE_DIR}/Proj8315Common/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/Proj8315Common/src/messages/*.cpp"

    # Game client files ---
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/net/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/net/platform/web/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/scenes/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/world/*.cpp"
)

add_executable(${proj_name} ${source_files})

set(html_shell_location "${PROJECT_SOURCE_DIR}/shell.html")

# NOTE: This gets converted to camel case for some reason...
set(html_out "Proj8315Client.html")

# NOTE: This has to be relative from build to assets dir
# NOTE: Currently stopped building the asset thig here and switched to having own script for it!
#   -> This was to be able to update assets without needing to build the code everytime as well
#set(assets_dir "../assets")

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    message(STATUS "___TEST___SYSTEM WAS EMSCRIPTEN!")
    set_target_properties(${proj_name} PROPERTIES LINK_FLAGS "-fexceptions -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 --shell-file ${html_shell_location} -o ${html_out} -sFORCE_FILESYSTEM --use-preload-plugins -sFETCH -sALLOW_MEMORY_GROWTH -l websocket.js")
endif()

