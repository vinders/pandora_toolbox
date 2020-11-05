# ![Pandora Toolbox](_img/pandora_logo.svg)
[![latest version](https://img.shields.io/github/v/tag/vinders/pandora_toolbox?color=4da36a&label=release)](https://github.com/vinders/pandora_toolbox/releases)
[![license - MIT](_img/badges/license_mit.svg)](LICENSE)
![project - Cmake](_img/badges/project_cmake.svg)
![lang - C++17](_img/badges/lang_cpp17.svg)
![tests - Gtest](_img/badges/tests_gtest.svg)
![docs - Doxygen](_img/badges/docs_doxygen.svg)

Pandora Toolbox is a collection of portable C++ libraries: general purpose utilities, data containers, common patterns, video filters, timers, concurrency tools... 
The code is meant to be highly efficient and easy to use in real-time and video projects : AI, video processing, video games, emulators or embedded systems. Written in C++ 17, it supports backwards compatibility with C++ 14.

See [Libraries](#libraries) section below for more information. Check changelog [here](./CHANGELOG.md).<br>
Contributors should take a look at the [issue reporting, workflow and coding style](./CONTRIBUTING.md) guidelines.

## Build status
![cmake 3.14](_img/badges/build_cmake_3_14.svg)
[![codecov coverage](https://codecov.io/gh/vinders/pandora_toolbox/branch/develop/graph/badge.svg?token=5NQ4BF7QRI)](https://codecov.io/gh/vinders/pandora_toolbox)
![C++17](_img/badges/build_cpp17.svg)
![C++14](_img/badges/build_cpp14.svg)

|           system            |        compiler        |         status         |
|-----------------------------|------------------------|------------------------|
| Windows <sub>(x86/x86_64)</sub>  | ![vs2019](https://img.shields.io/badge/visual_studio-2019-75b.svg)<br>![vs2017](https://img.shields.io/badge/visual_studio-2017-a99dc2.svg)<br>![mingw7](https://img.shields.io/badge/mingw64-8.1.0-75b.svg)<br>![clangcl](https://img.shields.io/badge/clang--cl-10.0.0-75b.svg) | ![master](_img/badges/branch_master.svg) [![appveyor](https://ci.appveyor.com/api/projects/status/38j8o8sc55iosqu6/branch/master?svg=true)](https://ci.appveyor.com/project/vinders/pandora-toolbox/branch/master)<br>![develop](_img/badges/branch_develop.svg) [![appveyor-dev](https://ci.appveyor.com/api/projects/status/38j8o8sc55iosqu6/branch/develop?svg=true)](https://ci.appveyor.com/project/vinders/pandora-toolbox/branch/develop)  |
| Linux <sub>(x86_64)</sub>        | ![gcc9](https://img.shields.io/badge/gcc-9.1-75b.svg)              | ![master](_img/badges/branch_master.svg) [![Linux-gcc9](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/1)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![Linux-gcc9-dev](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/2)](https://travis-ci.org/vinders/pandora_toolbox)       |
|                                  | ![clang9](https://img.shields.io/badge/clang-9.0.0-75b.svg)<br>![icc](https://img.shields.io/badge/intel_icc-2021.1-75b.svg)<br>![gcc7](https://img.shields.io/badge/gcc-7.1-75b.svg)<br>![gcc5](https://img.shields.io/badge/gcc-5.1-a99dc2.svg) | ![develop](_img/badges/branch_develop.svg) [![Linux-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/1)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![Linux-icc](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/8)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![Linux-gcc7](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/3)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![Linux-gcc5](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/4)](https://travis-ci.org/vinders/pandora_toolbox) |
| Mac OS X <sub>(x86_64)</sub>     | ![clang7](https://img.shields.io/badge/clang-7.0.0-75b.svg)        | ![master](_img/badges/branch_master.svg) [![osx-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/2)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![osx-clang-dev](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/5)](https://travis-ci.org/vinders/pandora_toolbox)         |
| iOS <sub>(arm/arm64)</sub>       | ![clang7](https://img.shields.io/badge/clang-7.0.0-75b.svg)        | ![master](_img/badges/branch_master.svg) [![iOS-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/3)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![iOS-clang-dev](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/6)](https://travis-ci.org/vinders/pandora_toolbox)         |
| Android <sub>(arm64)</sub>       | ![android](https://img.shields.io/badge/android-ndk_26-a99dc2.svg) | ![master](_img/badges/branch_master.svg) [![android64-ndk](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/4)](https://travis-ci.org/vinders/pandora_toolbox)<br>![develop](_img/badges/branch_develop.svg) [![android64-ndk-dev](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/7)](https://travis-ci.org/vinders/pandora_toolbox) |

## Libraries

|    name    |                                                   description                                                        |
|------------|----------------------------------------------------------------------------------------------------------------------|
| *hardware* | CPU specifications reader, instruction sets detection, process affinity, device handlers...                          |
| *io*       | File/directory utils, standard system locations, serialization, encoders, formatters...                              |
| *logic*    | Mathematical algorithms, string manipulation utils, search/sort algorithms, AI routines...                           |
| *memory*   | Fixed size string/vector/circular-queue (stack alloc), endianness utils, preallocated memory pool...                 |
| *pattern*  | Common design patterns (optional, locked, iterator, observer/delegate with contract...), advanced type traits...     |
| *system*   | System specifications (OS, arch...), intrinsics, macros, code generators, logger, debugging tools...                 |
| *thread*   | Thread pool, thread utils, spin-lock, semaphore, ordered lock...                                                     |
| *time*     | Time management utils (stopwatch, timer, rates...), high-precision native system clocks...                           |
| *video*    | Video rendering (shaders, camera, ...), video filtering/upscaling, display management utils...                       |

---

## Development

### Building project with Cmake

This project can be opened with any IDE that can natively handle Cmake files, or can be generated with Cmake. Some scripts are available to automatically generate the project for common IDEs ("_scripts" directory).
A few options can be used to build the solution when calling Cmake (or in the CMake settings of the IDE).

Using Cmake: \
**cmake -S [source_dir] -G [generator_name] -A [arch] -B [build_dir] -D[option_name]=[option_value]**

Examples:
> * cmake -S . -G "Visual Studio 16 2019" -A x64 -B _build -DCWORK_TESTS=OFF
> * cmake -S . -G "Unix Makefiles" -B _build -DCWORK_DOCS=ON
> * cmake -S . -G "MinGW Makefiles" -B _build -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe -DCWORK_TOOLS=ON

### Cmake options

|    Option    |    Name    |    Value(s)    |    Available for...    |
|--------------|------------|----------------|------------------------|
| Retro-compatibility with older<br>C++ revisions (ex: C++14).<br><sub>*Default: "17" if supported, otherwise "14"*</sub>  | CWORK_CPP_REVISION  | "17"<br>"14"  | all C++17-compliant compilers  |
| Build separate symbol files, for debuggers such as GDB.<br><sub>*Default: OFF*</sub>         | CWORK_BUILD_SYMBOL_FILES  | ON<br>OFF     | all systems                   |
| Include unit tests of each library<br>in the built solution.<br><sub>*Default: ON*</sub>     | CWORK_TESTS               | ON<br>OFF     | all except cross-compilation & ARM  |
| Generate code documentation (doxygen) for the built solution.<br><sub>*Default: OFF*</sub>   | CWORK_DOCS                | ON<br>OFF     | all except cross-compilation  |
| Include special tools related to project in the built solution.<br><sub>*Default: ON*</sub>  | CWORK_TOOLS               | ON<br>OFF     | all systems                   |
| Generate debugging headers to allow code coverage.<br><sub>*Default: OFF*</sub>              | CWORK_COVERAGE            | ON<br>OFF     | only linux/unix systems       |
| Minimum supported Windows (API retro-compatibility).<br><sub>*Default: "7"*</sub>            | CWORK_WINDOWS_SUPPORT     | "10" "8"<br>"7" "6"  | only windows systems   |
| Enable OpenGL4/ES3 features.<br><sub>*Default: ON*</sub>                                     | CWORK_VIDEO_OPENGL4       | ON<br>OFF     | all systems                   |
| Enable Direct3D 11 features.<br><sub>*Default: ON if MSVC/clang-cl compiler*</sub>           | CWORK_VIDEO_D3D11         | ON<br>OFF     | only windows systems<br>(MSVC/LLVM required)  |
| Enable Vulkan features.<br><sub>*Default: ON if Vulkan SDK is installed*</sub>               | CWORK_VIDEO_VULKAN        | ON<br>OFF     | all except iOS<br>(environment variable *VULKAN_SDK* required)  |

---

### Contribution guidelines

* [Foreword](./CONTRIBUTING.md#foreword)
* [Reporting Issues](./CONTRIBUTING.md#reporting-issues)
* [Development](./CONTRIBUTING.md#development)
    * [Workflow](./CONTRIBUTING.md#workflow)
    * [Commit Guidelines](./CONTRIBUTING.md#commit-guidelines)
    * [Pull Requests](./CONTRIBUTING.md#pull-requests)
    * [Coding Style](./CONTRIBUTING.md#coding-style)
