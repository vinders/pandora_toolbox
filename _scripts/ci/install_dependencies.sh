#!/bin/bash
if ! [ -e "./CMakeLists.txt" ]; then
  cd ..
  if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
  fi
fi

if [ -z "$1" ]; then
  PLATFORM="linux"
else
  PLATFORM="$1"
fi

# -- project dependencies --

# gtest + libs directory
if ! [ -e "./_libs/gtest/CMakeLists.txt" ]; then
  git submodule update --init --recursive --remote
fi
cd "./_libs"

# glew
if ! [ -e "./glew/CMakeLists.txt" ]; then
  if ! [ "$PLATFORM" = "android64" ]; then
    if ! [ "$PLATFORM" = "ios" ]; then
      git clone --depth 1 --branch "glew-cmake-2.1.0" https://github.com/Perlmint/glew-cmake glew
    fi
  fi
fi

# glm
if ! [ -e "./glm/CMakeLists.txt" ]; then
  git clone --depth 1 --branch "0.9.9.8" https://github.com/g-truc/glm glm
fi

# glfw3
# if ! [ -e "./glfw3/CMakeLists.txt" ]; then
#   git clone --depth 1 --branch "3.3.2" https://github.com/glfw/glfw glfw3
# fi

exit 0
