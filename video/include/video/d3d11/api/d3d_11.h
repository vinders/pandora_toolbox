/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS)
# if defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION >= 114
#   include <d3d11_4.h>
# elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 113
#   include <d3d11_3.h>
# elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 110
#   include <d3d11.h>
# else
#   include <d3d11_1.h>
# endif
#endif
