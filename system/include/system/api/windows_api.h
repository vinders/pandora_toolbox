/*******************************************************************************
Description : Microsoft Windows general API includes
*******************************************************************************/
#pragma once

#ifdef _WINDOWS

  // disable unwanted features
# ifndef NOMINMAX // no min/max macros
#   define NOMINMAX
# endif
# ifndef VC_EXTRALEAN // exclude MFC libraries
#   define VC_EXTRALEAN
# endif
# ifndef WIN32_LEAN_AND_MEAN // exclude rare MFC libraries
#   define WIN32_LEAN_AND_MEAN
# endif

# include "./windows_version.h"
# include <Windows.h>

# ifdef MessageBox
#   undef MessageBox
# endif

#endif
