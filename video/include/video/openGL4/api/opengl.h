/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
