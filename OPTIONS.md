> [Home](./README.md) - | - [Features](./FEATURES.md) - | - [Examples &amp; tutorials](./_examples/README.md) - | - [Changelog](./CHANGELOG.md) - | - [Issue reporting &amp; contribution](./CONTRIBUTING.md)

# CMake options

* [Building project with CMake](#building-project-with-cmake)
* [General project options](#general-project-options)
* [Rendering/API options](#renderingapi-options)

## Building project with CMake

This project can be open with any IDE that can natively handle CMake files, or can be generated with CMake. Some scripts are available to automatically generate the project for common IDEs ("_scripts" directory).
A few options can be used to build the solution when calling CMake (or in the CMake settings of the IDE).

Using CMake: \
**cmake -S [source_dir] -G [generator_name] -A [arch] -B [build_dir] -D[option_name]=[option_value]**

Examples:
> * cmake -S . -G "Visual Studio 16 2019" -A x64 -B _build -DCWORK_TESTS=OFF
> * cmake -S . -G "Unix Makefiles" -B _build -DCWORK_DOCS=ON
> * cmake -S . -G "MinGW Makefiles" -B _build -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_MAKE_PROGRAM=mingw32-make.exe

## General project options

|    Option    |    Name    |    Value(s)    |    Available for...    |
|--------------|------------|----------------|------------------------|
| Retro-compatibility with older<br>C++ revisions(ex: C++14).<br><sub>*Default: "17" if supported, otherwise "14"*</sub>   | CWORK_CPP_REVISION  | "20"<br>"17"<br>"14"  | all C++17-compliant compilers  |
| Build separate symbol files,<br>for debuggerssuch as GDB.<br><sub>*Default: OFF*</sub>          | CWORK_BUILD_SYMBOL_FILES   | ON<br>OFF     | all systems                   |
| Include unit tests of each library<br>in the built solution.<br><sub>*Default: ON*</sub>        | CWORK_TESTS                | ON<br>OFF     | all desktop systems           |
| Generate code documentation<br>(doxygen) for the built solution.<br><sub>*Default: OFF*</sub>   | CWORK_DOCS                 | ON<br>OFF     | all desktop systems           |
| Include special tools related to<br>projects in the built solution.<br><sub>*Default: ON*</sub> | CWORK_TOOLS                | ON<br>OFF     | all systems                   |
| Generate debugging headers to<br>allow code coverage.<br><sub>*Default: OFF*</sub>              | CWORK_COVERAGE             | ON<br>OFF     | only linux/unix systems       |

## Rendering/API options

|    Option    |    Name    |    Value(s)    |    Available for...    |
|--------------|------------|----------------|------------------------|
| Minimum Windows version<br>support (7/8.1/10.RS2).<br><sub>*Default: "8" on Win8.1/10+, otherwise "7"*</sub>       | CWORK_WINDOWS_VERSION  | "10" "8" "7"  | only windows systems   |
| Use Linux Wayland display server<br>(instead of X.org).<br><sub>*Default: OFF*</sub>            | CWORK_LINUX_WAYLAND        | ON<br>OFF     | only linux systems            |
| Enable Direct3D 11 features.<br><sub>*Default: ON if MSVC/clang-cl compiler*</sub>           | CWORK_VIDEO_D3D11          | ON<br>OFF     | only windows systems<br>(MSVC/LLVM required)  |
| Maximum Direct3D 11 feature<br>level (11.0 - 11.4).<br><sub>*Default: "114" on Win8.1/10+, otherwise "110"*</sub>  | CWORK_D3D11_VERSION  | "114" "113"<br>"111" "110"  | only windows systems<br>(MSVC/LLVM required)  |
| Enable Vulkan features.<br><sub>*Default: ON if Vulkan SDK is installed*</sub>               | CWORK_VIDEO_VULKAN         | ON<br>OFF     | all systems<br>(environment variable<br>*VULKAN_SDK* required)  |
| Maximum Vulkan feature level.<br><sub>*Default: "13" with sdk 1.3+, otherwise "12"*</sub>    | CWORK_VULKAN_VERSION       | "13" "12"     | all systems<br>(*VULKAN_SDK* required)  |
| Enable OpenGL 4 features.<br><sub>*Default: OFF*</sub>                                       | CWORK_VIDEO_OPENGL4        | ON<br>OFF     | all desktop systems           |
| Maximum OpenGL 4 feature level.<br><sub>*Default: "45" (or "41" on Mac OS)*</sub>            | CWORK_OPENGL4_VERSION      | "46" "45"<br>"43" "41"  | all desktop systems |
| | | | |
| Allow shader compilation<br>at runtime (D3D/Vulkan)<br><sub>*Default: ON*</sub>                 | CWORK_SHADER_COMPILERS     | ON<br>OFF     | all desktop systems           |
| Disable geometry shader stage<br>to reduce overhead (if not used)<br><sub>*Default: OFF*</sub> | CWORK_GEOM_STAGE_OFF     | ON<br>OFF     | all systems                   |
| Disable tessellation stage<br>to reduce overhead (if not used)<br><sub>*Default: OFF*</sub>  | CWORK_TESS_STAGE_OFF       | ON<br>OFF     | all systems                   |
