#/bin/bash

root_dir=`pwd`

# These depends on your setup
emsdk_dir=~/Documents/emsdk
engine_src=/home/kalle/Documents/projects/PortablePesukarhu
game_src=`pwd`/game
html_shell_location=`pwd`/shell.html
html_out=HelloEmscripten.html
build_dir=`pwd`/build


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
${game_src}/scenes/CreateFactionMenu.cpp \
${game_src}/scenes/InGame.cpp \
${game_src}/scenes/ChatTest.cpp \
${game_src}/world/World.cpp \
${game_src}/world/Tile.cpp \
${game_src}/net/Client.cpp \
${game_src}/net/ByteBuffer.cpp \
${game_src}/net/requests/Request.cpp \
${game_src}/net/requests/Commands.cpp \
${game_src}/net/requests/platform/web/WebRequest.cpp \
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
${engine_src}/ecs/systems/ui/Text.cpp \
${engine_src}/ecs/systems/ui/GUIImage.cpp \
${engine_src}/ecs/systems/ui/combinedFunctional/Button.cpp \
${engine_src}/ecs/systems/ui/combinedFunctional/InputField.cpp \
${engine_src}/ecs/systems/CameraUtils.cpp \
${engine_src}/ecs/systems/animations/SpriteAnimations.cpp \
-s FULL_ES2=1 -s WASM=1 --memory-init-file 0 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_FREETYPE=1 --shell-file ${html_shell_location} -o ${html_out} --preload-file assets --use-preload-plugins -sFETCH -sALLOW_MEMORY_GROWTH
