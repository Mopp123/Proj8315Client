# Proj8315 Client
Little web based scifi strategy/management game proj

## Building
currently build system works only with linux. Tested with Ubuntu 22.04.5

Only web build is supported atm which requires Emscripten to build wasm modules from the c++ source.
I didn't want to include Emscripten as submodule here since its' so huge and you may want to have it
installed and configured differently anyways.
(https://emscripten.org/docs/getting_started/downloads.html#sdk-download-and-install)

We also use docker to host the site using nginx.

To build the wasm and all necessary stuff you need to activate the Emscripten SDK from it's install location:
`./emsdk activate latest` and activate environment variables `source ./emsdk_env.sh`.
You will need to download assets directory from: https://drive.google.com/drive/folders/1ERCEZnycuS7zqDqi603cqm3qWsqBnJG-
and place this to the repository's root directory.

Run `./build-wasm.sh . build/ site/` from this project's directory.
Set the file's `assets/config.conf` first line to be your Proj8315Server's address:port.
(Make sure you don't have any additional spaces or empty lines)
After this run `./build-assets.sh` to create bundled asset file for emscripten's virtual filesystem.

To create docker image and container you may run `./build-docker.sh`

## Running
After building run `docker compose -f docker-compose.yml start` to start the site.

## Dependancies
* Emscripten
* PortablePesukarhu -engine
* Proj8315Common (commong stuff for both server and client)
