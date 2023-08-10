#!/bin/bash

# These depends on your setup
emsdk_dir=emsdk

cd $emsdk_dir
./emsdk activate latest
source ./emsdk_env.sh

em++ --clear-cache
em++ --clear-ports
