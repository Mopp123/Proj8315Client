#/bin/bash

root_dir=`pwd`

# These depends on your setup
emsdk_dir=~/Documents/emsdk
engine_src=`pwd`/PortablePesukarhu
game_src=`pwd`/game
html_shell_location=`pwd`/shell.html
html_out=HelloEmscripten.html
build_dir=/home/kalle/Documents/projects/web-proj/site/public-html

echo "assigned root dir = ${root_dir}"
echo "assigned engine src dir = ${engine_src}"
echo "assigned game src dir = ${game_src}"
echo "assigned html shell location = ${html_shell_location}"
echo "assigned html out = ${html_out}"
echo "assigned build dir = ${build_dir}"

echo "activating emsdk from: ${emsdk_dir}"
cd $emsdk_dir
./emsdk activate latest
source ./emsdk_env.sh

cd $build_dir

echo "starting building to ${build_dir}"

em++ \
${game_src}/Main.cpp \
${game_src}/scenes/LoginMenu.cpp \
${game_src}/scenes/MainMenu.cpp \
${game_src}/net/Client.cpp \
${game_src}/ui/ui.cpp \
${game_src}/net/platform/web/WebClient.cpp \
${game_src}/net/ByteBuffer.cpp \
${engine_src}/core/Debug.cpp \
${engine_src}/core/Timing.cpp \
${engine_src}/core/Window.cpp \
${engine_src}/core/platform/web/WebWindow.cpp \
${engine_src}/graphics/Context.cpp \
${engine_src}/graphics/platform/web/WebContext.cpp \
${engine_src}/core/input/InputManager.cpp \
${engine_src}/core/input/platform/web/WebInputManager.cpp \
${engine_src}/core/Application.cpp \
${engine_src}/core/SceneManager.cpp \
${engine_src}/utils/Algorithms.cpp \
${engine_src}/ecs/components/renderable/TerrainTileRenderable.cpp \
${engine_src}/graphics/platform/web/WebMasterRenderer.cpp \
${engine_src}/graphics/platform/web/WebTexture.cpp \
${engine_src}/graphics/platform/web/WebGUIRenderer.cpp \
${engine_src}/graphics/platform/web/WebFontRenderer.cpp \
${engine_src}/graphics/platform/web/WebTerrainRenderer.cpp \
${engine_src}/graphics/platform/web/WebSpriteRenderer.cpp \
${engine_src}/graphics/platform/web/WebBuffers.cpp \
${engine_src}/graphics/platform/web/shaders/WebShader.cpp \
${engine_src}/ecs/systems/CameraUtils.cpp \
${engine_src}/ecs/systems/animations/Animations.cpp \
${engine_src}/ecs/factories/ui/UIFactories.cpp \
-s FULL_ES2=1 -s WASM=1 --memory-init-file 0 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_FREETYPE=1 --shell-file ${html_shell_location} -o ${html_out} --preload-file assets --use-preload-plugins -sFETCH -sALLOW_MEMORY_GROWTH -l websocket.js


#EXPORTED_FUNCTIONS="[_main, _proxy_main]
#-l websocket.js

