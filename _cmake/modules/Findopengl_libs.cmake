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

set(opengl_libs__FOUND ON)

if(NOT IOS AND NOT ANDROID)
    if(APPLE)
        find_package(OpenGL REQUIRED)
        set(opengl_libs__INCLUDE ${OPENGL_INCLUDE_DIR})
        
        set(_EXTERNAL_FRAMEWORKS
            OpenGL
            GLUT
        )
        if(CWORK_EXTERN_FRAMEWORKS)
            set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} ${_EXTERNAL_FRAMEWORKS})
        else()
            set(CWORK_EXTERN_FRAMEWORKS ${_EXTERNAL_FRAMEWORKS})
        endif()
        unset(_EXTERNAL_FRAMEWORKS)
        
        set(opengl_libs__LINKED ${OPENGL_LIBRARIES})
        
    elseif(CWORK_LINUX_WAYLAND)
        include(GNUInstallDirs)
        include(FindPkgConfig)
        pkg_check_modules(Wayland REQUIRED
            wayland-client>=0.2.7
            wayland-egl>=0.2.7
        )
        set(opengl_libs__INCLUDE ${Wayland_INCLUDE_DIRS})
        set(opengl_libs__LINKED ${Wayland_LINK_LIBRARIES})

    else()
        find_package(OpenGL REQUIRED)
        set(opengl_libs__LINKED ${OPENGL_LIBRARIES})
    endif()

    if(CWORK_EXTERN_FRAMEWORKS AND NOT CWORK_EXTERN_FRAMEWORKS_SCOPE)
        if(CWORK_EXTERN_LIBS_SCOPE)
            set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${CWORK_EXTERN_LIBS_SCOPE})
        else()
            set(CWORK_EXTERN_FRAMEWORKS_SCOPE PUBLIC)
        endif()
    endif()
endif()
