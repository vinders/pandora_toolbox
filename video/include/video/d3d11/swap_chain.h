/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <memory>
# include "../swap_chain_params.h"
# include "../window_handle.h"
# include "./renderer.h" // includes D3D11
# include "./_private/_swap_chain_config.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class SwapChain
        /// @brief Direct3D rendering swap-chain (framebuffers)
        /// @warning - SwapChain is the Direct3D display output tool, and should be kept alive while the window exists.
        ///          - If the window is re-created, a new SwapChain must be created.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new Renderer with a new SwapChain must be created.
        ///          - handle() and handleLevel1() should be reserved for internal usage or for advanced features.
        /// @remarks - To render on display output, call setActiveRenderTarget on Renderer with getRenderTargetView() and a depth buffer.
        ///          - Swap-chains are meant for display: to "render to texture", use TextureBuffer.
        ///          - Multi-window rendering (same adapter): alternate between different SwapChain instances on the same Renderer.
        ///          - Multi-window rendering (different adapters): use different Render instances with their own SwapChain.
        ///          - Split-screen rendering (same window): alternate between different Viewport instances on the same SwapChain.
        class SwapChain final {
        public:
          using Handle = void*; // IDXGISwapChain*/IDXGISwapChain1*
        
          /// @brief Create rendering swap-chain for existing renderer
          /// @throws - invalid_argument: if renderer/window/width/height is 0/NULL.
          ///         - runtime_error: creation failure.
          SwapChain(std::shared_ptr<Renderer> renderer, const pandora::video::SwapChainParams& params, 
                    pandora::video::WindowHandle window, uint32_t width, uint32_t height);
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
          
          /// @brief Get native Direct3D 11.0 compatible swap-chain handle (cast to IDXGISwapChain*)
          inline Handle handle() const noexcept { return this->_swapChain; }
          /// @brief Get native Direct3D 11.1+ swap-chain handle, if available (cast to IDXGISwapChain1*)
          /// @returns Handle for Direct3D 11.1+ devices (or NULL for Direct3D 11.0 devices).
          inline Handle handleLevel1() const noexcept { 
            return (this->_swapChainLevel != D3D_FEATURE_LEVEL_11_0) ? this->_swapChain : nullptr;
          }
          inline bool isEmpty() const noexcept { return (this->_swapChain == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          
          /// @brief Get render-target view of current SwapChain
          /// @remarks - This value should be used to call 'Renderer.setActiveRenderTargets'.
          ///          - Activating the render-target is necessary to draw into SwapChain back-buffers.
          inline Renderer::RenderTargetViewHandle getRenderTargetView() const noexcept { return this->_renderTargetView; }
          
          inline uint32_t width() const noexcept  { return this->_settings.width; } ///< Get current swap-chain width
          inline uint32_t height() const noexcept { return this->_settings.height; }///< Get current swap-chain height
          /// @brief Verify if HDR is enabled in current swap-chain (should be verified before using HDR shaders and data)
          /// @remarks HDR might be disabled even if it was enabled in creation params:
          ///          - if the adapter or monitor doesn't support it.
          ///          - if the rendering API level is too old to support it.
          ///          - if the buffer color/component format isn't compatible with it.
          inline bool isHdrEnabled() const noexcept {
            return (this->_settings.colorSpace == (int32_t)DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
                 || this->_settings.colorSpace == (int32_t)DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709);
          }

          // -- operations --
          
          /// @brief Change back-buffer(s) size + refresh color space
          /// @remarks Must be called when the window size changes, or when the display monitor is different.
          /// @warning - A new depth buffer (with the same size) should be created.
          ///          - Disables all active render-targets of Renderer -> they need to be activated again! ('Renderer.setActiveRenderTargets')
          /// @returns True if buffers/active-render-target must be reconfigured / false if only color-space has been updated.
          /// @throws - invalid_argument: if width/height is 0 -> retry with valid arguments.
          ///         - domain_error: if new monitor uses a different adapter -> a new Renderer (with a new SwapChain) must be created (using current may crash).
          ///         - runtime_error: resize failure -> a new SwapChain must be created (using current swap-chain again may crash).
          bool resize(uint32_t width, uint32_t height);
          
          /// @brief Swap back-buffer(s) and front-buffer, to display drawn content on screen
          /// @param useVsync  Wait for adapter vsync signal (avoids screen tearing, but may cause up to 1 frame of delay before display).
          /// @throws - domain_error: if the device has been lost (disconnected/switched/updated) -> recreate Renderer and SwapChain!
          ///         - invalid_argument: SwapChainOutputFlag::localOutput was set but is not supported -> recreate SwapChain without it!
          ///         - runtime_error: internal GPU/device/network error -> if the error repeats itself, recreate Renderer and SwapChain!
          void swapBuffers(bool useVsync);
          /// @brief Swap back-buffer(s) and front-buffer, to display drawn content on screen + discard render-target/depth-stencil view content after it
          /// @param useVsync  Wait for adapter vsync signal (avoids screen tearing, but may cause up to 1 frame of delay before display).
          /// @remarks - Efficient way to release GPU resources -> should be called instead of 'swapBuffers' when each pixel is redrawn for each frame.
          ///          - If only part of the content is rewritten, and view must be cleared: use 'swapBuffer' + 'Renderer.clear'.
          ///          - If only part of the content is rewritten, and previous view content must be kept: only use 'swapBuffer'.
          ///          - Discard feature only supported in Direct3D 11.1+ -> 'swapBuffersDiscard' is the same as 'swapBuffers' with API level 11.0.
          /// @throws - domain_error: if the device has been lost (disconnected/switched/updated) -> recreate Renderer and SwapChain!
          ///         - invalid_argument: SwapChainOutputFlag::localOutput was set but is not supported -> recreate SwapChain without it!
          ///         - runtime_error: internal GPU/device/network error -> if the error repeats itself, recreate Renderer and SwapChain!
          void swapBuffersDiscard(bool useVsync, Renderer::DepthStencilViewHandle depthBuffer = nullptr);
        
        
        private:
          void _createSwapChainTargetView(); // throws
          static void _convertSwapChainParams(const Renderer& renderer, const pandora::video::SwapChainParams& params, 
                                              _SwapChainConfig& outConfig) noexcept;
        private:
          _SwapChainConfig _settings{};
          std::shared_ptr<Renderer> _renderer = nullptr;
          Renderer::RenderTargetViewHandle _renderTargetView = nullptr;
#         if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
            ID3D11DeviceContext1* _deviceContext11_1 = nullptr;
#         endif

          Handle _swapChain = nullptr; // IDXGISwapChain*/IDXGISwapChain1*
          D3D_FEATURE_LEVEL _swapChainLevel = D3D_FEATURE_LEVEL_11_0;
          uint32_t _presentFlags = 0; // flags used when swapping buffers
        };
      }
    }
  }

#endif
