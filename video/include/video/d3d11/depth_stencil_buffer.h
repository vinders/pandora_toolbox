/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./_private/_depth_stencil_buffer_config.h"
# include "./renderer.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class DepthStencilBuffer
        /// @brief Direct3D depth/stencil fragment verification buffer/view for output merger
        /// @warning - If the render-target is resized, a new DepthStencilBuffer must be created.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new DepthStencilBuffer must be created.
        ///          - handle() should be reserved for internal usage or for advanced features.
        /// @remarks - To use a depth/stencil buffer, call setActiveRenderTarget on Renderer with getDepthStencilView() and a target render view (same size).
        ///          - The same depth/stencil buffer may only be used with multiple/different render-targets if they have the same size.
        class DepthStencilBuffer final {
        public:
          using Handle = void*; // ID3D11Texture2D*
        
          /// @brief Create depth/stencil buffer for existing renderer/render-target
          /// @warning - Params 'width'/'height' should be the same as the associated render-target (swap-chain/texture buffer).
          ///          - Param 'format' should be a depth/stencil format (d32_f, d32_f_s8_ui, d24_unorm_s8_ui, d16_unorm).
          /// @throws - invalid_argument: if width/height is 0.
          ///         - runtime_error: creation failure.
          DepthStencilBuffer(Renderer& renderer, pandora::video::ComponentFormat format, 
                             uint32_t width, uint32_t height);
          /// @brief Destroy depth/stencil buffer
          /// @warning If still drawing, the depth/stencil buffer should be unbound before destroying it ('Renderer.setActiveRenderTargets' with NULL or other buffer).
          ~DepthStencilBuffer() noexcept { release(); }
          /// @brief Destroy depth/stencil buffer
          /// @warning If still drawing, the depth/stencil buffer should be unbound before destroying it ('Renderer.setActiveRenderTargets' with NULL or other buffer).
          void release() noexcept;
          
          DepthStencilBuffer() = default; ///< Empty depth/stencil buffer -- not usable (only useful to store variable not immediately initialized)
          DepthStencilBuffer(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept;
          DepthStencilBuffer& operator=(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer& operator=(DepthStencilBuffer&& rhs) noexcept;
        
          // -- accessors --
          
          /// @brief Get native Direct3D 11.0 compatible depth/stencil buffer handle (cast to ID3D11Texture2D*)
          inline Handle handle() const noexcept { return this->_depthStencilBuffer; }
          inline bool isEmpty() const noexcept { return (this->_depthStencilBuffer == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          
          inline uint32_t width() const noexcept  { return this->_settings.width; } ///< Get depth/stencil buffer width
          inline uint32_t height() const noexcept { return this->_settings.height; }///< Get depth/stencil buffer height
          /// @brief Get depth/stencil buffer data format
          pandora::video::ComponentFormat getFormat() const noexcept { return this->_settings.format; }
          
          /// @brief Get depth/stencil view of current DepthStencilBuffer
          /// @remarks - This value should be used to call 'Renderer.setActiveRenderTargets'.
          ///          - Activating the depth/stencil view is necessary for depth/stencil testing.
          Renderer::DepthStencilViewHandle getDepthStencilView() const noexcept { return this->_depthStencilView; }

        private:
          Renderer::DepthStencilViewHandle _depthStencilView = nullptr; // ID3D11DepthStencilView*
          Handle _depthStencilBuffer = nullptr;                         // ID3D11Texture2D*
          _DepthStencilBufferConfig _settings{};
        };
      }
    }
  }

#endif
