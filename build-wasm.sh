#!/bin/bash

# args:
#   Positional:
#       Required: <source>($1), <build destination>($2)
#       Optional: <site media dir>($3)
#           * This copies built .js, .wasm, .data and .html if found
#           from <build destination>($2) into <site media dir>($3)
#           * This is to use a single command to build the required
#           wasm stuff into the "site media target"
#           (we dont want to copy the whole "original build dir" into the site
#           since it contains so much unnecessary stuff for the site itself)

# NOTE: To run this you first need to have done:
#   <path to emsdk>/emsdk activate latest
#   source <path to emsdk>/emsdk_env.sh

root_dir=$( pwd )
echo "Using root dir: $root_dir"

copy_to_site () {
    local ext=$1
    local media_dir=$2
    result=$(find -type f -regex ".*\.\($ext\)" -exec cp -v -t $media_dir {} \;)
    if [ -z "$result" ]
    then
        echo -e "\e[33m\t$ext files not found\e[0m"
    fi
}

if [[ -z "$1" ]] && [[ -z "$2" ]]
then
    echo -e "\e[31mInvalid arguments. Source dir and build destination dir required\e[0m"
else
    src_dir=$root_dir/$1
    build_dir=$root_dir/$2

    echo "Using source dir: $src_dir"
    echo "Using build dir: $build_dir"

    cd $build_dir

    echo "starting building to $build_dir"

    emcmake cmake -S ${src_dir} -B .
    cmake --build .

    if [ ! -z "$3" ]
    then
        media_dir=$root_dir/$3
        copy_to_site "html" $media_dir
        copy_to_site "js" $media_dir
        copy_to_site "wasm" $media_dir
        # Switched to hawing asset build script separately
        #copy_to_site "data" $media_dir
    else
        echo -e "\e[33mWarning! Built files are not copied into site media!\e[0m"
    fi
fi

