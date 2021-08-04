/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#pragma once

#include <cstdint>
#include <cstddef>

// Mac OS
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
# if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
#   ifdef _VIDEO_OPENGL_VERSION
#     if _VIDEO_OPENGL_VERSION == 41 || _VIDEO_OPENGL_VERSION == 40 || _VIDEO_OPENGL_VERSION == 4 || _VIDEO_OPENGL_VERSION == 33 || _VIDEO_OPENGL_VERSION == 32 || _VIDEO_OPENGL_VERSION == 31 || _VIDEO_OPENGL_VERSION == 30 || _VIDEO_OPENGL_VERSION == 3
#       include <OpenGL/gl3.h>
#       include <OpenGL/gl3ext.h>
#     elif _VIDEO_OPENGL_VERSION == 21 || _VIDEO_OPENGL_VERSION == 20 || _VIDEO_OPENGL_VERSION == 2 || _VIDEO_OPENGL_VERSION == 15 || _VIDEO_OPENGL_VERSION == 14 || _VIDEO_OPENGL_VERSION == 13 || _VIDEO_OPENGL_VERSION == 12 || _VIDEO_OPENGL_VERSION == 11 || _VIDEO_OPENGL_VERSION == 10 || _VIDEO_OPENGL_VERSION == 1
#       include <OpenGL/gl.h>
#       include <OpenGL/glu.h>
#     else
#       include <OpenGL/gl3.h>
#       include <OpenGL/gl3ext.h>
#     endif
#   else
#     include <OpenGL/gl3.h>
#     include <OpenGL/gl3ext.h>
#   endif
# endif

// Windows / Linux
#elif !defined(__ANDROID__)
# ifndef GLEW_STATIC
#  define GLEW_STATIC
# endif
# include <GL/glew.h>
#endif
