/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <system/_private/_enum_flags.h>

namespace pandora {
  namespace video {
    // -- resource creation param types --
    
    /// @brief Rendering shader category
    enum class ShaderType : uint32_t {
      vertex        = 0u, ///< Vertex shader: process input vertex data -> outputs vertex projection.
      tesselControl = 1u, ///< Tessellation control/hull shader: tessellate primitive -> outputs geometry patch.
      tesselEval    = 2u, ///< Tessellation evaluation/domain shader: calculate new vertex positions.
      geometry      = 3u, ///< Geometry shader: modify/duplicate primitive.
      fragment      = 4u, ///< Fragment/pixel shader: process rasterized fragment -> outputs pixel/depth data.
      compute       = 5u  ///< Compute shader: GPU calculations (not supported below OpenGL 4.3).
    };
    
    /// @brief Advanced swap-chain target output params (bit-mask flags)
    /// @warning - Some of these features are not supported by the OpenGL API (ignored if not supported).
    ///          - Some of these features are not supported by old API versions (Direct3D 11.0, OpenGL 4.1...)
    enum class TargetOutputFlag : uint32_t {
      none            = 0x0u, ///< No option
      hdrPreferred    = 0x1u, ///< use HDR color space if possible:
                              ///  - HDR only used if supported by display monitor;
                              ///  - HDR only used if supported by rendering API version (ex: D3D11.1+) + by operating system (ex: Win10+);
                              ///  - HDR only used with HDR back-buffer formats: rgba32/rgb32/rg32, rgba16/rgb16/rg16, rgb10a2/rg11b10;
                              ///  -> Always verify if the swap-chain supports HDR after creating it (SwapChain.isHdrEnabled).
      variableRefresh = 0x2u, ///< Required for variable refresh rate display in fullscreen/borderless.
      partialOutput   = 0x4u, ///< Partial display output config (instead of unique display output):
                              ///  allows multiple partial swap-chains (for example, for multi-window rendering).
      stereo          = 0x8u, ///< Stereo rendering mode, for 3D devices/glasses (only usable in fullscreen mode)
      shaderInput     = 0x10u ///< Allow using swap-chain output as shader input data.
    };
    
    /// @brief Rasterizer depth offset settings
    struct DepthBias {
      int32_t depthBias = 0;       ///< scalar factor controlling the constant depth value added to each fragment
      float depthBiasClamp = 0.0f; ///< Maximum (or minimum) depth bias of a fragment.
      float depthBiasSlopeFactor = 0.0f; ///< Scalar factor applied to a fragment’s slope. 
                                         ///  Used to improve shadow maps + to avoid depth artifacts and self-shadowing
    };
    
    // ---

    /// @class DisplayRate
    /// @brief Swap-chain / render-target refresh rate
    class DisplayRate final {
      /// @brief Create refresh rate -- Hz rational -- default: 60 / 1
      constexpr inline DisplayRate(uint32_t numerator, uint32_t denominator) noexcept 
        : _numerator(numerator), _denominator(denominator ? denominator : 1u) {}
      /// @brief Create refresh rate -- milliHz -- default: 60000
      /// @remarks Can be used with the refreshRate value of a DisplayMonitor instance
      inline DisplayRate(uint32_t rateMilliHz) noexcept {
        switch (rateMilliHz) {
          case 59940u: this->_numerator = 60000u; this->_denominator = 1001u; break;
          case 29970u: this->_numerator = 30000u; this->_denominator = 1001u; break;
          case 23976u: this->_numerator = 24000u; this->_denominator = 1001u; break;
          default: if ((rateMilliHz % 1000u) == 0) { this->_numerator = rateMilliHz/1000u; this->_denominator = 1u; }
                   else { this->_numerator = rateMilliHz; this->_denominator = 1000u; }
                   break;
          }
        }
      }
      
      constexpr DisplayRate() noexcept = default; ///< Create default refresh rate (60Hz)
      DisplayRate(const DisplayRate&) = default;
      DisplayRate(DisplayRate&&) noexcept = default;
      DisplayRate& operator=(const DisplayRate&) = default;
      DisplayRate& operator=(DisplayRate&&) noexcept = default;
      ~DisplayRate() noexcept = default;
      
      constexpr inline uint32_t numerator() const noexcept { return _numerator; }
      constexpr inline uint32_t denominator() const noexcept { return _denominator; }

    private:
      uint32_t _numerator = 60u;
      uint32_t _denominator = 1u;
    };
    
    // ---
    
    /// @brief Swap-chain build params
    /// @remarks The same params can be used to build multiple SwapChain instances.
    struct SwapChainDescriptor final {
      uint32_t width;   ///< Framebuffer pixel width (usually the window client width, except in special cases (emulator with internal resolution...))
      uint32_t height;  ///< Framebuffer pixel height (usually the window client height, except in special cases (emulator with internal resolution...))
      uint32_t frameBufferCount = 2u; ///< number of framebuffers (back-buffers + front-buffer) -- default: 2 (minimum: 1).
      DisplayRate refreshRate;        ///< framebuffer refresh rate -- default: 60Hz.
      TargetOutputFlag outputFlags = TargetOutputFlag::none; ///< special swap-chain output settings.
    };
  }
}
_P_FLAGS_OPERATORS(pandora::video::TargetOutputFlag, uint32_t);
