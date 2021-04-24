#!/bin/bash

# Identify Cmake target
if [ -z "$1" ]; then
    echo "Missing argument to identify platform (name of platform)"
    exit 1
fi
if ! [ -e "./CMakeLists.txt" ]; then
    cd ..
    if ! [ -e "./CMakeLists.txt" ]; then
        echo "CMakeLists.txt not found in current directory nor in parent directory!"
        exit 1
    fi
fi

# Create build directory
if ! [ -d "./_build" ]; then
    mkdir _build
fi

# Generate per platform
for arg in "$@"; do
    cd ./_build
    if [ "${arg}" = "android" ]; then
        [ -d "./${arg}-${3}" ] && rm -rf "./${arg}-${3}"
        mkdir "${arg}-${3}"
        if [ -z "$ANDROID_NDK_ROOT" ]; then
            if [ -z "$ANDROID_NDK_HOME" ]; then
                echo "Missing environment variable: ANDROID_NDK_ROOT"
                exit 1
            fi
            ANDROID_NDK_ROOT=$ANDROID_NDK_HOME
        fi
    else
        [ -d "./${arg}" ] && rm -rf "./${arg}"
        mkdir "${arg}"
    fi
    cd ..

    case $arg in
        android)
            cmake -G Ninja -S . -B "./_build/${arg}-${3}" -DCMAKE_BUILD_TYPE=Release -DCWORK_TESTS=OFF \
                  -DANDROID_ABI=$3 \
                  -DANDROID_ARM_NEON=$4 \
                  -DANDROID_PLATFORM="android-${2}" \
                  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
                  -DANDROID_TOOLCHAIN=clang \
                  -DCMAKE_SYSTEM_NAME=Android \
                  -DCMAKE_ANDROID_STANDALONE_TOOLCHAIN=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
                  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
                  -DCMAKE_ANDROID_API=$2 \
                  -DCMAKE_ANDROID_ARCH_ABI=$3 \
                  -DCMAKE_EXE_LINKER_FLAGS="-pie" \
                  -DCMAKE_ANDROID_ARM_NEON=$4 \
                  || exit 1
            break
            ;;
        codeblocks)
            cmake -G "CodeBlocks - Unix Makefiles" -S . -B "./_build/${arg}" || exit 1
            ;;
        codelite)
            cmake -G "CodeLite - Unix Makefiles" -S . -B "./_build/${arg}" || exit 1
            ;;
        eclipse)
            cmake -G "Eclipse CDT4 - Unix Makefiles" -S . -B "./_build/${arg}" || exit 1
            ;;
        ios)
            cmake -G Xcode -S . -B "./_build/${arg}" -DCMAKE_BUILD_TYPE=Release -DCWORK_TESTS=OFF -DCWORK_TOOLS=OFF -DCWORK_CPP_REVISION="14" \
                  -DCMAKE_SYSTEM_NAME=iOS \
                  "-DCMAKE_OSX_ARCHITECTURES=armv7;armv7s;arm64;i386;x86_64" \
                  -DCMAKE_OSX_DEPLOYMENT_TARGET=9.3 \
                  -DCMAKE_INSTALL_PREFIX=`pwd`/_install \
                  -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
                  -DCMAKE_IOS_INSTALL_COMBINED=YES \
                  || exit 1
            ;;
        icc-make)
            cmake -G "Unix Makefiles" -S . -B "./_build/${arg}" -DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icpc || exit 1
            ;;
        unix-make)
            cmake -G "Unix Makefiles" -S . -B "./_build/${arg}" || exit 1
            ;;
        unix-make-cpp14)
            cmake -G "Unix Makefiles" -S . -B "./_build/${arg}" -DCWORK_CPP_REVISION="14" || exit 1
            ;;
        xcode)
            cmake -G Xcode -S . -B "./_build/${arg}" || exit 1
            ;;
    esac
done
exit 0
