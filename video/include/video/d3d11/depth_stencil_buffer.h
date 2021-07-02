/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./renderer.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class DepthStencilBuffer
        /// @brief Direct3D depth/stencil fragment verification buffer/view for output merger
        /// @warning - If the render-target is resized, a new DepthStencilBuffer must be created.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new DepthStencilBuffer must be created.
        /// @remarks - To use a depth/stencil buffer, call setActiveRenderTarget on Renderer with getDepthStencilView() and a target render view (same size).
        ///          - The same depth/stencil buffer may only be used with multiple/different render-targets if they have the same size.
        class DepthStencilBuffer final {
        public:
          /// @brief Create depth/stencil buffer for existing renderer/render-target
          /// @warning Params 'width'/'height' should be the same as the associated render-target size (swap-chain/texture buffer).
          /// @throws - invalid_argument: if width/height is 0.
          ///         - runtime_error: creation failure.
          DepthStencilBuffer(Renderer& renderer, DepthStencilFormat format, uint32_t width, uint32_t height);
          ~DepthStencilBuffer() noexcept { release(); }
          /// @brief Destroy depth/stencil buffer
          void release() noexcept;
          
          DepthStencilBuffer() = default; ///< Empty depth/stencil buffer -- not usable (only useful to store variable not immediately initialized)
          DepthStencilBuffer(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept;
          DepthStencilBuffer& operator=(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer& operator=(DepthStencilBuffer&& rhs) noexcept;
        
          // -- accessors --
          
          /// @brief Get native Direct3D 11 compatible depth/stencil buffer handle (cast to ID3D11Texture2D*)
          inline TextureHandle2D handle() const noexcept { return this->_depthStencilBuffer; }
          inline bool isEmpty() const noexcept { return (this->_depthStencilBuffer == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          
          inline uint32_t width() const noexcept  { return _width(); } ///< Get depth/stencil buffer width
          inline uint32_t height() const noexcept { return _height(); }///< Get depth/stencil buffer height
          inline DepthStencilFormat getFormat() const noexcept { return this->_format; } ///< Get depth/stencil buffer data format
          
          /// @brief Get depth/stencil view of current DepthStencilBuffer
          /// @remarks - This value should be used to call 'Renderer.setActiveRenderTargets'.
          ///          - Activating the depth/stencil view is necessary for depth/stencil testing.
          inline DepthStencilView getDepthStencilView() const noexcept { return this->_depthStencilView; }


        private:
          inline uint32_t _width() const noexcept { return (this->_pixelSize & 0xFFFFu); }
          inline uint32_t _height() const noexcept { return (this->_pixelSize >> 16); }
          static constexpr inline uint32_t _toPixelSize(uint32_t width, uint32_t height) noexcept { return (width | (height << 16)); }
          
        private:
          DepthStencilView _depthStencilView = nullptr;  // ID3D11DepthStencilView*
          TextureHandle2D _depthStencilBuffer = nullptr; // ID3D11Texture2D*
          uint32_t _pixelSize; // width / height
          DepthStencilFormat _format;
        };
      }
    }
  }

#endif
