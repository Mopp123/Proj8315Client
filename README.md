# Proj8315 Client
Little web based scifi strategy/management game proj

## Building
Only web build is supported atm which requires Emscripten to build wasm modules from the c++ source.
I didn't want to include Emscripten as submodule here since its' so huge and you may want to have it
installed and configured differently anyways.
(https://emscripten.org/docs/getting_started/downloads.html#sdk-download-and-install)

We also use docker to host the site using nginx.

To build the wasm and all necessary stuff you need to activate the Emscripten SDK from it's install location:
`./emsdk activate latest` and activate environment variables `source ./emsdk_env.sh`.
After this run `./build-wasm.sh . build/ site/` from this project's directory.

To create docker image and container you may run `./build-docker.sh`

## Running
After building run `docker compose -f docker-compose.yml start` to start the site.
NOTE: Currently Client site and Proj8315 Server has to be hosted from same address.

## Dependancies
* Emscripten
* PortablePesukarhu -engine
* Proj8315Common (commong stuff for both server and client)
