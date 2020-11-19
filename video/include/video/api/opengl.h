/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <cstddef>
#include <stdexcept>

#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
// iOS
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   ifdef _VIDEO_OPENGL_ES_VERSION
#     if _VIDEO_OPENGL_ES_VERSION == 32 || _VIDEO_OPENGL_ES_VERSION == 31 || _VIDEO_OPENGL_ES_VERSION == 30 || _VIDEO_OPENGL_ES_VERSION == 3
#       include <OpenGLES/ES3/gl.h>
#       include <OpenGLES/ES3/glext.h>
#     elif _VIDEO_OPENGL_ES_VERSION == 2 || _VIDEO_OPENGL_ES_VERSION == 20
#       include <OpenGLES/ES2/gl.h>
#       include <OpenGLES/ES2/glext.h>
#     elif _VIDEO_OPENGL_ES_VERSION == 1 || _VIDEO_OPENGL_ES_VERSION == 10
#       include <OpenGLES/ES1/gl.h>
#       include <OpenGLES/ES1/glext.h>
#     else
#       include <OpenGLES/ES3/gl.h>
#       include <OpenGLES/ES3/glext.h>
#     endif
#   else
#     include <OpenGLES/ES3/gl.h>
#     include <OpenGLES/ES3/glext.h>
#   endif
// Mac OS
# else
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

// Android
#elif defined(__ANDROID__)
# ifdef _VIDEO_OPENGL_ES_VERSION
#   if _VIDEO_OPENGL_ES_VERSION == 32
#     include <GLES3/gl32.h>
#     include <GLES3/gl3ext.h>
#   elif _VIDEO_OPENGL_ES_VERSION == 31
#     include <GLES3/gl31.h>
#     include <GLES3/gl3ext.h>
#   elif _VIDEO_OPENGL_ES_VERSION == 3 || _VIDEO_OPENGL_ES_VERSION == 30
#     include <GLES3/gl3.h>
#     include <GLES3/gl3ext.h>
#   elif _VIDEO_OPENGL_ES_VERSION == 2 || _VIDEO_OPENGL_ES_VERSION == 20
#     include <GLES2/gl2.h>
#     include <GLES2/gl2ext.h>
#   elif _VIDEO_OPENGL_ES_VERSION == 1 || _VIDEO_OPENGL_ES_VERSION == 10
#     include <GLES/gl.h>
#     include <GLES/glext.h>
#   else
#     include <GLES3/gl3.h>
#     include <GLES3/gl3ext.h>
#   endif
# else
#   include <GLES3/gl3.h>
#   include <GLES3/gl3ext.h>
# endif

// Windows / Linux
#else
# ifndef GLEW_STATIC
#  define GLEW_STATIC
# endif
# include <GL/glew.h>
#endif
