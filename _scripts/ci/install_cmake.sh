#!/bin/bash
if [ -z "$1" ]; then
  CMAKE_VERSION=3.14.7
else
  CMAKE_VERSION="$1"
fi
if [ -z "$2" ]; then
  CMAKE_VERSION_BASE=3.14
else
  CMAKE_VERSION_BASE="$2"
fi
if [ -z "$3" ]; then
  ADDONS_DIR="_addons"
else
  ADDONS_DIR="$3"
fi

if [ "$(uname -s)" = "Darwin" ]; then
  CMAKE_PACKAGE_NAME=cmake-$CMAKE_VERSION-Darwin-x86_64
else
  CMAKE_PACKAGE_NAME=cmake-$CMAKE_VERSION-Linux-x86_64
fi

#download and extract cmake
if ! [ -e "./CMakeLists.txt" ]; then
  cd ..
  if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
  fi
fi
mkdir $ADDONS_DIR && cd ./$ADDONS_DIR

RETRY_COUNT=0
until [ "$RETRY_COUNT" -ge 5 ]
do
  wget "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/$CMAKE_PACKAGE_NAME.tar.gz" \
    || wget "https://cmake.org/files/v$CMAKE_VERSION_BASE/$CMAKE_PACKAGE_NAME.tar.gz"
  tar -zxf $CMAKE_PACKAGE_NAME.tar.gz && break
  
  RETRY_COUNT=$((RETRY_COUNT+1)) 
  rm $CMAKE_PACKAGE_NAME.tar.gz
  sleep 15
done
if [ $RETRY_COUNT = 5 ]; then
  exit 1
fi

mv $CMAKE_PACKAGE_NAME "cmake_${CMAKE_VERSION}"
rm $CMAKE_PACKAGE_NAME.tar.gz
echo "Cmake successfully installed in $(pwd)/cmake_${CMAKE_VERSION}"

#add installation to path to hide preinstalled cmake (commented for CI) :
#PATH=$(pwd)/_addons/cmake_${CMAKE_VERSION}:$(pwd)/_addons/cmake_${CMAKE_VERSION}/bin:$PATH
