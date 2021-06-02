/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include "./component_format.h"
#include <system/preprocessor_tools.h>

namespace pandora {
  namespace video {
    /// @brief Render target output mode
    enum class RenderTargetMode : uint32_t {
      uniqueOutput      = 0u, ///< Unique display output: must be the only swap-chain of a rendering device (standard usage).
      partialOutput     = 1u  ///< Partial display output: multiple partial swap-chains used (for example, for split-screen with different params).
    };
    /// @brief Advanced swap-chain output params (bit-mask flags)
    /// @warning Most of these features are not supported on old renderers (Direct3D 11.0, OpenGL 4.1...)
    enum class SwapChainOutputFlag : uint32_t {
      none            = 0x0u, ///< No option
      variableRefresh = 0x1u, ///< Required for variable refresh rate display in fullscreen/borderless.
      localOutput     = 0x2u, ///< Restrict to local displays (prevent shared display, remote access, API access...)
      shaderInput     = 0x4u, ///< Allow using swap-chain output as shader input data.
      stereo          = 0x8u  ///< Stereo rendering mode, for 3D devices/glasses (only usable in fullscreen mode)
    };
    /// @brief Output stretching mode
    enum class StretchingMode : uint32_t {
      none         = 0u, ///< Output not stretched: centered/top-left (depending on API) with original buffer size.
      stretch      = 1u, ///< Output stretched to fill render view.
      stretchRatio = 2u  ///< Output stretched to fill one dimension of the render view (keep aspect ratio).
    };
    /// @brief Output buffer transparency mode
    enum class AlphaBlending : uint32_t {
      ignored       = 0u, ///< No transparency
      standard      = 1u, ///< Standard transparency: - rgba(255,0,0,255) == full saturation red, opaque.
                          ///                         - rgba(127,0,0,127) == 50% saturation red, 50% transparent.
                          ///                         - rgba(255,0,0,127) == full saturation red, 50% transparent.
      preMultiplied = 2u  ///< Pre-multiplied transparency: - rgba(255,0,0,255) == full saturation red, opaque.
                          ///                               - rgba(127,0,0,127) == full saturation red, 50% transparent.
                          ///                               - rgba(255,0,0,127) == additive blended red, 50% transparent.
    };
    
    // ---
    
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
      inline SwapChainParams& setBackBufferFormat(pandora::video::ComponentFormat format) noexcept;
      /// @brief Set color/component format for back-buffers (cast from native enum value: DXGI_FORMAT/GLenum/VkFormat)
      inline SwapChainParams& setCustomBackBufferFormat(int32_t format) noexcept;
      
      /// @brief Set depth/component format for depth-buffer (portable enum binding) -- default: DEPTH32 FLOAT
      inline SwapChainParams& setDepthBufferFormat(pandora::video::ComponentFormat format) noexcept;
      /// @brief Set depth/component format for depth-buffer (cast from native enum value: DXGI_FORMAT/GLenum/VkFormat)
      inline SwapChainParams& setCustomDepthBufferFormat(int32_t format) noexcept;
      
      /// @brief Set number of drawing buffers (back-buffers + front-buffer) -- default: 2 (minimum: 1)
      inline SwapChainParams& setDrawingBufferNumber(uint32_t number) noexcept { this->_drawingBufferCount = (number) ? number : 2u; return *this; }
      
      // -- output settings --
      
      /// @brief Set render target output mode
      inline SwapChainParams& setRenderTargetMode(RenderTargetMode mode) noexcept { this->_targetMode = mode; return *this; }
      /// @brief Set special swap-chain output settings
      inline SwapChainParams& setOutputFlags(SwapChainOutputFlag flags) noexcept { this->_outputFlags = flags; return *this; }
      /// @brief Set output stretching mode
      inline SwapChainParams& setStretchingMode(StretchingMode mode) noexcept { this->_stretchingMode = mode; return *this; }
      /// @brief Set transparency mode for rendering buffers
      inline SwapChainParams& setAlphaBlending(AlphaBlending mode) noexcept { this->_alphaBlending = mode; return *this; }
      /// @brief Set multi-sample anti-aliasing mode (1: disabled / 2-4-8: MSAA 2x-4x-8x)
      inline SwapChainParams& setMultisample(uint32_t sampleNumber) noexcept { this->_multiSampleCount = (sampleNumber) ? sampleNumber : 1u; return *this; }
      
      /// @brief Set refresh rate -- milliHz -- default: 60000
      inline SwapChainParams& setRefreshRate(uint32_t rateMilliHz) noexcept;
      /// @brief Set refresh rate -- Hz rational -- default: 60 / 1
      inline SwapChainParams& setRefreshRate(uint32_t rateNumerator, uint32_t rateDenominator) noexcept;
      
      // -- accessors --
      
      inline pandora::video::ComponentFormat backBufferFormat() const noexcept { return _backBufferFormat; }
      inline int32_t customBackBufferFormat() const noexcept { return _customBackBufferFormat; }
      inline pandora::video::ComponentFormat depthBufferFormat() const noexcept { return _depthBufferFormat; }
      inline int32_t customDepthBufferFormat() const noexcept { return _customDepthBufferFormat; }
      inline uint32_t drawingBufferCount() const noexcept { return _drawingBufferCount; }

      inline RenderTargetMode renderTargetMode() const noexcept { return _targetMode; }
      inline SwapChainOutputFlag outputFlags() const noexcept { return _outputFlags; }
      inline StretchingMode stretchingMode() const noexcept { return _stretchingMode; }
      inline AlphaBlending alphaBlending() const noexcept { return _alphaBlending; }
      inline uint32_t multisampleCount() const noexcept { return _multiSampleCount; }
      inline uint32_t rateNumerator() const noexcept { return _rateNumerator; }
      inline uint32_t rateDenominator() const noexcept { return _rateDenominator; }
      
    private:
      pandora::video::ComponentFormat _backBufferFormat = pandora::video::ComponentFormat::rgba8_sRGB;
      int32_t _customBackBufferFormat = 0;
      pandora::video::ComponentFormat _depthBufferFormat = pandora::video::ComponentFormat::d32_f;
      int32_t _customDepthBufferFormat = 0;
      uint32_t _drawingBufferCount = 2u;
      
      RenderTargetMode _targetMode = RenderTargetMode::uniqueOutput;
      SwapChainOutputFlag _outputFlags = SwapChainOutputFlag::none;
      StretchingMode _stretchingMode = StretchingMode::stretch;
      AlphaBlending _alphaBlending = AlphaBlending::standard;
      uint32_t _multiSampleCount = 1u;
      uint32_t _rateNumerator = 60u;
      uint32_t _rateDenominator = 1u;
    };
  }
}
_P_FLAGS_OPERATORS(pandora::video::SwapChainOutputFlag, uint32_t);

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

inline pandora::video::SwapChainParams&
pandora::video::SwapChainParams::setDepthBufferFormat(pandora::video::ComponentFormat format) noexcept { 
  this->_depthBufferFormat = format;
  return *this; 
}
inline pandora::video::SwapChainParams&
pandora::video::SwapChainParams::setCustomDepthBufferFormat(int32_t format) noexcept { 
  this->_depthBufferFormat = pandora::video::ComponentFormat::custom;
  this->_customDepthBufferFormat = format;
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
