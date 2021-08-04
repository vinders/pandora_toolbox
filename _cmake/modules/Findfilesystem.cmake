# MIT License
# Copyright (c) 2021 Romain Vinders

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
# IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

set(filesystem__FOUND ON)

if(MINGW)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL 9.0)
        set(filesystem__LINKED user32.lib Shlwapi.lib)
    else()
        set(filesystem__LINKED Shlwapi.lib)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL 9.0 
AND (NOT CWORK_CPP_REVISION OR NOT CWORK_CPP_REVISION STREQUAL "14"))
    set(filesystem__LINKED "stdc++fs")
elseif(MSVC AND (WIN32 OR WIN64 OR _WIN32 OR _WIN64))
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.14)
        set(filesystem__LINKED User32.lib Shlwapi.lib)
    else()
        set(filesystem__LINKED Shlwapi.lib)
    endif()
endif()
