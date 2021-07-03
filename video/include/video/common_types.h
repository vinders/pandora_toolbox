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
      variableRefresh = 0x2u, ///< Variable refresh rate display in fullscreen/borderless.
      partialOutput   = 0x4u, ///< Partial display output config (instead of unique display output):
                              ///  allows multiple partial swap-chains (for example, for multi-window rendering).
      stereo          = 0x8u, ///< Stereo rendering mode, for 3D devices/glasses (only usable in fullscreen mode).
      shaderInput     = 0x10u,///< Allow using swap-chain output as shader input data.
      disableFlipSwap = 0x20u ///< Prevent swap-chain from using flip-swap mode if supported.
    };
    
    // ---

    /// @class RefreshRate
    /// @brief Swap-chain / render-target refresh rate
    class RefreshRate final {
    public:
      /// @brief Create refresh rate -- Hz rational -- default: 60 / 1
      constexpr inline RefreshRate(uint32_t numerator, uint32_t denominator) noexcept 
        : _numerator(numerator), _denominator(denominator ? denominator : 1u) {}
      /// @brief Create refresh rate -- milliHz -- default: 60000
      /// @remarks Can be used with the refreshRate value of a DisplayMonitor instance
      inline RefreshRate(uint32_t rateMilliHz) noexcept {
        switch (rateMilliHz) {
          case 59940u: this->_numerator = 60000u; this->_denominator = 1001u; break;
          case 29970u: this->_numerator = 30000u; this->_denominator = 1001u; break;
          case 23976u: this->_numerator = 24000u; this->_denominator = 1001u; break;
          default: if ((rateMilliHz % 1000u) == 0) { this->_numerator = rateMilliHz/1000u; this->_denominator = 1u; }
                   else { this->_numerator = rateMilliHz; this->_denominator = 1000u; }
                   break;
        }
      }
      
      constexpr RefreshRate() noexcept = default; ///< Create default refresh rate (60Hz)
      RefreshRate(const RefreshRate&) = default;
      RefreshRate(RefreshRate&&) noexcept = default;
      RefreshRate& operator=(const RefreshRate&) = default;
      RefreshRate& operator=(RefreshRate&&) noexcept = default;
      ~RefreshRate() noexcept = default;
      
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
      SwapChainDescriptor() noexcept = default;
      SwapChainDescriptor(uint32_t width, uint32_t height, uint32_t framebufferCount = 2u, 
                          RefreshRate refreshRate = RefreshRate{}, TargetOutputFlag outputFlags = TargetOutputFlag::none) noexcept
        : width(width), height(height), framebufferCount(framebufferCount), refreshRate(refreshRate), outputFlags(outputFlags) {}

      SwapChainDescriptor(const SwapChainDescriptor&) = default;
      SwapChainDescriptor& operator=(const SwapChainDescriptor&) = default;
      ~SwapChainDescriptor() noexcept = default;

      uint32_t width = 0;   ///< Framebuffer pixel width (usually the window client width, except in special cases (emulator with internal resolution...))
      uint32_t height = 0;  ///< Framebuffer pixel height (usually the window client height, except in special cases (emulator with internal resolution...))
      uint32_t framebufferCount = 2u; ///< number of framebuffers (back-buffers + front-buffer) -- default: 2 (minimum: 1).
      RefreshRate refreshRate;        ///< framebuffer refresh rate -- default: 60Hz.
      TargetOutputFlag outputFlags = TargetOutputFlag::none; ///< special swap-chain output settings.
    };
  }
}
_P_FLAGS_OPERATORS(pandora::video::TargetOutputFlag, uint32_t);
