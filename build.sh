#!/bin/bash

# eg:
# ./build.sh Debug pull
# ./build.sh Release pull
# ./build.sh Release
# build 脚本只 build

if [ "$2" == "pull" ]; then
    git submodule foreach git pull
fi

mkdir -p ./build
cd ./build

# Debug / Release
build_type="$1"
if [ ! -n "$1" ] ;then
    build_type="Debug"
fi

if [ $build_type != "Debug" ] && [ $build_type != "Release" ] && [ $build_type != "RelWithDebInfo" ]; then
    echo "=====> Build types Error: $build_type. MUST BE: [Debug | Release | RelWithDebInfo]"
    exit
fi
echo "=====> build types: $build_type"

# Debug / Release
cmake -DCMAKE_BUILD_TYPE=$build_type ..
make -j4
