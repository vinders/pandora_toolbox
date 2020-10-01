/*******************************************************************************
Description : Microsoft Windows base type definitions
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
  // disable unwanted features
# ifndef NOMINMAX // no min/max macros
#   define NOMINMAX
# endif
# ifndef WIN32_LEAN_AND_MEAN // exclude rare MFC libraries
#   define WIN32_LEAN_AND_MEAN
# endif

# include <windef.h>

#endif
