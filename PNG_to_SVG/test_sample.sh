#!/bin/bash

readonly BASE_DIR=$(dirname "$(readlink -f "$0")")

readonly BUILD_DIR="$BASE_DIR/build"
readonly EXE="png2svg"
readonly EXE_PATH="$BUILD_DIR/$EXE"

cmake -B $BUILD_DIR
if [[ -d $BUILD_DIR ]]; then
    cmake --build $BUILD_DIR
    if [[ -f $EXE_PATH ]]; then
        echo ""
        cd $BUILD_DIR
        ./$EXE "--example"
    fi
fi
