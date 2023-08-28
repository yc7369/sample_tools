#!/bin/bash -x
echo "build $CI_REPOSITORY_URL"

git config --global user.email "ci@mail"
git config --global user.name "ci"

source /opt/rh/devtoolset-11/enable

git submodule init
git submodule update

mkdir -p ./build
# cd ./build

echo "=====> build types: Release"

top_dir=`pwd`


top_build_dir="$top_dir/build"
mkdir -p $top_build_dir
cd $top_build_dir

echo "=====> build dir: $top_build_dir"

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j $nproc

ls -l $top_build_dir/../lib

