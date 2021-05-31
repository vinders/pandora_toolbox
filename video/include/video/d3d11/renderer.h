/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <vector>
# include "../window_handle.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        using DxgiFactory = void*;   // IDXGIFactory1*
        using Device = void*;        // ID3D11Device*
        using DeviceContext = void*; // ID3D11DeviceContext*
        
        /// @brief Direct3D feature level
        enum class RendererDeviceLevel : uint32_t {
          direct3D_11_0 = 0, ///< 11.0
          direct3D_11_1 = 1  ///< 11.1+
        };
        
        // ---
        
        /// @class Renderer
        /// @brief Direct3D rendering device and swap-chain
        /// @remarks - The renderer should be instanced before any other Direct3D resource, and kept alive while the window exists.
        ///          - Before trying to display anything, a swap-chain must be created (by calling 'addSwapChain')
        ///          - Additional swap-chains can be created, when using multiple render targets (car games with rear/side mirrors, split-screen...)
        /// @warning - The renderer/swap-chain destruction closes Direct3D context -> it should be done at the end of the program.
        ///          - Accessors to D3D11 resources should only be called to use of special features (not accessible in the class)
        class Renderer final {
        public:
          /// @brief Create Direct3D device and context
          /// @param minLevel  The system tries to use the highest available device level (based on Cmake/cwork options).
          ///                  If some feature level is not available, the level below is used (and so on).
          ///                  Argument 'minLevel' specifies the minimum level allowed.
          /// @throws exception on failure
          Renderer(RendererDeviceLevel minLevel = RendererDeviceLevel::direct3D_11_0) { createDeviceResources((uint32_t)minLevel); }
          /// @brief Destroy device and context resources
          ~Renderer() noexcept;
          
          inline Device device() const noexcept { return this->_device; }          ///< Get Direct3D rendering device (cast to 'ID3D11Device*')
          inline DeviceContext context() const noexcept { return this->_context; } ///< Get Direct3D device context (cast to 'ID3D11DeviceContext*')
          inline uint32_t dxgiLevel() const noexcept { return this->_dxgiLevel; }  ///< Get available level of DXGI on current system (1-6)
          inline RendererDeviceLevel featureLevel() const noexcept { return this->_deviceLevel; } ///< Get available feature level on current device (11.0/11.1)

          // -- feature support --
          
          /// @brief Verify if HDR is supported (flip swap effects)
          inline bool isHdrAvailable() const noexcept { return (this->_dxgiLevel >= 5u); }
          /// @brief Verify if buffer "flip swap mode" is supported
          inline bool isFlipAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          /// @brief Verify if screen tearing is supported
          inline bool isTearingAvailable() const noexcept { return (this->_dxgiLevel >= 5u); }
          /// @brief Verify if a multisample mode is supported (anti-aliasing)
          /// @param sampleCount  Number of samples: 1, 4 or 8
          bool isMultisampleSupported(uint32_t sampleCount, uint32_t& outMaxQualityLevel) const noexcept;

        private:
          void createDeviceResources(uint32_t minLevel);
        
        private:
          Device _device = nullptr;           // ID3D11Device*
          DeviceContext _context = nullptr;   // ID3D11DeviceContext*
          DxgiFactory _dxgiFactory = nullptr; // IDXGIFactory1*
          RendererDeviceLevel _deviceLevel = RendererDeviceLevel::direct3D_11_1;
          uint32_t _dxgiLevel = 1;
        };
      }
    }
  }
#endif
