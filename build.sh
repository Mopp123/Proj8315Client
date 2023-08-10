#/bin/bash

root_dir=`pwd`

# These depends on your setup
#emsdk_dir=~/Documents/emsdk
emsdk_dir=emsdk
build_dir=${root_dir}/public-html

echo "assigned build dir = ${build_dir}"
echo "activating emsdk from: ${emsdk_dir}"

cd $emsdk_dir
./emsdk activate latest
source ./emsdk_env.sh

cd $build_dir

echo "starting building to ${build_dir}"

emcmake cmake -S ../ -B .
cmake --build .
