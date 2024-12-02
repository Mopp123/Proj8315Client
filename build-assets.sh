#!/bin/bash

file_packager_dir=../emsdk/upstream/emscripten/tools/file_packager

proj_name="Proj8315Client"
build_dir="build"

$file_packager_dir build/assets.data --preload assets@assets --js-output=build/assets.js --use-preload-plugins && cp $build_dir/assets.js site/assets.js && cp $build_dir/assets.data site/assets.data
