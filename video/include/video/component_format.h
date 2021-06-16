/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <system/align.h>

namespace pandora {
  namespace video {
    /// @brief Component vector structure - 4 * 32-bit (position/color container)
    __align_type(16,
    struct ComponentVector128 {
      union {
        float f[4];
        int32_t i32[4];
      };
    });

    /// @brief Color/depth/stencil component formats bindings for drawing buffers
    /// @remarks - For HDR rendering and color space, use RG(BA) components between 10 and 32 bits ('rgba16_f_hdr_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
    ///          - For SDR rendering and color space, use other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
    ///          - For HDR rendering on SDR devices, shaders will need to convert color values (or they'll be clipped when displayed).
    /// @warning Block-compression types (bc*) are not available for OpenGL.
    enum class ComponentFormat : int32_t {
      unknown = 0, ///< Unknown/empty format
      custom = 1, ///< Use custom/native value
      
      // HDR / 32 bit
      rgba32_f = 6,///< GL_RGBA32F - DXGI_FORMAT_R32G32B32A32_FLOAT - VK_FORMAT_R32G32B32A32_SFLOAT
      rgb32_f = 7, ///< GL_RGB32F - DXGI_FORMAT_R32G32B32_FLOAT - VK_FORMAT_R32G32B32_SFLOAT
      rg32_f = 8, ///< GL_RG32F - DXGI_FORMAT_R32G32_FLOAT - VK_FORMAT_R32G32_SFLOAT
      r32_f = 9,  ///< GL_R32F - DXGI_FORMAT_R32_FLOAT - VK_FORMAT_R32_SFLOAT
      
      rgba32_ui = 10,///< GL_RGBA32UI - DXGI_FORMAT_R32G32B32A32_UINT - VK_FORMAT_R32G32B32A32_UINT
      rgb32_ui = 11, ///< GL_RGB32UI - DXGI_FORMAT_R32G32B32_UINT - VK_FORMAT_R32G32B32_UINT
      rg32_ui = 12, ///< GL_RG32UI - DXGI_FORMAT_R32G32_UINT - VK_FORMAT_R32G32_UINT
      r32_ui = 13,  ///< GL_R32UI - DXGI_FORMAT_R32_UINT - VK_FORMAT_R32_UINT

      rgba32_i = 14,///< GL_RGBA32I - DXGI_FORMAT_R32G32B32A32_SINT - VK_FORMAT_R32G32B32A32_SINT
      rgb32_i = 15, ///< GL_RGB32I - DXGI_FORMAT_R32G32B32_SINT - VK_FORMAT_R32G32B32_SINT
      rg32_i = 16, ///< GL_RG32I - DXGI_FORMAT_R32G32_SINT - VK_FORMAT_R32G32_SINT
      r32_i = 17,  ///< GL_R32I - DXGI_FORMAT_R32_SINT - VK_FORMAT_R32_SINT
      
      // HDR / 16 bit
      rgba16_f_hdr_scRGB = 21, ///< HDR-FP16/scRGB: GL_RGBA16F - DXGI_FORMAT_R16G16B16A16_FLOAT - VK_FORMAT_R16G16B16A16_SFLOAT
      rg16_f = 22, ///< GL_RG16F - DXGI_FORMAT_R16G16_FLOAT - VK_FORMAT_R16G16_SFLOAT
      r16_f = 23,  ///< GL_R16F - DXGI_FORMAT_R16_FLOAT - VK_FORMAT_R16_SFLOAT
      
      rgba16_ui = 24,///< GL_RGBA16UI - DXGI_FORMAT_R16G16B16A16_UINT - VK_FORMAT_R16G16B16A16_UINT
      rg16_ui = 25,  ///< GL_RG16UI - DXGI_FORMAT_R16G16_UINT - VK_FORMAT_R16G16_UINT
      r16_ui = 26,   ///< GL_R16UI - DXGI_FORMAT_R16_UINT - VK_FORMAT_R16_UINT
      
      rgba16_i = 27,///< GL_RGBA16I - DXGI_FORMAT_R16G16B16A16_SINT - VK_FORMAT_R16G16B16A16_SINT
      rg16_i = 28,  ///< GL_RG16I - DXGI_FORMAT_R16G16_SINT - VK_FORMAT_R16G16_SINT
      r16_i = 29,   ///< GL_R16I - DXGI_FORMAT_R16_SINT - VK_FORMAT_R16_SINT
      
      rgba16_unorm = 30,///< GL_RGBA16 - DXGI_FORMAT_R16G16B16A16_UNORM - VK_FORMAT_R16G16B16A16_UNORM
      rg16_unorm = 31,  ///< GL_RG16 - DXGI_FORMAT_R16G16_UNORM - VK_FORMAT_R16G16_UNORM
      r16_unorm = 32,   ///< GL_R16 - DXGI_FORMAT_R16_UNORM - VK_FORMAT_R16_UNORM
      
      rgba16_snorm = 33,///< GL_RGBA16_SNORM - DXGI_FORMAT_R16G16B16A16_SNORM - VK_FORMAT_R16G16B16A16_SNORM
      rg16_snorm = 34,  ///< GL_RG16_SNORM - DXGI_FORMAT_R16G16_SNORM - VK_FORMAT_R16G16_SNORM
      r16_snorm = 35,   ///< GL_R16_SNORM - DXGI_FORMAT_R16_SNORM - VK_FORMAT_R16_SNORM
      
