#!/usr/bin/env sh

rm ./cmake_install.cmake
rm ./CMakeCache.txt
rm -R ./CMakeFiles/
rm -R ./.cache/
rm ./compile_commands.json
rm ./build.ninja
cmake -GNinja ..
