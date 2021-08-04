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

# Android
if(ANDROID)
    set(android_glue_interface__FOUND ON)
    
    if(ANDROID_NDK)
        set(_PATH_PREFIX ${ANDROID_NDK})
    elseif(ANDROID_NDK_ROOT)
        set(_PATH_PREFIX ${ANDROID_NDK_ROOT})
    elseif(DEFINED ENV{ANDROID_NDK_ROOT})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_ROOT})
    elseif(ANDROID_NDK_HOME)
        set(_PATH_PREFIX ${ANDROID_NDK_HOME})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_HOME})
    else()
        set(_PATH_PREFIX /usr/local/android-ndk)
    endif()
    
    include(${CMAKE_CURRENT_LIST_DIR}/path_utils.cmake)
    
    cwork_unknown_path_to_absolute(${_PATH_PREFIX} "/sources/android/native_app_glue/android_native_app_glue.c")
    set(android_glue_interface__INCLUDE ${CWORK_VALID_ABSOLUTE_PATH}/sources/android/native_app_glue)
    unset(CWORK_VALID_ABSOLUTE_PATH)
    unset(_PATH_PREFIX)
endif()
