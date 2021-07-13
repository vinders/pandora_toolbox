/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./renderer.h"       // includes D3D11
# include "./renderer_state.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // -- rasterizer state params --
        
        /// @class RasterizerParams
        /// @brief Rasterizer state configuration (filling/culling/clipping/bias params)
        /// @remarks The same RasterizerParams can (and should) be used to build multiple RasterizerState instances (if needed).
        class RasterizerParams final {
        public:
          /// @brief Create default rasterizer params: filled, back-face culling, clockwise order, depth clipping.
          RasterizerParams() noexcept;
          /// @brief Initialize rasterizer config params
          /// @param cull              Identify polygons to hide: back-facing, front-facing, none.
          /// @param fill              Filled/wireframe polygon rendering.
          /// @param isFrontClockwise  Choose vertex order of front-facing polygons (true = clockwise / false = counter-clockwise)
          /// @param depthClipping     Enable clipping based on distance
          /// @param scissorClipping   Enable scissor-rectangle clipping
          RasterizerParams(CullMode cull, FillMode fill = FillMode::fill, bool isFrontClockwise = true, 
                           bool depthClipping = true, bool scissorClipping = false) noexcept;
          
          RasterizerParams(const RasterizerParams&) = default;
          RasterizerParams& operator=(const RasterizerParams&) = default;
          ~RasterizerParams() noexcept = default;
          
          /// @brief Choose vertex order of front-facing polygons (true = clockwise / false = counter-clockwise)
          inline RasterizerParams& vertexOrder(bool isFrontClockwise) noexcept { _params.FrontCounterClockwise = isFrontClockwise ? FALSE : TRUE; return *this; }
          /// @brief Identify polygons to hide: back-facing, front-facing, none.
          inline RasterizerParams& cullMode(CullMode cull) noexcept { _params.CullMode = (D3D11_CULL_MODE)cull; return *this; }
          /// @brief Set filled/wireframe polygon rendering
          inline RasterizerParams& fillMode(FillMode fill) noexcept {
            if (fill == FillMode::linesAA) { _params.FillMode = D3D11_FILL_WIREFRAME;  _params.AntialiasedLineEnable = TRUE; }
            else                           { _params.FillMode = (D3D11_FILL_MODE)fill; _params.AntialiasedLineEnable = FALSE; }
            return *this;
          }
          
          /// @brief Enable clipping based on distance
          inline RasterizerParams& depthClipping(bool isEnabled) noexcept { _params.DepthClipEnable = isEnabled ? TRUE : FALSE; return *this; }
          /// @brief Enable scissor-rectangle clipping
          inline RasterizerParams& scissorClipping(bool isEnabled) noexcept { _params.ScissorEnable = isEnabled ? TRUE : FALSE; return *this; }
          
          /// @brief Set depth offset factor (used to improve shadow maps + to avoid depth artifacts and self-shadowing)
          ///< @param depthBias            Scalar factor controlling the constant depth value added to each fragment
          ///< @param depthBiasClamp       Maximum (or minimum) depth bias of a fragment.
          ///< @param depthBiasSlopeFactor Scalar factor applied to a fragment’s slope. 
          inline RasterizerParams& depthBias(int32_t depthBias, float depthBiasClamp = 0.f, float depthBiasSlopeFactor = 0.f) noexcept { 
            _params.DepthBias = (INT)depthBias;
            _params.DepthBiasClamp = (FLOAT)depthBiasClamp;
            _params.SlopeScaledDepthBias = (FLOAT)depthBiasSlopeFactor;
            return *this;
          }
          
          inline D3D11_RASTERIZER_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_RASTERIZER_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
       
        private:
          D3D11_RASTERIZER_DESC _params;
        };
        
        
        // -- depth/stencil testing state params --
        
        /// @class DepthStencilParams
        /// @brief Depth/stencil testing state configuration
        /// @remarks The same DepthStencilParams can (and should) be used to build multiple DepthStencilState instances (if needed).
        class DepthStencilParams final {
        public:
          /// @brief Create default depth-test params: "less" -> "keep" / "incrementWrap"(front fail) / "decrementWrap" (back fail)).
          inline DepthStencilParams() noexcept {
            _init(TRUE, FALSE, D3D11_COMPARISON_LESS, D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS);
            frontFaceOp((StencilOp)D3D11_STENCIL_OP_KEEP, (StencilOp)D3D11_STENCIL_OP_INCR, (StencilOp)D3D11_STENCIL_OP_KEEP);
            backFaceOp((StencilOp)D3D11_STENCIL_OP_KEEP, (StencilOp)D3D11_STENCIL_OP_DECR, (StencilOp)D3D11_STENCIL_OP_KEEP);
          }
          /// @brief Initialize depth-test params (no stencil-test).
          /// @param depthComp              Depth-test comparison.
          /// @param frontFaceDepthFailedOp Operation on front depth/stencil pixel when depth-test fails.
          /// @param frontFacePassedOp      Operation on front stencil pixel when depth-test passes.
          /// @param backFaceDepthFailedOp  Operation on back depth/stencil pixel when depth-test fails.
          /// @param backFacePassedOp       Operation on back stencil pixel when depth-test passes.
          inline DepthStencilParams(StencilCompare depthComp, StencilOp frontFaceDepthFailedOp, StencilOp frontFacePassedOp, 
                                    StencilOp backFaceDepthFailedOp, StencilOp backFacePassedOp) noexcept {
            _init(TRUE, FALSE, (D3D11_COMPARISON_FUNC)depthComp, D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS);
            frontFaceOp(frontFacePassedOp, frontFaceDepthFailedOp, frontFacePassedOp);
            backFaceOp(backFacePassedOp, backFaceDepthFailedOp, backFacePassedOp);
          }
          /// @brief Initialize stencil-test params (no depth-test).
          /// @param frontFaceComp          Stencil-test front comparison.
          /// @param backFaceComp           Stencil-test back comparison.
          /// @param frontFaceFailedOp      Operation on front depth/stencil pixel when stencil-test fails.
          /// @param frontFacePassedOp      Operation on front stencil pixel when depth/stencil-tests pass.
          /// @param backFaceFailedOp       Operation on back depth/stencil pixel when stencil-test fails.
          /// @param backFacePassedOp       Operation on back stencil pixel when depth/stencil-tests pass.
          inline DepthStencilParams(StencilCompare frontFaceComp, StencilCompare backFaceComp, 
                                    StencilOp frontFaceFailedOp, StencilOp frontFacePassedOp,
                                    StencilOp backFaceFailedOp, StencilOp backFacePassedOp) noexcept {
            _init(FALSE, TRUE, D3D11_COMPARISON_ALWAYS, (D3D11_COMPARISON_FUNC)frontFaceComp, (D3D11_COMPARISON_FUNC)backFaceComp);
            frontFaceOp(frontFaceFailedOp, frontFaceFailedOp, frontFacePassedOp);
            backFaceOp(backFaceFailedOp, backFaceFailedOp, backFacePassedOp);
          }
          /// @brief Initialize depth/stencil-test params (both).
          /// @param depthComp              Depth-test comparison.
          /// @param frontFaceComp          Stencil-test front comparison.
          /// @param backFaceComp           Stencil-test back comparison.
          /// @param frontFaceFailedOp      Operation on front depth/stencil pixel when stencil-test fails.
          /// @param frontFaceDepthFailedOp Operation on front depth/stencil pixel when depth-test fails (stencil test passes).
          /// @param frontFacePassedOp      Operation on front stencil pixel when depth/stencil-tests pass.
          /// @param backFaceFailedOp       Operation on back depth/stencil pixel when stencil-test fails.
          /// @param backFaceDepthFailedOp  Operation on back depth/stencil pixel when depth-test fails (stencil test passes).
          /// @param backFacePassedOp       Operation on back stencil pixel when depth/stencil-tests pass.
          inline DepthStencilParams(StencilCompare depthComp, StencilCompare frontFaceComp, StencilCompare backFaceComp, 
                                    StencilOp frontFaceFailedOp, StencilOp frontFaceDepthFailedOp, StencilOp frontFacePassedOp,
                                    StencilOp backFaceFailedOp, StencilOp backFaceDepthFailedOp, StencilOp backFacePassedOp) noexcept {
            _init(TRUE, TRUE, (D3D11_COMPARISON_FUNC)depthComp, (D3D11_COMPARISON_FUNC)frontFaceComp, (D3D11_COMPARISON_FUNC)backFaceComp);
            frontFaceOp(frontFaceFailedOp, frontFaceDepthFailedOp, frontFacePassedOp);
            backFaceOp(backFaceFailedOp, backFaceDepthFailedOp, backFacePassedOp);
          }
          
          DepthStencilParams(const DepthStencilParams&) = default;
          DepthStencilParams& operator=(const DepthStencilParams&) = default;
          ~DepthStencilParams() noexcept = default;


          // -- depth/stencil test operations --

          /// @brief Enable/disable depth-test
          inline DepthStencilParams& enableDepthTest(bool isEnabled) noexcept { _params.DepthEnable = isEnabled ? TRUE : FALSE; return *this; }
          /// @brief Enable/disable stencil-test
          inline DepthStencilParams& enableStencilTest(bool isEnabled) noexcept { _params.StencilEnable = isEnabled ? TRUE : FALSE; return *this; }

          /// @brief Set depth-test comparison
          inline DepthStencilParams& depthTest(StencilCompare comp) noexcept { _params.DepthFunc = (D3D11_COMPARISON_FUNC)comp; return *this; }
          /// @brief Set front stencil-test comparison
          inline DepthStencilParams& frontFaceStencilTest(StencilCompare comp) noexcept { _params.FrontFace.StencilFunc=(D3D11_COMPARISON_FUNC)comp; return *this; }
          /// @brief Set back stencil-test comparison
          inline DepthStencilParams& backFaceStencilTest(StencilCompare comp) noexcept { _params.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)comp; return *this; }
          /// @brief Set operations to perform on buffer pixels after front depth/stencil-test
          inline DepthStencilParams& frontFaceOp(StencilOp failed, StencilOp depthFailed, StencilOp passed) noexcept {
            _params.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)failed;
            _params.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)depthFailed;
            _params.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)passed;
            return *this;
          }
          /// @brief Set operations to perform on buffer pixels after back depth/stencil-test
          inline DepthStencilParams& backFaceOp(StencilOp failed, StencilOp depthFailed, StencilOp passed) noexcept {
            _params.BackFace.StencilFailOp = (D3D11_STENCIL_OP)failed;
            _params.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)depthFailed;
            _params.BackFace.StencilPassOp = (D3D11_STENCIL_OP)passed;
            return *this;
          }
          
          // -- depth/stencil masks --
          
          /// @brief Enable/disable pixel writing for depth-test (default: true)
          inline DepthStencilParams& depthMask(bool allowDepthWrite) noexcept {
            _params.DepthWriteMask = allowDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            return *this;
          }
          /// @brief Set pixel read/write mask for stencil-test (default: 0xFF)
          inline DepthStencilParams& stencilMask(uint8_t stencilReadMask, uint8_t stencilWriteMask) noexcept { 
            _params.StencilReadMask = (UINT8)stencilReadMask;
            _params.StencilWriteMask = (UINT8)stencilWriteMask;
            return *this;
          }

          inline D3D11_DEPTH_STENCIL_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_DEPTH_STENCIL_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          
        private:
          void _init(BOOL enableDepth, BOOL enableStencil, D3D11_COMPARISON_FUNC depthComp, 
                     D3D11_COMPARISON_FUNC frontFaceComp, D3D11_COMPARISON_FUNC backFaceComp) noexcept;
        
        private:
          D3D11_DEPTH_STENCIL_DESC _params;
        };
        
        
        // -- blend state params --
        
        /// @brief Blend operation state per render-target (color/alpha blend params for one render-target)
        /// @remarks To configure common blend params for all render-targets, prefer 'BlendParams' instead.
        struct BlendTargetParams final {
          BlendTargetParams() noexcept = default;
          BlendTargetParams(BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                            BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                            bool isEnabled = true, ColorComponentFlag targetWriteMask = ColorComponentFlag::all) noexcept
            : srcColorFactor(srcColorFactor), destColorFactor(destColorFactor), colorBlendOp(colorBlendOp),
              srcAlphaFactor(srcAlphaFactor), destAlphaFactor(destAlphaFactor), alphaBlendOp(alphaBlendOp),
              isEnabled(isEnabled), targetWriteMask(targetWriteMask) {}

          BlendTargetParams(const BlendTargetParams&) = default;
          BlendTargetParams& operator=(const BlendTargetParams&) = default;
          ~BlendTargetParams() noexcept = default;

          BlendFactor srcColorFactor  = BlendFactor::one;  ///< Pre-blend operation to perform on pixel shader output RGB value
          BlendFactor destColorFactor = BlendFactor::zero; ///< Pre-blend operation to perform on existing render-target RGB value
          BlendOp colorBlendOp        = BlendOp::add;      ///< Color blend operation (between srcColorFactor and destColorFactor)
          BlendFactor srcAlphaFactor  = BlendFactor::one;  ///< Pre-blend operation to perform on pixel shader output alpha value
          BlendFactor destAlphaFactor = BlendFactor::zero; ///< Pre-blend operation to perform on existing render-target alpha value
          BlendOp alphaBlendOp        = BlendOp::add;      ///< Alpha blend operation (between srcAlphaFactor and destAlphaFactor)
          bool isEnabled = true;                           ///< Enable/disable blending
          ColorComponentFlag targetWriteMask = ColorComponentFlag::all; ///< Bit-mask specifying which RGBA components are enabled for writing
        };
        
        // ---
        
        /// @class BlendParams
        /// @brief Blend operation state configuration (color/alpha blend params, common to all render-targets)
        /// @remarks The same BlendParams can (and should) be used to build multiple BlendState instances (if needed).
        class BlendParams final {
        public:
          /// @brief Create default blend params: source=one, dest=zero, op=add (writeMask=all).
          inline BlendParams() noexcept
            : BlendParams((BlendFactor)D3D11_BLEND_ONE, (BlendFactor)D3D11_BLEND_ZERO, (BlendOp)D3D11_BLEND_OP_ADD, 
                          (BlendFactor)D3D11_BLEND_ONE, (BlendFactor)D3D11_BLEND_ZERO, (BlendOp)D3D11_BLEND_OP_ADD) {}
          
          /// @brief Initialize blend config params (writeMask=all)
          /// @param srcColorFactor  Pre-blend operation to perform on pixel shader output RGB value (any value)
          /// @param destColorFactor Pre-blend operation to perform on existing render-target RGB value (any value)
          /// @param colorBlendOp    Color blend operation (between srcColorFactor and destColorFactor)
          /// @param srcAlphaFactor  Pre-blend operation to perform on pixel shader output alpha value (one/zero/value containing "Alpha")
          /// @param destAlphaFactor Pre-blend operation to perform on existing render-target alpha value (one/zero/value containing "Alpha")
          /// @param alphaBlendOp    Alpha blend operation between (srcAlphaFactor and destAlphaFactor)
          /// @param targetWriteMask Bit-mask specifying which RGBA components are enabled for writing
          BlendParams(BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                      BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp) noexcept;
          
          /// @brief Initialize common blend config params from render-target blend params
          inline BlendParams(const BlendTargetParams& params) noexcept
            : BlendParams(params.srcColorFactor, params.destColorFactor, params.colorBlendOp, 
                          params.srcAlphaFactor, params.destAlphaFactor, params.alphaBlendOp) { 
            targetWriteMask(params.targetWriteMask).enable(params.isEnabled);
          }
          
          BlendParams(const BlendParams&) = default;
          BlendParams& operator=(const BlendParams&) = default;
          ~BlendParams() noexcept = default;
          
          /// @brief Pre-blend operation to perform on pixel shader output RGB value (any value)
          inline BlendParams& srcColorFactor(BlendFactor factor) noexcept { _params.RenderTarget->SrcBlend = (D3D11_BLEND)factor; return *this; }
          /// @brief Pre-blend operation to perform on existing render-target RGB value (any value)
          inline BlendParams& destColorFactor(BlendFactor factor) noexcept { _params.RenderTarget->DestBlend = (D3D11_BLEND)factor; return *this; }
          /// @brief Pre-blend operation to perform on pixel shader output alpha value (one/zero/value containing "Alpha")
          inline BlendParams& srcAlphaFactor(BlendFactor factor) noexcept { _params.RenderTarget->SrcBlendAlpha = (D3D11_BLEND)factor; return *this; }
          /// @brief Pre-blend operation to perform on existing render-target alpha value (one/zero/value containing "Alpha")
          inline BlendParams& destAlphaFactor(BlendFactor factor) noexcept { _params.RenderTarget->DestBlendAlpha=(D3D11_BLEND)factor; return *this; }

          /// @brief Common color/alpha blend operation 
          inline BlendParams& blendOp(BlendOp op) noexcept { _params.RenderTarget->BlendOp=_params.RenderTarget->BlendOpAlpha=(D3D11_BLEND_OP)op; return *this; }
          /// @brief Color blend operation (between srcColorFactor and destColorFactor)
          inline BlendParams& colorBlendOp(BlendOp op) noexcept { _params.RenderTarget->BlendOp = (D3D11_BLEND_OP)op; return *this; }
          /// @brief Alpha blend operation (between srcAlphaFactor and destAlphaFactor)
          inline BlendParams& alphaBlendOp(BlendOp op) noexcept { _params.RenderTarget->BlendOpAlpha = (D3D11_BLEND_OP)op; return *this; }
          
          /// @brief Bit-mask specifying which RGBA components are enabled for writing
          inline BlendParams& targetWriteMask(ColorComponentFlag mask) noexcept { _params.RenderTarget->RenderTargetWriteMask=(UINT8)mask; return *this; }
          /// @brief Enable/disable blending
          inline BlendParams& enable(bool isEnabled) noexcept { _params.RenderTarget->BlendEnable = isEnabled ? TRUE : FALSE; return *this; }
          
          
          inline D3D11_BLEND_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_BLEND_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          
        private:
          D3D11_BLEND_DESC _params;
        };
        
        
        // -- sampler filter state params --
        
        /// @class FilterParams
        /// @brief Filter/sampler state configuration (texture filter/wrap, level-of-detail...)
        /// @remarks The same FilterParams can be used to build multiple FilterState instances (if needed).
        class FilterParams final {
        public:
          static constexpr inline uint32_t maxAnisotropy() noexcept { return (uint32_t)D3D11_MAX_MAXANISOTROPY; } ///< Max anisotropy level value (usually 8 or 16)
          static constexpr inline float highestLod() noexcept { return 0.f; } ///< Highest / most detailed level-of-detail.
          static constexpr inline float infiniteLod() noexcept { return D3D11_FLOAT32_MAX; } ///< Infinite level-of-detail: only for lodMax.
        
          // ---
        
          /// @brief Create default filter/sampler params: trilinear, clamp to edges, level-of-detail 1 to highest, no depth compared.
          FilterParams() noexcept;
          
          /// @brief Initialize filter/sampler config params
          /// @param minifyFilter   Filter for minification (downscaling).
          /// @param magnifyFilter  Filter for magnification (upscaling).
          /// @param mipMapFilter   Filter between closest mip levels (mip-map).
          /// @param textureWrapUVW Texture out-of-range addressing modes (dimensions[3]: U/V/W). Default border color (with clampToBorder) is transparent.
          /// @param lodMin         Minimum level-of-detail: lower end of mip-map range (value >= 0, or 'highestLod()' for highest mip level).
          /// @param lodMax         Maximum level-of-detail: upper end of mip-map range
          ///                       (lodMax >= lodMin, or 'highestLod()' for highest mip level, or 'infiniteLod()' to have no limit).
          inline FilterParams(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter,
                              const TextureWrap textureWrapUVW[3], float lodMin = highestLod(), float lodMax = infiniteLod()) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setFilter(minifyFilter, magnifyFilter, mipMapFilter);
          }
          /// @brief Initialize filter/sampler config params -- depth comparison
          /// @param depthComp  depth compare-mode with existing pixels ("compared" filter)
          inline FilterParams(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter,
                              const TextureWrap textureWrapUVW[3], float lodMin, float lodMax, StencilCompare depthComp) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setFilter(minifyFilter, magnifyFilter, mipMapFilter, depthComp);
          }
          
          /// @brief Initialize anisotropic filter/sampler config params
          /// @param maxAnisotropy  Max anisotropy level (clamping value: 1 to 'maxAnisotropy()')
          /// @param textureWrapUVW Texture out-of-range addressing modes (dimensions[3]: U/V/W). Default border color (with clampToBorder) is transparent.
          /// @param lodMin         Minimum level-of-detail: lower end of mip-map range (value >= 0, or 'highestLod()' for highest mip level).
          /// @param lodMax         Maximum level-of-detail: upper end of mip-map range
          ///                       (lodMax >= lodMin, or 'highestLod()' for highest mip level, or 'infiniteLod()' to have no limit).
          inline FilterParams(uint32_t maxAnisotropy, const TextureWrap textureWrapUVW[3], float lodMin = highestLod(), float lodMax = infiniteLod()) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setAnisotropicFilter(maxAnisotropy);
          }
          /// @brief Initialize anisotropic filter/sampler config params -- depth comparison
          /// @param depthComp  depth compare-mode with existing pixels ("compared" filter)
          inline FilterParams(uint32_t maxAnisotropy, const TextureWrap textureWrapUVW[3], float lodMin, float lodMax, StencilCompare depthComp) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setAnisotropicFilter(maxAnisotropy, depthComp);
          }
          
          FilterParams(const FilterParams&) = default;
          FilterParams& operator=(const FilterParams&) = default;
          ~FilterParams() noexcept = default;
          
          
          // -- filter type --
          
          /// @brief Use point/linear filter + set type for minification (downscaling), magnification (upscaling), between mip levels (mip-map) -- no depth comparison
          inline FilterParams& setFilter(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter) noexcept {
            _params.Filter = _toFilterType(minifyFilter, magnifyFilter, mipMapFilter);
            _params.MaxAnisotropy = 1;
            _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            return *this; 
          }
          /// @brief Use point/linear filter + set type for minification (downscaling), magnification (upscaling), between mip levels (mip-map).
          ///        Enable depth compare-mode with existing pixels ("compared" filter)
          inline FilterParams& setFilter(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter, StencilCompare depthComp) noexcept {
            _params.Filter = (D3D11_FILTER)((int)_toFilterType(minifyFilter, magnifyFilter, mipMapFilter) 
                                          + ((int)D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT - (int)D3D11_FILTER_MIN_MAG_MIP_POINT));
            _params.MaxAnisotropy = 1;
            _params.ComparisonFunc = (D3D11_COMPARISON_FUNC)depthComp;
            return *this; 
          }
          /// @brief Use anisotropic filter + set max anisotropy level (clamping value: 1 to 'maxAnisotropy()') -- no depth comparison
          inline FilterParams& setAnisotropicFilter(uint32_t maxAnisotropy) noexcept {
            _params.Filter = D3D11_FILTER_ANISOTROPIC;
            _params.MaxAnisotropy = maxAnisotropy;
            _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            return *this; 
          }
          /// @brief Use anisotropic filter + set max anisotropy level (clamping value: 1 to 'maxAnisotropy()')
          ///        Enable depth compare-mode with existing pixels ("compared" filter)
          inline FilterParams& setAnisotropicFilter(uint32_t maxAnisotropy, StencilCompare depthComp) noexcept {
            _params.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
            _params.MaxAnisotropy = maxAnisotropy;
            _params.ComparisonFunc = (D3D11_COMPARISON_FUNC)depthComp;
            return *this; 
          }

          
          // -- texture addressing / mip-map level-of-detail --
          
          /// @brief Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          inline FilterParams& textureWrap(const TextureWrap textureWrapUVW[3]) noexcept {
            _params.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapUVW[0];
            _params.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapUVW[1];
            _params.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapUVW[2];
            return *this;
          }
          /// @brief Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          inline FilterParams& textureWrap(TextureWrap textureWrapU, TextureWrap textureWrapV, TextureWrap textureWrapW) noexcept {
            _params.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapU;
            _params.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapV;
            _params.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapW;
            return *this;
          }
          /// @brief Border color used with TextureWrap::clampToBorder (color[4]: R/G/B/A) - color range == [0.0 ; 1.0] - NULL to use default (transparent).
          ///        Color should be gamma correct (see Renderer.toGammaCorrectColor).
          FilterParams& borderColor(const ColorChannel rgba[4]) noexcept;
          
          /// @brief Set mip-map level-of-detail range
          /// @param lodMin  Minimum level-of-detail: lower end of mip-map range (value above 0, or 'highestLod()' for highest mip level).
          /// @param lodMax  Maximum level-of-detail: upper end of mip-map range
          inline FilterParams& lod(float lodMin, float lodMax) noexcept { _params.MinLOD = lodMin; _params.MaxLOD = lodMax; return *this; }
          /// @brief Set offset added to calculated mip-map level
          inline FilterParams& lodBias(float bias) noexcept { _params.MipLODBias = bias; return *this; }
          

          inline D3D11_SAMPLER_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_SAMPLER_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          
        private:
          void _init(const TextureWrap textureWrapUVW[3], float lodMin, float lodMax) noexcept;
          
          static constexpr inline D3D11_FILTER _toFilterType(TextureFilter minify, TextureFilter magnify, TextureFilter mip) noexcept {
            if (minify == TextureFilter::linear) {
              if (magnify == TextureFilter::linear)
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; // L-L-L / L-L-N
              else
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; // L-N-L / L-N-N
            }
            else {
              if (magnify == TextureFilter::linear)
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; // N-L-L / N-L-N
              else
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT; // N-N-L / N-N-N
            }
          }
        
        private:
          D3D11_SAMPLER_DESC _params;
        };
        
        
        // ---------------------------------------------------------------------
        // renderer state factory
        // ---------------------------------------------------------------------
        
        /// @class RendererStateFactory
        /// @brief Utility to create renderer state buffers (depth/stencil, rasterization, blending, sampler/filter)
        class RendererStateFactory final {
        public:
          RendererStateFactory(Renderer& renderer) : _device(renderer.device()) {}
          RendererStateFactory(const RendererStateFactory&) = default;
          RendererStateFactory(RendererStateFactory&&) noexcept = default;
          RendererStateFactory& operator=(const RendererStateFactory&) = default;
          RendererStateFactory& operator=(RendererStateFactory&&) noexcept = default;
          ~RendererStateFactory() noexcept = default;
          
          // -- rasterizer state buffer --
          
          /// @brief Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
          /// @remarks The same RasterizerParams can (and should) be used to build multiple RasterizerState instances (if needed).
          /// @throws runtime_error on creation failure
          RasterizerState createRasterizerState(const RasterizerParams& params);
          
          // -- depth/stencil testing state buffer --
          
          /// @brief Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          DepthStencilState createDepthStencilTestState(const DepthStencilParams& params);
          
          // -- blend state buffer --
          
          /// @brief Create blend state (common to all render-targets)
          /// @remarks The same BlendParams can (and should) be used to build multiple BlendState instances (if needed).
          /// @throws runtime_error on creation failure
          BlendState createBlendState(const BlendParams& params);
          /// @brief Create blend state with different color/alpha params for each render-target (up to 'Renderer::maxRenderTargets()' targets (usually 8))
          /// @param perTargetParams  Array of color/alpha blending params, for each render-target (array index == target index).
          /// @param arrayLength      Array length of 'perTargetParams'.
          /// @throws runtime_error on creation failure
          BlendState createBlendStatePerTarget(const BlendTargetParams* perTargetParams, size_t arrayLength);

          // -- sampler filter state buffer --

          /// @brief Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @remarks The same FilterParams can be used to build multiple FilterState instances (if needed).
          /// @throws runtime_error on creation failure
          FilterState createFilterState(const FilterParams& params);
          
        private:
          DeviceHandle _device = nullptr;
        };
      }
    }
  }

#endif
