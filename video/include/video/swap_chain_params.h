/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include "./render_options.h"
#include "./component_format.h"

namespace pandora {
  namespace video {
    /// @class SwapChainParams
    /// @brief Swap-chain build params, to create a SwapChain instance (via a 'Renderer' instance: d3d11/openGL4/...).
    /// @remarks The same params can be used to build multiple SwapChain instances.
    class SwapChainParams final {
    public:
      SwapChainParams() = default;
      ~SwapChainParams() = default;
      SwapChainParams(const SwapChainParams&) = default;
      SwapChainParams(SwapChainParams&&) = default;
      SwapChainParams& operator=(const SwapChainParams&) = default;
      SwapChainParams& operator=(SwapChainParams&&) = default;
      
      // -- buffer format --
      
      /// @brief Set color/component format for back-buffers (portable enum binding) -- default: RGBA8 UNORM sRGB
      /// @warning Swap-chains only support display formats: 8-bit normalized formats, HDR10, or 16-bit float formats
      inline SwapChainParams& setBackBufferFormat(pandora::video::ComponentFormat format) noexcept;
      /// @brief Set color/component format for back-buffers (cast from native enum value: DXGI_FORMAT/GLenum/VkFormat)
      inline SwapChainParams& setCustomBackBufferFormat(int32_t format) noexcept;
      
      /// @brief Set number of frame-buffers (back-buffers + front-buffer) -- default: 2 (minimum: 1)
      inline SwapChainParams& setFrameBufferNumber(uint32_t number) noexcept { this->_frameBufferCount = (number) ? number : 2u; return *this; }
      /// @brief Use HDR color space if possible
      /// @warning - HDR only used if supported by monitor
      ///          - HDR only used if fully supported by system API (Direct3D-OpenGL-Vulkan version / operating system)
      ///          - HDR only used with back-buffer formats: RGBA32/RGB32/RG32, RGBA16/RGB16/RG16, RGB10A2/RG11B10
      ///          - Always verify if the swap-chain actually supports HDR after creating it (SwapChain.isHdrEnabled)
      inline SwapChainParams& setHdrPreferred(bool isEnabled) noexcept { this->_isHdrPreferred = isEnabled; return *this; }

      // -- output settings --
      
      /// @brief Set render target output mode
      inline SwapChainParams& setRenderTargetMode(SwapChainTargetMode mode) noexcept { this->_targetMode = mode; return *this; }
      /// @brief Set special swap-chain output settings
      inline SwapChainParams& setOutputFlags(SwapChainOutputFlag flags) noexcept { this->_outputFlags = flags; return *this; }
      
      /// @brief Set refresh rate -- milliHz -- default: 60000
      inline SwapChainParams& setRefreshRate(uint32_t rateMilliHz) noexcept;
      /// @brief Set refresh rate -- Hz rational -- default: 60 / 1
      inline SwapChainParams& setRefreshRate(uint32_t rateNumerator, uint32_t rateDenominator) noexcept;
      
      // -- accessors --
      
      inline pandora::video::ComponentFormat backBufferFormat() const noexcept { return _backBufferFormat; }
      inline int32_t customBackBufferFormat() const noexcept { return _customBackBufferFormat; }
      inline uint32_t frameBufferCount() const noexcept { return _frameBufferCount; }
      inline bool isHdrPreferred() const noexcept { return _isHdrPreferred; }

      inline SwapChainTargetMode renderTargetMode() const noexcept { return _targetMode; }
      inline SwapChainOutputFlag outputFlags() const noexcept { return _outputFlags; }
      inline uint32_t rateNumerator() const noexcept { return _rateNumerator; }
      inline uint32_t rateDenominator() const noexcept { return _rateDenominator; }
      
    private:
      pandora::video::ComponentFormat _backBufferFormat = pandora::video::ComponentFormat::rgba8_sRGB;
      int32_t _customBackBufferFormat = 0;
      uint32_t _frameBufferCount = 2u;
      bool _isHdrPreferred = false;
      
      SwapChainTargetMode _targetMode = SwapChainTargetMode::uniqueOutput;
      SwapChainOutputFlag _outputFlags = SwapChainOutputFlag::none;
      uint32_t _rateNumerator = 60u;
      uint32_t _rateDenominator = 1u;
    };
  }
}

// ---
  
inline pandora::video::SwapChainParams&
pandora::video::SwapChainParams::setBackBufferFormat(pandora::video::ComponentFormat format) noexcept { 
  this->_backBufferFormat = format;
  return *this;
}
inline pandora::video::SwapChainParams&
pandora::video::SwapChainParams::setCustomBackBufferFormat(int32_t format) noexcept { 
  this->_backBufferFormat = pandora::video::ComponentFormat::custom;
  this->_customBackBufferFormat = format;
  return *this;
}

// ---

inline pandora::video::SwapChainParams&
pandora::video::SwapChainParams::setRefreshRate(uint32_t rateMilliHz) noexcept {
  switch (rateMilliHz) {
    case 59940u: this->_rateNumerator = 60000u; this->_rateDenominator = 1001u; break;
    case 29970u: this->_rateNumerator = 30000u; this->_rateDenominator = 1001u; break;
    case 23976u: this->_rateNumerator = 24000u; this->_rateDenominator = 1001u; break;
    default: {
      if ((rateMilliHz % 1000u) == 0) {
        this->_rateNumerator = rateMilliHz/1000u; this->_rateDenominator = 1u;
      }
      else {
        this->_rateNumerator = rateMilliHz; this->_rateDenominator = 1000u;
      }
      break;
    }
  }
  return *this; 
}
inline pandora::video::SwapChainParams&
pandora::video::SwapChainParams::setRefreshRate(uint32_t rateNumerator, uint32_t rateDenominator) noexcept { 
  this->_rateNumerator = rateNumerator;
  this->_rateDenominator = (rateDenominator) ? rateDenominator : 1u; 
  return *this; 
}
