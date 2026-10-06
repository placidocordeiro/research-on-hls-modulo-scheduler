#!/bin/bash
set -e

mkdir -p build
cmake -S . -B build "$@"
cmake --build build

echo ""
echo "Build pronto. Binarios em build/"
echo "  ./build/ms-mii <instancia.json>"
echo "  cmake --build build --target test-mii"
