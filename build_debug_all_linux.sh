#!/usr/bin/env bash

set -e

TOOLCHAIN_NAME="GCC"
TOOLCHAIN_FILE="cmake/gcc_toolchain.cmake"
SOURCE_DIR="."
BUILD_DIR="build/${TOOLCHAIN_NAME}"
GENERATOR="Ninja"

# rm -rf "${BUILD_DIR}"

cmake \
    -S "${SOURCE_DIR}" \
    -B "${BUILD_DIR}" \
    -G "${GENERATOR}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}"

cmake --build "${BUILD_DIR}" --config Debug
