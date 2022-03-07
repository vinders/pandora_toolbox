# ![Pandora Toolbox](_img/pandora_logo.svg)
[![latest version](https://img.shields.io/github/v/tag/vinders/pandora_toolbox?color=4da36a&label=release)](https://github.com/vinders/pandora_toolbox/releases)
[![license - MIT](_img/badges/license_mit.svg)](LICENSE)
![project - CMake](_img/badges/project_cmake.svg)
![lang - C++17](_img/badges/lang_cpp17.svg)
![tests - Gtest](_img/badges/tests_gtest.svg)
![docs - Doxygen](_img/badges/docs_doxygen.svg)<br>
![Direct3D11](https://img.shields.io/badge/Direct3D_11-supported-6c4.svg) ![Vulkan](https://img.shields.io/badge/Vulkan_1.3-ongoing-28b.svg) ![OpenGL4](https://img.shields.io/badge/OpenGL_4-planned-28b.svg)

**Pandora Toolbox** is a collection of portable C++ libraries useful to create **video/real-time projects**: video processing, video games, emulators, simple AI, screensavers...
The code is meant to be highly efficient and easy to use, while remaining lightweight and highly customizable.
Libraries (video/io/hardware...) can be included separately.

> [Features](./FEATURES.md) - | - [CMake options](./OPTIONS.md) - | - [Examples &amp; tutorials](./_examples/README.md) - | - [Changelog](./CHANGELOG.md) - | - [Issue reporting &amp; contribution](./CONTRIBUTING.md)

![Preview](_examples/_img/02_simple_renderer_2D_maze.png)![Preview2](_examples/_img/03_simple_renderer_3D_cube.jpg)

**Feature overview:**
* Window, renderer, message box, input, filters...
* Display monitor, CPU spec reader, CPU affinity...
* Timers, stopwatch, ordered lock, spin-lock, semaphore...
* Fixed max size vector/string, circular queue, endianness...
* Design patterns, search/sort algorithms, math/string utils...
* File utils, UTF-8/UTF-16 encoders, ini/json, logger, location finder...
* See [detailed list of features and compatibility](./FEATURES.md).

## Build status
![cmake 3.14](_img/badges/build_cmake_3_14.svg)
[![codecov coverage](https://codecov.io/gh/vinders/pandora_toolbox/branch/develop/graph/badge.svg?token=5NQ4BF7QRI)](https://codecov.io/gh/vinders/pandora_toolbox)
![C++17](_img/badges/build_cpp17.svg)
![C++14](_img/badges/build_cpp14.svg)

|  |           system            |  compilers  |  status  |
|--|-----------------------------|-------------|----------|
| ![win](_img/badges/system_win.png) | Windows         <br>![supported](https://img.shields.io/badge/-supported-6c4.svg)  | ![vs2019](https://img.shields.io/badge/visual-2019+-57a.svg) ![clangcl](https://img.shields.io/badge/clang--cl-9.1+-57a.svg)<br>![mingw7](https://img.shields.io/badge/mingw64-8.1+-57a.svg) ![vs2017](https://img.shields.io/badge/visual-2017-99a.svg) | ![master](_img/badges/branch_master.svg) [![appveyor-ci](https://ci.appveyor.com/api/projects/status/38j8o8sc55iosqu6/branch/master?svg=true)](https://ci.appveyor.com/project/vinders/pandora-toolbox/branch/master)<br>![develop](_img/badges/branch_develop.svg) [![appveyor-dev](https://ci.appveyor.com/api/projects/status/38j8o8sc55iosqu6/branch/develop?svg=true)](https://ci.appveyor.com/project/vinders/pandora-toolbox/branch/develop) |
| ![x11](_img/badges/system_x11.png) | Linux/BSD (X11) <br>![ongoing](https://img.shields.io/badge/-ongoing-28b.svg)      | ![gcc7](https://img.shields.io/badge/gcc-7.1+-57a.svg) ![clang7](https://img.shields.io/badge/clang-7.0+-57a.svg)<br>![gcc5](https://img.shields.io/badge/gcc-5.1-99a.svg)          | ![master](_img/badges/branch_master.svg) [![travis-ci](https://travis-ci.com/vinders/pandora_toolbox.svg?branch=master)](https://travis-ci.com/github/vinders/pandora_toolbox/branches)<br>![develop](_img/badges/branch_develop.svg) [![travis](https://travis-ci.com/vinders/pandora_toolbox.svg?branch=develop)](https://travis-ci.com/github/vinders/pandora_toolbox/branches) |
| ![mac](_img/badges/system_mac.png) | Mac OS          <br>![planned](https://img.shields.io/badge/-planned-28b.svg)      | ![clang7](https://img.shields.io/badge/clang-7.0+-57a.svg) | ![same ci](https://img.shields.io/badge/build-see_above-999.svg) |
| ![wln](_img/badges/system_wln.png) | Linux (Wayland) <br>![planned](https://img.shields.io/badge/-planned-28b.svg)      | ![gcc7](https://img.shields.io/badge/gcc-7.1+-57a.svg) ![clang7](https://img.shields.io/badge/clang-7.0+-57a.svg) | ![same ci](https://img.shields.io/badge/build-see_above-999.svg) |

---

## Development

### Building project with CMake

This project can be open with any IDE that can natively handle CMake files, or can be generated with CMake. Some scripts are available to automatically generate the project for common IDEs ("_scripts" directory).
A few options can be used to build the solution when calling CMake (or in the CMake settings of the IDE).

Using CMake: \
**cmake -S [source_dir] -G [generator_name] -A [arch] -B [build_dir] -D[option_name]=[option_value]**

Examples:
> * cmake -S . -G "Visual Studio 16 2019" -A x64 -B _build -DCWORK_TESTS=OFF
> * cmake -S . -G "Unix Makefiles" -B _build -DCWORK_DOCS=ON
> * cmake -S . -G "MinGW Makefiles" -B _build -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe

See [CMake options](./OPTIONS.md).
