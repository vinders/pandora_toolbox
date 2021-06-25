/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./renderer_state.h" // includes D3D11
# include "./renderer.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class RendererStateFactory final {
        public:
          RendererStateFactory(Renderer& renderer) : _device(renderer.device()) {}
          RendererStateFactory(const RendererStateFactory&) = default;
          RendererStateFactory(RendererStateFactory&&) noexcept = default;
          RendererStateFactory& operator=(const RendererStateFactory&) = default;
          RendererStateFactory& operator=(RendererStateFactory&&) noexcept = default;
          ~RendererStateFactory() noexcept = default;
          
          
          // -- depth/stencil state buffer --
          
          /// @brief Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          /// @param frontFaceOp  Operations to perform on front-facing pixels of depth buffer, based on depth test result
          /// @param backFaceOp   Operations to perform on back-facing pixels of depth buffer, based on depth test result
          /// @param depthTest    Type of comparison for depth test
          /// @param writeMaskAll Depth write mask: all (true) or none (false)
          /// @throws runtime_error on creation failure
          DepthStencilState createDepthTestState(const DepthOperationGroup& frontFaceOp, const DepthOperationGroup& backFaceOp,
                                                 DepthComparison depthTest = DepthComparison::less, bool writeMaskAll = true);
          /// @brief Create stencil test state (disable depth test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          /// @param frontFaceOp  Operations to perform on front-facing pixels of stencil buffer, based on stencil test result
          /// @param backFaceOp   Operations to perform on back-facing pixels of stencil buffer, based on stencil test result
          /// @param readMask     Portion of depth/stencil buffer to read
          /// @param writeMask    Portion of depth/stencil buffer to write
          /// @throws runtime_error on creation failure
          DepthStencilState createStencilTestState(const DepthStencilOperationGroup& frontFaceOp, const DepthStencilOperationGroup& backFaceOp, 
                                                   uint8_t readMask = 0xFF, uint8_t writeMask = 0xFF);
          /// @brief Create depth/stencil test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          /// @param frontFaceOp       Operations to perform on front-facing pixels of depth/stencil buffer, based on depth/stencil test results
          /// @param backFaceOp        Operations to perform on back-facing pixels of depth/stencil buffer, based on depth/stencil test results
          /// @param depthTest         Type of comparison for depth test
          /// @param depthWriteMaskAll Depth write mask: all (true) or none (false)
          /// @param stencilReadMask   Portion of depth/stencil buffer to read
          /// @param stencilWriteMask  Portion of depth/stencil buffer to write
          /// @throws runtime_error on creation failure
          DepthStencilState createDepthStencilTestState(const DepthStencilOperationGroup& frontFaceOp, const DepthStencilOperationGroup& backFaceOp, 
                                                        DepthComparison depthTest = DepthComparison::less, bool depthWriteMaskAll = true, 
                                                        uint8_t stencilReadMask = 0xFF, uint8_t stencilWriteMask = 0xFF);
          
          
          // -- rasterizer state buffer --
          
          /// @brief Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
          /// @param culling           Identify shapes to draw: front-facing, back-facing, both.
          /// @param isFrontClockwise  Choose vertex order of front-facing shapes
          /// @param depthBias         Depth-bias settings (depth control, Z-order, clipping)
          /// @param scissorClipping   Enable scissor-rectangle clipping
          /// @throws runtime_error on creation failure
          RasterizerState createRasterizerState(pandora::video::CullMode culling, bool isFrontClockwise, 
                                                const pandora::video::DepthBias& depthBias, bool scissorClipping = false);
          
          
          // -- blend state buffer --
          
          //...


          // -- sampler filter state buffer --

          /// @brief Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateContainer  RAII container in which to insert/append new state item.
          /// @param minFilter          Filter to use for minification (downscaling).
          /// @param magFilter          Filter to use for magnification (upscaling).
          /// @param texAddressUVW      Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias            Offset from calculated mip-map level (added).
          /// @param borderColor        Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0] - NULL to use default (black).
          ///                           Color should be gamma correct (see Renderer.toGammaCorrectColor).
          /// @throws runtime_error on creation failure
          FilterState createFilter(MinificationFilter minFilter, MagnificationFilter magFilter, 
                                   const TextureAddressMode texAddressUVW[3], float lodMin = 0.0, float lodMax = 0.0, 
                                   float lodBias = 0.0, const FLOAT borderColor[4] = nullptr);
          /// @brief Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateContainer  RAII container in which to insert/append new state item.
          /// @param minFilter          Filter to use for minification (downscaling).
          /// @param magFilter          Filter to use for magnification (upscaling).
          /// @param texAddressUVW      Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param compare            Depth comparison mode with existing reference pixels.
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias            Offset from calculated mip-map level (added).
          /// @param borderColor        Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0] - NULL to use default (black).
          ///                           Color should be gamma correct (see Renderer.toGammaCorrectColor).
          /// @throws runtime_error on creation failure
          FilterState createComparedFilter(MinificationFilter minFilter, MagnificationFilter magFilter, 
                                           const TextureAddressMode texAddressUVW[3], DepthComparison compare, 
                                           float lodMin = 0.0, float lodMax = 0.0, float lodBias = 0.0,
                                           const FLOAT borderColor[4] = nullptr);
          
          // ---
          
          /// @brief Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateContainer  RAII container in which to insert/append new state item.
          /// @param maxAnisotropy      Clamping anisotropy value - range: [1;maxAnisotropy()].
          /// @param texAddressUVW      Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias            Offset from calculated mip-map level (added).
          /// @param borderColor        Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0] - NULL to use default (black).
          ///                           Color should be gamma correct (see Renderer.toGammaCorrectColor).
          /// @throws runtime_error on creation failure
          FilterState createAnisotropicFilter(uint32_t maxAnisotropy, const TextureAddressMode texAddressUVW[3], 
                                              float lodMin = 0.0, float lodMax = 0.0, float lodBias = 0.0, 
                                              const FLOAT borderColor[4] = nullptr);
          /// @brief Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateCont  RAII container in which to insert/append new state item.
          /// @param maxAniso      Clamping anisotropy value - range: [1;maxAnisotropy()].
          /// @param txAddrUVW     Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param compare       Depth comparison mode with existing reference pixels.
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias       Offset from calculated mip-map level (added).
          /// @param borderColor   Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0] - NULL to use default (black).
          ///                           Color should be gamma correct (see Renderer.toGammaCorrectColor).
          /// @throws runtime_error on creation failure
          FilterState createComparedAnisotropicFilter(uint32_t maxAnisotropy, const TextureAddressMode txAddrUVW[3], 
                                                      DepthComparison compare, float lodMin = 0.0, float lodMax = 0.0, float lodBias = 0.0, 
                                                      const FLOAT borderColor[4] = nullptr);
          
          /// @brief Max anisotropy value (usually 8 or 16)
          static constexpr inline uint32_t maxAnisotropy() noexcept { return (uint32_t)D3D11_MAX_MAXANISOTROPY; }

        private:
          Renderer::DeviceHandle _device = nullptr;
        };
      }
    }
  }

#endif