      // HDR / 10-11 bit
      rgb10a2_unorm_hdr10 = 36, ///< HDR-10/BT.2100: GL_RGB10_A2 - DXGI_FORMAT_R10G10B10A2_UNORM - VK_FORMAT_A2R10G10B10_UNORM_PACK32
      rgb10a2_ui = 38, ///< GL_RGB10_A2UI - DXGI_FORMAT_R10G10B10A2_UINT - VK_FORMAT_A2R10G10B10_UINT_PACK32
      rg11b10_f = 39,  ///< GL_R11F_G11F_B10F - DXGI_FORMAT_R11G11B10_FLOAT - VK_FORMAT_B10G11R11_UFLOAT_PACK32
      
      // 9 bit
      rgb9e5_uf = 40,  ///< GL_RGB9_E5 - DXGI_FORMAT_R9G9B9E5_SHAREDEXP - VK_FORMAT_E5B9G9R9_UFLOAT_PACK32
      
      // 8 bit
      rgba8_ui = 45,///< GL_RGBA8UI - DXGI_FORMAT_R8G8B8A8_UINT - VK_FORMAT_R8G8B8A8_UINT
      rg8_ui = 46,  ///< GL_RG8UI - DXGI_FORMAT_R8G8_UINT - VK_FORMAT_R8G8_UINT
      r8_ui = 47,   ///< GL_R8UI - DXGI_FORMAT_R8_UINT - VK_FORMAT_R8_UINT
      
      rgba8_i = 48,///< GL_RGBA8I - DXGI_FORMAT_R8G8B8A8_SINT - VK_FORMAT_R8G8B8A8_SINT
      rg8_i = 49,  ///< GL_RG8I - DXGI_FORMAT_R8G8_SINT - VK_FORMAT_R8G8_SINT
      r8_i = 50,   ///< GL_R8I - DXGI_FORMAT_R8_SINT - VK_FORMAT_R8_SINT
      
      rgba8_sRGB = 51, ///< GL_SRGB8_ALPHA8 - DXGI_FORMAT_R8G8B8A8_UNORM_SRGB - VK_FORMAT_R8G8B8A8_SRGB
      bgra8_sRGB = 52, ///< GL_SRGB8_ALPHA8 - DXGI_FORMAT_B8G8R8A8_UNORM_SRGB - VK_FORMAT_B8G8R8A8_SRGB
      rgba8_unorm = 53,///< GL_RGBA8 - DXGI_FORMAT_R8G8B8A8_UNORM - VK_FORMAT_R8G8B8A8_UNORM
      bgra8_unorm = 54,///< GL_RGBA8 - DXGI_FORMAT_B8G8R8A8_UNORM - VK_FORMAT_B8G8R8A8_UNORM
      rg8_unorm = 55,  ///< GL_RG8 - DXGI_FORMAT_R8G8_UNORM - VK_FORMAT_R8G8_UNORM
      r8_unorm = 56,   ///< GL_R8 - DXGI_FORMAT_R8_UNORM - VK_FORMAT_R8_UNORM
      a8_unorm = 57,   ///< GL_R8_SNORM - DXGI_FORMAT_A8_UNORM - VK_FORMAT_R8_SNORM
      
      rgba8_snorm = 58,///< GL_RGBA8_SNORM - DXGI_FORMAT_R8G8B8A8_SNORM - VK_FORMAT_R8G8B8A8_SNORM
      rg8_snorm = 59,  ///< GL_RG8_SNORM - DXGI_FORMAT_R8G8_SNORM - VK_FORMAT_R8G8_SNORM
      r8_snorm = 60,   ///< GL_R8_SNORM - DXGI_FORMAT_R8_SNORM - VK_FORMAT_R8_SNORM
      
      // 4-6 bit
      rgb5a1_unorm = 61, ///< GL_RGB5_A1 - DXGI_FORMAT_B5G5R5A1_UNORM - VK_FORMAT_B5G5R5A1_UNORM_PACK16
      r5g6b5_unorm = 62, ///< GL_RGB565 - DXGI_FORMAT_B5G6R5_UNORM - VK_FORMAT_B5G6R5_UNORM_PACK16
      rgba4_unorm = 63, ///< GL_RGBA4 - DXGI_FORMAT_B4G4R4A4_UNORM - VK_FORMAT_B4G4R4A4_UNORM_PACK16
      
      // depth/stencil
      d32_f = 64,     ///< GL_DEPTH_COMPONENT32F - DXGI_FORMAT_D32_FLOAT - VK_FORMAT_D32_SFLOAT
      d16_unorm = 65, ///< GL_DEPTH_COMPONENT16 - DXGI_FORMAT_D16_UNORM - VK_FORMAT_D16_UNORM
      d32_f_s8_ui = 66,    ///< GL_DEPTH32F_STENCIL8 - DXGI_FORMAT_D32_FLOAT_S8X24_UINT - VK_FORMAT_D32_SFLOAT_S8_UINT
      d24_unorm_s8_ui = 67,///< GL_DEPTH24_STENCIL8 - DXGI_FORMAT_D24_UNORM_S8_UINT - VK_FORMAT_D24_UNORM_S8_UINT

      // block-compression (not supported in OpenGL)
      bc6h_uf = 68, ///< DXGI_FORMAT_BC6H_UF16 - VK_FORMAT_BC6H_UFLOAT_BLOCK
      bc6h_f = 69,  ///< DXGI_FORMAT_BC6H_SF16 - VK_FORMAT_BC6H_SFLOAT_BLOCK
      
      bc7_sRGB  = 70, ///< DXGI_FORMAT_BC7_UNORM_SRGB - VK_FORMAT_BC7_SRGB_BLOCK
      bc7_unorm = 71  ///< DXGI_FORMAT_BC7_UNORM - VK_FORMAT_BC7_UNORM_BLOCK
    };
  }
}
