@echo off

if not exist CMakeLists.txt cd ..
if not exist CMakeLists.txt cd ..
if not exist CMakeLists.txt (echo CMakeLists.txt not found in current directory nor in parent directory! && exit /B 1)

:: gtest + libs directory
if not exist ./_libs/gtest/CMakeLists.txt call git submodule update --init --recursive --remote
cd _libs

:: glew
if not exist ./glew/CMakeLists.txt (
    call git clone --depth 1 --branch "glew-cmake-2.1.0" https://github.com/Perlmint/glew-cmake glew
)
:: glm
if not exist ./glm/CMakeLists.txt (
    call git clone --depth 1 --branch "0.9.9.8" https://github.com/g-truc/glm glm
)
:: glfw3
if not exist ./glfw3/CMakeLists.txt (
    call git clone --depth 1 --branch "3.3.2" https://github.com/glfw/glfw glfw3
)

cd ..
exit /B 0
