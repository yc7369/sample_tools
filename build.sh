#!/bin/bash

# Debug / Release
build_type="$1"

TOP_DIR=`pwd`

if [ ! -n "$1" ] ;then
    build_type="Debug"
fi

if [ $build_type != "Debug" ] && [ $build_type != "Release" ] && [ $build_type != "RelWithDebInfo" ]; then
    echo "=====> Build types Error: $build_type. MUST BE: [Debug | Release | RelWithDebInfo]"
    exit
fi
echo "=====> build types: $build_type"

build_project() {
    echo "build $1"
    PROJECT_DIR="${TOP_DIR}/$1/build"
    if [ ! -d $PROJECT_DIR ]; then
        mkdir $PROJECT_DIR
    fi
    cd $PROJECT_DIR
    # rm -rf ./*
    # Debug / Release
    cmake -DCMAKE_BUILD_TYPE=$build_type ..
    make -j20
}

build_project "deps"
echo "----------> copy deps lib ..."
cd $TOP_DIR/deps
sh build.sh

echo "return to path $TOP_DIR"
cd $TOP_DIR

build_project ""

