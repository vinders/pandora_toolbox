# Pandora Toolbox
![CI_AppVeyor](https://img.shields.io/badge/CI-AppVeyor-777.svg)
![CI_Travis](https://img.shields.io/badge/CI-Travis_CI-777.svg)
![Cmake](https://img.shields.io/badge/project-CMake-49788c.svg)
![C++](https://img.shields.io/badge/lang-C++_17-49788c.svg)
![Gtest](https://img.shields.io/badge/tests-Googletest-49788c.svg)
![Doxy](https://img.shields.io/badge/docs-Doxygen-49788c.svg)

Pandora Toolbox is a collection of portable C++ libraries: general purpose utilities, data containers, common patterns, video filters and helpers of all kinds. 
The code is meant to be highly efficient and easy to use in real-time and video projects : AI, video processing, video games, emulators or embedded systems. Written in C++ 17, it supports backwards compatibility with C++ 14.

See [Libraries](#libraries) section below for more information. Check changelog [here](./CHANGELOG.md).
Contributors should also check the [Workflow](#workflow) and [Coding style](#coding-style) guidelines.

## Build status
![Cmake3_14](https://img.shields.io/badge/cmake-3.14-888.svg)
[![codecov](https://codecov.io/gh/vinders/pandora_toolbox/branch/develop/graph/badge.svg?token=5NQ4BF7QRI)](https://codecov.io/gh/vinders/pandora_toolbox)
![C++17](https://img.shields.io/badge/c++-17-75b.svg)
![C++14](https://img.shields.io/badge/c++-14-a99dc2.svg)

|           system            |        compiler        |         status         |
|-----------------------------|------------------------|------------------------|
| Windows 7+ <sub>(x86/x86_64)</sub> | ![vs2019](https://img.shields.io/badge/visual_studio-2019-75b.svg)<br>![vs2017](https://img.shields.io/badge/visual_studio-2017-a99dc2.svg)<br>![mingw7](https://img.shields.io/badge/mingw64-8.1.0-75b.svg) | ![BranchMaster](https://img.shields.io/badge/-master-49788c.svg) [![AppVeyor Master](https://ci.appveyor.com/api/projects/status/38j8o8sc55iosqu6/branch/master?svg=true)](https://ci.appveyor.com/project/vinders/pandora-toolbox/branch/master)<br>![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![AppVeyor Develop](https://ci.appveyor.com/api/projects/status/38j8o8sc55iosqu6/branch/develop?svg=true)](https://ci.appveyor.com/project/vinders/pandora-toolbox/branch/develop) |
| Linux <sub>(x86_64)</sub>          | ![gcc9](https://img.shields.io/badge/gcc-9.1-75b.svg)       | ![BranchMaster](https://img.shields.io/badge/-master-49788c.svg) [![Build1-Linux-gcc](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/1)](https://travis-ci.org/vinders/pandora_toolbox)<br>![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build2-Linux-gcc](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/2)](https://travis-ci.org/vinders/pandora_toolbox) |
|                                    | ![clang9](https://img.shields.io/badge/clang-9.0.0-75b.svg) | ![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build-Linux-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/1)](https://travis-ci.org/vinders/pandora_toolbox) |
|                                    | ![gcc7](https://img.shields.io/badge/gcc-7.1-75b.svg)       | ![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build-Linux-gcc7](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/3)](https://travis-ci.org/vinders/pandora_toolbox) |
|                                    | ![gcc5](https://img.shields.io/badge/gcc-5.1-a99dc2.svg)    | ![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build-Linux-gcc5](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/4)](https://travis-ci.org/vinders/pandora_toolbox) |
| Mac OS X <sub>(x86_64)</sub>       | ![clang7](https://img.shields.io/badge/clang-7.0.0-75b.svg) | ![BranchMaster](https://img.shields.io/badge/-master-49788c.svg) [![Build1-Mac-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/3)](https://travis-ci.org/vinders/pandora_toolbox)<br>![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build2-Mac-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/5)](https://travis-ci.org/vinders/pandora_toolbox) |
| iOS 9+ <sub>(arm/arm64)</sub>      | ![clang7](https://img.shields.io/badge/clang-7.0.0-75b.svg) | ![BranchMaster](https://img.shields.io/badge/-master-49788c.svg) [![Build1-iOS-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/4)](https://travis-ci.org/vinders/pandora_toolbox)<br>![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build2-iOS-clang](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/6)](https://travis-ci.org/vinders/pandora_toolbox) |
| Android 8+ <sub>(arm64)</sub>      | ![android64](https://img.shields.io/badge/android-ndk_26-a99dc2.svg) | ![BranchMaster](https://img.shields.io/badge/-master-49788c.svg) [![Build1-Android64-ndk](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/master/5)](https://travis-ci.org/vinders/pandora_toolbox)<br>![BranchDevelop](https://img.shields.io/badge/-develop-49788c.svg) [![Build2-Android64-ndk](https://travis-matrix-badges.herokuapp.com/repos/vinders/pandora_toolbox/branches/develop/7)](https://travis-ci.org/vinders/pandora_toolbox) |

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
| Build separate symbol files, for debuggers such as GDB.<br><sub>*Default: OFF*</sub>        | CWORK_BUILD_SYMBOL_FILES  | ON<br>OFF     | all systems              |
| Include unit tests of each library<br>in the built solution.<br><sub>*Default: ON*</sub>     | CWORK_TESTS              | ON<br>OFF     | all except cross-compilation & ARM  |
| Generate code documentation (doxygen) for the built solution.<br><sub>*Default: OFF*</sub>   | CWORK_DOCS               | ON<br>OFF     | all except cross-compilation  |
| Include special tools related to project in the built solution.<br><sub>*Default: ON*</sub>  | CWORK_TOOLS              | ON<br>OFF     | all systems              |
| Generate debugging headers to allow code coverage.<br><sub>*Default: OFF*</sub>              | CWORK_COVERAGE           | ON<br>OFF     | only linux/unix systems  |
| Enable OpenGL4/ES features.<br><sub>*Default: ON*</sub>                                     | USE_PANDORA_VIDEO_OPENGL  | ON<br>OFF     | all systems              |
| Enable Vulkan features.<br><sub>*Default: ON, if Vulkan SDK is installed*</sub>             | USE_PANDORA_VIDEO_VULKAN  | ON<br>OFF     | all except iOS           |

---

### Workflow

Branches:
* *master* : no direct commit, **only merges** from develop (for each release). Always call '_scripts/merge_develop_into_master.sh' (SSH key access to repo required) for every merge of *develop* into *master*.
* *develop*: main development branch, with full continuous integration and code coverage. Small commits can be directly committed in *develop*. Complex commits belong in feature branches.
* *feat/...* or *fix/...* (ex: *feat/json_serializer*): branches for new features and fixes, with continuous integration on main systems. Should be merged into *develop* when complete (with param --no-ff).

Generated version and docs:

The *build_version.txt* and *changelog.md* files are automatically generated by the script '_scripts/merge_develop_into_master.sh', based on the prefixes of commits (see 'Commit guidelines' below).

### Commit guidelines

All commits should follow one of these guidelines for the message (no additional space between prefix and dash):
> * **prefix**-library_or_module_name: description_of_commit
> * **prefix**-issue_number-library_or_module_name: description_of_commit

| prefix |             usage              |
|--------|--------------------------------|
| *major* | Major changes, important breaking of compatibility, new product line, ... Increases the major version number of the project (ex: 1.#.#) and appears in change log. |
| *feat*  | New feature or subfeature, or additional behavior. Increases the minor version number of the project (ex: #.1.#) and appears in change log. If the same feature is subdivided in multiple commits, use *patch* for the first commits, then *feat* for the last commit (feature delivery). Neglectible changes made later (comments, formatting, ...) can also use *patch*. If a feature needs to be refactored or bug-fixed, use *fix* instead. |
| *fix*   | Bug fix or refactoring of a feature. Increases the fix version number of the project (ex: #.#.1) and appears in change log. |
| *patch* | Small changes that do not really affect the behavior of a feature, or comments or additional tests. No impact on the version number. |
| *infra* | Infrastructure changes: cmake, CI, coverage, scripts, ... No impact on the version number. |

*Library or module name*: library containing the changed code (system, memory, pattern, video, ...), or name of the infrastructure module changed (ci, cwork, libs, scripts, ...).

Examples:
> * **feat**-pattern: iterator/iterable pattern + declaration helpers
> * **fix**-9976-pattern: iterator/iterable: fix missing pointer init
> * **infra**-cwork: improve cmake versioning

---

### Coding style

#### Naming

* **Types (classes, structures, enums)**: *PascalCase*.
* **Type aliases (using, typedef)**: *PascalCase* (except for types similar to C++ stdlib, such as *'time_point'* in a clock).
* **Functions**: *camelCase* (except when dealing with standard C++ stdlib, for cases such as *'try_lock'* or *'max_size'*).
* **Variables, namespaces**: *camelCase*.
* **Macros**: *CAPITAL_LETTERS*.

#### Code indent

* **Indent**: 2 spaces (no tabulations)
* **Brace style**: stroustrup's K&R style:
```c
    if (condition) {
      doStuff();
    }
    else {
      cancel();
    }
```
* **Preprocessor**: also indent preprocessor commands (after the '#' symbol):
```c
    #ifdef _WINDOWS
    # include <Windows.h>
    # if _MSC_VER > 1924
    #   include <filesystem>
    # endif
    #endif
```

#### Class organization

* **Order**: *public* first, then *protected*, and *private* last. Make the class *final* if it's not meant to be inherited.
* **Naming**: private class member variables & private functions are prefixed with '\_' *(ex: int \_value{ 0 };)*.
* **Constructors**: use *=default* or *=delete* for copy/move constructors and operators if possible. If they must be explicitly defined, then the move constructor/operator must be *noexcept*.

#### Comments

* **Guideline**: always add comment headers for classes to describe their goal. Add comments for functions when they're not obvious or when they're complex.
* **Classes headers**: use Doxygen formatting:
```c
    /// @class ClassName
    /// @brief Short description of the class.
    /// @warning Optional warnings about the behavior of the class.
```
* **Functions headers**: use Doxygen formatting:
```c
    /// @brief Short description of the function.
    /// @param paramName  Optional description of an argument, if it's not obvious.
    /// @returns Optional information about the return type, if it's not obvious.
    /// @warning Optional warnings about the behavior of the function.
```
* **Section comments**: to separate sections of a header or sub-sections of a class, use ```// -- title --```.

#### Macros

* **Guideline**: only use macros for code generators (automatic enum serializers, code multipliers...). Prefer inline functions for anything else.
* **Naming**: use capital letters and prefixes. Public macros are prefixed with '\_P\_', and private macros (for internal usage) are prefixed with '__' *(ex: __FILL_PARAMS)*.
* **Definition**: private macros located in header files \*.h (for internal usage in the header file) are undefined at the end of header file (with *#undef __MACRO_NAME*).

#### Good practices

* **Scoped enums**: always use *enum class* instead of *enum* to declare enumerations.
* **Namespaces**: enclose code with namespaces (project and library name) *(ex: pandora::memory::MyMemClass)*.
* **Pointers**: prefer *std::unique_ptr* or *std::shared_ptr* to raw pointers (*Type\**) in most cases. Only use raw pointers for function arguments (optional params or polymorphism) or for special patterns (optional references).
* **Header-only**: only create header-only classes for templates and objects with simple implementations. Use implementation files (\*.cpp) for business logic, complex code, or when including a system API (win32, openGL...).
* **Warnings**: try to fix warnings when it's possible without too much overhead (and without breaking some functionalities).
