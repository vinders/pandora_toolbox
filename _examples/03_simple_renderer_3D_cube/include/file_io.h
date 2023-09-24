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
#ifdef _P_SHADER_PRECOMPILED
# include <memory>
#endif
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
# ifdef _P_SHADER_PRECOMPILED
#   ifdef _WIN64 // -> constant defined by cmake 'cwork' framework
#     define __VERT_SHADER_SUFFIX L".vs.hlsl64.cso"
#     define __FRAG_SHADER_SUFFIX L".ps.hlsl64.cso"
#   else
#     define __VERT_SHADER_SUFFIX L".vs.hlsl.cso"
#     define __FRAG_SHADER_SUFFIX L".ps.hlsl.cso"
#   endif
# else
#   define __VERT_SHADER_SUFFIX L".vs.hlsl"
#   define __FRAG_SHADER_SUFFIX L".ps.hlsl"
# endif

#else
# define _RESOURCE_PATH(path) path
# define _RESOURCE_STRING(id) toString(id)
# define _PATH_CHAR char
# if defined(_VIDEO_VULKAN_SUPPORT) && defined(_P_SHADER_PRECOMPILED)
#   define __VERT_SHADER_SUFFIX ".vert.spv"
#   define __FRAG_SHADER_SUFFIX ".frag.spv"
# else
#   define __VERT_SHADER_SUFFIX ".vert"
#   define __FRAG_SHADER_SUFFIX ".frag"
# endif
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

// read pre-compiled shader binary files
// -> throws runtime_error on failure
#ifdef _P_SHADER_PRECOMPILED
  void readBinaryShaderFile(const _PATH_CHAR* filePath, std::unique_ptr<uint8_t[]>& outBuffer, size_t& outLength);
#endif

// read texture & sprite data files
// -> throws runtime_error on failure
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  video_api::Texture2D readTextureFile(const std::wstring& commonFilePath, const wchar_t* fileSuffix,
                                       TextureMapId id, video_api::DeviceHandle device);
  video_api::Texture2D readSpriteFile(const wchar_t* imagePath, SpriteId id, video_api::DeviceHandle device);
#else
  video_api::Texture2D readTextureFile(const std::string& commonFilePath, const char_t* fileSuffix,
                                       TextureMapId id, video_api::DeviceHandle device);
  video_api::Texture2D readSpriteFile(const char_t* imagePath, SpriteId id, video_api::DeviceHandle device);
#endif
