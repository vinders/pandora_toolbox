/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - resource file IO (shaders, image files)
*******************************************************************************/
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/api/types.h>
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/api/types.h>
#endif
#include "resources.h"

// -- file path discovery --

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# define _RESOURCE_PATH(path) L"" path
# define _RESOURCE_STRING(id) toWideString(id)
# define _PATH_CHAR wchar_t

# define __VERT_SHADER L"textured2D.vs.hlsl"
# define __FRAG_SHADER L"textured2D.ps.hlsl"
#else
# define _RESOURCE_PATH(path) path
# define _RESOURCE_STRING(id) toString(id)
# define _PATH_CHAR char

# define __VERT_SHADER "textured2D.vert"
# define __FRAG_SHADER "textured2D.frag"
#endif

// find path of resource directory (containing shaders, textures, sprites)
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  std::wstring getResourceDirectory();
  inline std::wstring getShaderDirectory() { return getResourceDirectory() + L"shaders/d3d11/"; }
#else
  std::string getResourceDirectory();
# if defined(_VIDEO_VULKAN_SUPPORT)
  inline std::string getShaderDirectory() { return getResourceDirectory() + "shaders/vulkan/"; }
# endif
#endif


// -- file IO --

// read texture/sprite data files
// -> throws runtime_error on failure
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  video_api::Texture2D readImageFile(const wchar_t* imagePath, ImageId id, video_api::DeviceHandle device);
#else
  video_api::Texture2D readImageFile(const char_t* imagePath, ImageId id, video_api::DeviceHandle device);
#endif
