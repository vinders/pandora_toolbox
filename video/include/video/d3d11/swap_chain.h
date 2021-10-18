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

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <memory>
# include "../window_handle.h"
# include "../common_types.h"
# include "./api/types.h" // includes D3D11
# include "./renderer.h"  // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class SwapChain;

        /// @class DisplaySurface
        /// @brief Direct3D output surface of a window - tied to a Window instance and used to create a swap-chain
        /// @warning Only one DisplaySurface instance is allowed for the same Window.
        class DisplaySurface final {
        public:
          /// @brief Create output surface for a swap-chain
          /// @throws invalid_argument: if 'renderer' or 'window' is NULL.
          DisplaySurface(std::shared_ptr<Renderer> renderer, pandora::video::WindowHandle window)
            : _renderer(std::move(renderer)), _window(window) {
            if (this->_renderer == nullptr || this->_window == nullptr)
              throw std::invalid_argument("DisplaySurface: NULL renderer/window");
          }
          ~DisplaySurface() noexcept = default;

          DisplaySurface(const DisplaySurface&) = delete;
          DisplaySurface(DisplaySurface&&) noexcept = default;
          DisplaySurface& operator=(const DisplaySurface&) = delete;
          DisplaySurface& operator=(DisplaySurface&&) noexcept = default;

          /// @brief Verify if a buffer format is supported to create swap-chains and render targets
          /// @remarks Can be used to verify HDR support by API/hardware
          bool isFormatSupported(DataFormat bufferFormat) const noexcept;

        private:
          friend class SwapChain;
          std::shared_ptr<Renderer> _renderer = nullptr;
          pandora::video::WindowHandle _window = (pandora::video::WindowHandle)0;
        };

        // ---

        /// @class SwapChain
        /// @brief Direct3D rendering swap-chain (framebuffers) - tied to a Window instance
        /// @warning - SwapChain is the Direct3D display output tool, and should be kept alive while the window exists.
        ///          - All SwapChains should be created before any Pipeline object.
        ///          - If the window is re-created, a new SwapChain must be created.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new Renderer with a new SwapChain must be created.
        ///          - handle() and handleExt() should be reserved for internal usage or for advanced features.
        /// @remarks - To render on display output, call 'Renderer.setActiveRenderTarget' with getRenderTargetView() value + optional depth buffer.
        ///          - Swap-chains are meant for display. To "render to texture", use TextureTarget2D instead.
        ///          - Multi-window rendering (same adapter): alternate between different SwapChain instances on the same Renderer.
        ///          - Multi-window rendering (different adapters): use different Render instances with their own SwapChain.
        ///          - Split-screen rendering (same window): alternate between different Viewport instances on the same SwapChain.
        class SwapChain final {
        public:
          using OutputFlag = pandora::video::TargetOutputFlag;
          using RefreshRate = pandora::video::RefreshRate;
          using Descriptor = SwapChainDescriptor;
        
          /// @brief Create rendering swap-chain for existing renderer
          /// @throws runtime_error: creation failure.
          SwapChain(DisplaySurface&& surface, const Descriptor& params, DataFormat backBufferFormat = DataFormat::rgba8_sRGB)
            : _flags(params.outputFlags),
              _pixelSize(_toPixelSize(params.width, params.height)),
              _framebufferCount(params.framebufferCount ? params.framebufferCount : 2u),
              _backBufferFormat(_getDataFormatComponents(backBufferFormat)),
              _renderer(std::move(surface._renderer)) {
            _createSwapChain(surface._window, params.refreshRate); // throws
            _createOrRefreshTargetView(); // throws
          }
          /// @brief Destroy swap-chain
          /// @warning Disables all active render-targets of Renderer -> they need to be activated again! ('Renderer.setActiveRenderTargets')
          ~SwapChain() noexcept { release(); }
          /// @brief Destroy swap-chain
          /// @warning Disables all active render-targets of Renderer -> they need to be activated again! ('Renderer.setActiveRenderTargets')
          void release() noexcept;
          
          SwapChain() = default; ///< Empty swap-chain -- not usable (only useful to store variable not immediately initialized)
          SwapChain(const SwapChain&) = delete;
          SwapChain(SwapChain&& rhs) noexcept;
          SwapChain& operator=(const SwapChain&) = delete;
          SwapChain& operator=(SwapChain&& rhs) noexcept;
        
          // -- accessors --
          
          /// @brief Get native Direct3D 11.0+ compatible swap-chain handle (cast to IDXGISwapChain*)
          inline SwapChainHandle handle() const noexcept { return this->_swapChain; }
          /// @brief Get native Direct3D 11.1+ swap-chain handle, if available (cast to IDXGISwapChain1*)
          /// @returns Handle for Direct3D 11.1+ devices (or NULL for Direct3D 11.0 devices).
          inline SwapChainHandle handleExt() const noexcept { 
            return (this->_renderer->_dxgiLevel >= 2u) ? this->_swapChain : nullptr;
          }
          inline bool isEmpty() const noexcept { return (this->_swapChain == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          
          /// @brief Get render-target view of current SwapChain
          /// @remarks - This value should be used to call 'Renderer.setActiveRenderTargets'.
          ///          - Activating the render-target is necessary to draw into SwapChain back-buffers.
          inline RenderTargetView getRenderTargetView() const noexcept { return this->_renderTargetView; }
          
          inline uint32_t width() const noexcept  { return _width(); } ///< Get current swap-chain width
          inline uint32_t height() const noexcept { return _height(); }///< Get current swap-chain height

          // -- operations --
          
          /// @brief Change back-buffer(s) size
          /// @remarks Must be called when the window size changes, or when the display monitor is different.
          /// @warning - A new depth buffer (with the same size) should be created.
          ///          - Disables all active render-targets of Renderer -> they need to be activated again! ('Renderer.setActiveRenderTargets')
          /// @returns True if buffers/active-render-target must be reconfigured / false if nothing has changed.
          /// @throws - domain_error: if new monitor uses a different adapter -> a new Renderer (with a new SwapChain) must be created (using current may crash).
          ///         - runtime_error: resize failure or incompatible monitor -> a new SwapChain must be created (using current swap-chain again may crash).
          bool resize(uint32_t width, uint32_t height);
          
          /// @brief Swap back-buffer(s) and front-buffer, to display drawn content on screen
          /// @param useVsync     Wait for adapter vsync signal (avoids screen tearing, but may cause up to 1 frame of delay before display).
          /// @param depthBuffer  Depth buffer associated with swap-chain, for resource cleanup.
          /// @throws - domain_error: if the device has been lost (disconnected/switched/updated) -> recreate Renderer and SwapChain!
          ///         - runtime_error: internal GPU/device/network error -> if the error repeats itself, recreate Renderer and SwapChain!
          void swapBuffers(bool useVsync, DepthStencilView depthBuffer = nullptr);
        
        
        private:
          void _createSwapChain(pandora::video::WindowHandle window, const RefreshRate& rate); // throws
          void _createOrRefreshTargetView(); // throws
          
          inline uint32_t _width() const noexcept { return (this->_pixelSize & 0xFFFFu); }
          inline uint32_t _height() const noexcept { return (this->_pixelSize >> 16); }
          static constexpr inline uint32_t _toPixelSize(uint32_t width, uint32_t height) noexcept { return (width | (height << 16)); }
          
        private:
          SwapChainHandle _swapChain = nullptr; // IDXGISwapChain*/IDXGISwapChain1*
          UINT _tearingSwapFlags = 0;           // flags used for screen tearing during swap
          
          OutputFlag _flags = OutputFlag::none; // advanced settings (various params + flip-swap on/off)
          uint32_t _pixelSize = 0;              // width / height
          uint32_t _framebufferCount = 0;       // framebuffer count
          DXGI_FORMAT _backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
          
          std::shared_ptr<Renderer> _renderer = nullptr;
          RenderTargetView _renderTargetView = nullptr;
          void* _deviceContext11_1 = nullptr; // ID3D11DeviceContext1* - only used if supported
        };
      }
    }
  }

#endif
