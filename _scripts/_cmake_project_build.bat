@echo off

:: Identify CMake target
if not exist CMakeLists.txt cd ..
if not exist CMakeLists.txt (echo CMakeLists.txt not found in current directory nor in parent directory! && exit /B 1)
if "%1" == "" (echo Missing argument to identify platform (name of platform) && exit /B 1)

:: Create build directory
if not exist ./_build mkdir _build

:: Generate per platform
:loop
if not "%1" == "" (
    cd _build
    if exist "./%1" rmdir /S /Q "%1"
    mkdir "%1"
    cd ..

    if "%1" == "codeblocks" cmake -G "CodeBlocks - MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe
    if "%1" == "codeblocks-cpp14" cmake -G "CodeBlocks - MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe -DCWORK_CPP_REVISION="14"
    if "%1" == "codelite" cmake -G "CodeLite - MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe
    if "%1" == "codelite-cpp14" cmake -G "CodeLite - MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe -DCWORK_CPP_REVISION="14"
    if "%1" == "eclipse" cmake -G "Eclipse CDT4 - MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe
    if "%1" == "eclipse-cpp14" cmake -G "Eclipse CDT4 - MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe -DCWORK_CPP_REVISION="14"
    if "%1" == "icc-make" cmake -G "NMake Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=icl -DCMAKE_CXX_COMPILER=icl
    if "%1" == "mingw-make" cmake -G "MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe
    if "%1" == "mingw-make-cpp14" cmake -G "MinGW Makefiles" -S . -B "./_build/%1" -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe -DCWORK_CPP_REVISION="14"
    if "%1" == "vs2017-32" cmake -G "Visual Studio 15 2017" -A Win32 -S . -B "./_build/%1" -DCWORK_CPP_REVISION="14"
    if "%1" == "vs2017-64" cmake -G "Visual Studio 15 2017" -A x64 -S . -B "./_build/%1" -DCWORK_CPP_REVISION="14"
    if "%1" == "vs2019-32" cmake -G "Visual Studio 16 2019" -A Win32 -S . -B "./_build/%1"
    if "%1" == "vs2019-64" cmake -G "Visual Studio 16 2019" -A x64 -S . -B "./_build/%1"
    if "%1" == "vs2019-clangcl32" cmake -G "Visual Studio 16 2019" -A Win32 -T ClangCL -S . -B "./_build/%1" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_FLAGS="-m32"
    if "%1" == "vs2019-clangcl64" cmake -G "Visual Studio 16 2019" -A x64 -T ClangCL -S . -B "./_build/%1" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_FLAGS="-m64"
    shift
    goto loop
)

exit /B %errorlevel%
