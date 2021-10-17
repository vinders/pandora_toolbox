/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
      variableRefresh = 0x1u, ///< Variable refresh rate display in fullscreen/borderless.
      partialOutput   = 0x2u, ///< Partial display output config (instead of unique display output):
                              ///  allows multiple partial swap-chains (for example, for multi-window rendering).
      stereo          = 0x4u, ///< Stereo rendering mode, for 3D devices/glasses (only usable in fullscreen mode).
      shaderInput     = 0x10u,///< Allow using swap-chain output as shader input data.
      disableFlipSwap = 0x20u,///< Prevent swap-chain from using flip-swap mode if supported.
      swapNoDiscard   = 0x40u ///< Keep existing data after swapping buffers if supported:
                              ///  - GPU resources not released as efficiently -> should only be used if frames aren't completely redrawn.
                              ///  - Required when previous data must be kept (otherwise, the buffer would contain garbage after being swapped).
                              ///  - Can also be useful if 'clearView' is called after every swap call (clearing the buffer also releases GPU resources).
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
