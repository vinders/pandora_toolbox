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
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   include <OpenGLES/ES3/gl.h>
#   include <OpenGLES/ES3/glext.h>
# else
#   include <OpenGL/gl.h>
# endif
#elif defined(__ANDROID__)
# include <GLES/gl.h>
# include <GLES/glext.h>
#else
# ifndef GLEW_STATIC
#  define GLEW_STATIC
# endif
# include <GL/glew.h>
#endif
