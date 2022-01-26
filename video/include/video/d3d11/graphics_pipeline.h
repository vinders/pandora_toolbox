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
# ifndef __MINGW32__
#   pragma warning(push)
#   pragma warning(disable: 4100)  // disable warnings about unused params
#   pragma warning(disable: 33011) // disable warnings about indices
# endif
# include <cassert>
# include <cstddef>
# include <cstdint>
# include <memory>
# include "./_private/_shared_resource.h" // includes D3D11
# include "./shader.h"   // includes D3D11
# include "./viewport.h" // includes D3D11
# include "./scissor.h"  // includes D3D11
# include "./renderer.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // ---------------------------------------------------------------------
        // pipeline state params
        // ---------------------------------------------------------------------

		// -- rasterizer state params -- ---------------------------------------
        
        /// @class RasterizerParams
        /// @brief Rasterizer state configuration (filling/culling/clipping/bias params)
        /// @remarks The same RasterizerParams can (and should) be used to build multiple GraphicsPipeline (if needed).
        class RasterizerParams final {
        public:
          /// @brief Create default rasterizer params: filled, back-face culling, clockwise order, no depth clipping.
          RasterizerParams() noexcept;
          /// @brief Initialize rasterizer config params
          /// @param cull              Identify polygons to hide: back-facing, front-facing, none.
          /// @param fill              Filled/wireframe polygon rendering.
          /// @param isFrontClockwise  Choose vertex order of front-facing polygons (true = clockwise / false = counter-clockwise)
          /// @param depthClipping     Enable clipping based on distance
          /// @param scissorClipping   Enable scissor-rectangle clipping
          RasterizerParams(CullMode cull, FillMode fill = FillMode::fill,
                           bool isFrontClockwise = true, bool depthClipping = false, bool scissorClipping = false) noexcept;
          
          RasterizerParams(const RasterizerParams&) = default;
          RasterizerParams& operator=(const RasterizerParams&) = default;
          ~RasterizerParams() noexcept = default;
          
          /// @brief Choose vertex order of front-facing polygons (true = clockwise / false = counter-clockwise)
          inline RasterizerParams& vertexOrder(bool isFrontClockwise) noexcept {
            _params.FrontCounterClockwise = isFrontClockwise ? FALSE : TRUE; return *this;
          }
          /// @brief Identify polygons to hide: back-facing, front-facing, none.
          inline RasterizerParams& cullMode(CullMode cull) noexcept { _params.CullMode = (D3D11_CULL_MODE)cull; return *this; }
          /// @brief Set filled/wireframe polygon rendering
          inline RasterizerParams& fillMode(FillMode fill) noexcept {
            if (fill == FillMode::linesAA) { _params.FillMode = D3D11_FILL_WIREFRAME;  _params.AntialiasedLineEnable = TRUE; }
            else                           { _params.FillMode = (D3D11_FILL_MODE)fill; _params.AntialiasedLineEnable = FALSE; }
            return *this;
          }
          
          /// @brief Enable clipping based on distance
          inline RasterizerParams& depthClipping(bool isEnabled) noexcept {
            _params.DepthClipEnable = isEnabled ? TRUE : FALSE; return *this;
          }
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
          RasterizerStateId computeId() const noexcept; ///< Compute resource ID based on params -- Reserved for internal use
       
        private:
          D3D11_RASTERIZER_DESC _params;
        };
        
        
        // -- depth/stencil testing state params -- ----------------------------
        
        /// @class DepthStencilParams
        /// @brief Depth/stencil testing state configuration
        /// @remarks The same DepthStencilParams can (and should) be used to build multiple GraphicsPipeline instances (if needed).
        class DepthStencilParams final {
        public:
          /// @brief Create default depth-test params: "less" -> "keep" / "incrementWrap"(front fail) / "decrementWrap" (back fail)).
          inline DepthStencilParams() noexcept {
            _init(TRUE, FALSE, D3D11_COMPARISON_LESS, D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS);
            frontFaceOp((StencilOp)D3D11_STENCIL_OP_KEEP, (StencilOp)D3D11_STENCIL_OP_INCR, (StencilOp)D3D11_STENCIL_OP_KEEP);
            backFaceOp((StencilOp)D3D11_STENCIL_OP_KEEP, (StencilOp)D3D11_STENCIL_OP_DECR, (StencilOp)D3D11_STENCIL_OP_KEEP);
          }
          /// @brief Create default depth/stencil-test params, with custom status for depth/stencil tests.
          inline DepthStencilParams(bool isDepthEnabled, bool isStencilEnabled) noexcept {
            _init(isDepthEnabled ? TRUE : FALSE, isStencilEnabled ? TRUE : FALSE,
                  D3D11_COMPARISON_LESS, D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS);
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
                                    StencilOp backFaceDepthFailedOp, StencilOp backFacePassedOp, uint32_t stencilRef = 1u) noexcept
            : _stencilRef(stencilRef) {
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
                                    StencilOp backFaceFailedOp, StencilOp backFacePassedOp, uint32_t stencilRef = 1u) noexcept
            : _stencilRef(stencilRef) {
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
                                    StencilOp backFaceFailedOp, StencilOp backFaceDepthFailedOp, StencilOp backFacePassedOp,
                                    uint32_t stencilRef = 1u) noexcept : _stencilRef(stencilRef) {
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
          /// @brief Set depth-test comparison
          inline DepthStencilParams& depthTest(StencilCompare comp) noexcept { _params.DepthFunc = (D3D11_COMPARISON_FUNC)comp; return *this; }

          /// @brief Enable/disable stencil-test
          inline DepthStencilParams& enableStencilTest(bool isEnabled) noexcept { _params.StencilEnable = isEnabled ? TRUE : FALSE; return *this; }
          /// @brief Set front stencil-test comparison
          inline DepthStencilParams& frontFaceStencilTest(StencilCompare comp) noexcept {
            _params.FrontFace.StencilFunc=(D3D11_COMPARISON_FUNC)comp; return *this;
          }
          /// @brief Set operations to perform on buffer pixels after front depth/stencil-test
          inline DepthStencilParams& frontFaceOp(StencilOp failed, StencilOp depthFailed, StencilOp passed) noexcept {
            _params.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)failed;
            _params.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)depthFailed;
            _params.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)passed;
            return *this;
          }
          /// @brief Set back stencil-test comparison
          inline DepthStencilParams& backFaceStencilTest(StencilCompare comp) noexcept {
            _params.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)comp; return *this;
          }
          /// @brief Set operations to perform on buffer pixels after back depth/stencil-test
          inline DepthStencilParams& backFaceOp(StencilOp failed, StencilOp depthFailed, StencilOp passed) noexcept {
            _params.BackFace.StencilFailOp = (D3D11_STENCIL_OP)failed;
            _params.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)depthFailed;
            _params.BackFace.StencilPassOp = (D3D11_STENCIL_OP)passed;
            return *this;
          }

          /// @brief Set depth/stencil operation reference (default: 1)
          inline DepthStencilParams& stencilReference(uint32_t stencilRef) noexcept { _stencilRef = stencilRef; return *this; }
          /// @brief Get depth/stencil operation reference
          inline uint32_t stencilReference() const noexcept { return _stencilRef; }
          
          // -- depth/stencil masks --
          
          /// @brief Enable/disable pixel writing for depth-test (default: true)
          inline DepthStencilParams& depthMask(bool allowDepthWrite) noexcept {
            _params.DepthWriteMask = allowDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            return *this;
          }
          /// @brief Set pixel read/write mask for stencil-test (default: 0xFF)
          inline DepthStencilParams& stencilMask(uint32_t stencilReadMask, uint32_t stencilWriteMask) noexcept { 
            _params.StencilReadMask = (UINT8)stencilReadMask;
            _params.StencilWriteMask = (UINT8)stencilWriteMask;
            return *this;
          }

          inline D3D11_DEPTH_STENCIL_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_DEPTH_STENCIL_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          DepthStencilStateId computeId() const noexcept; ///< Compute resource ID based on params -- Reserved for internal use
          
        private:
          void _init(BOOL enableDepth, BOOL enableStencil, D3D11_COMPARISON_FUNC depthComp, 
                     D3D11_COMPARISON_FUNC frontFaceComp, D3D11_COMPARISON_FUNC backFaceComp) noexcept;
        
        private:
          D3D11_DEPTH_STENCIL_DESC _params;
          uint32_t _stencilRef = 1u;
        };
        
        
        // -- blend state params -- --------------------------------------------
        
        /// @class BlendParams
        /// @brief Blend operation state configuration (color/alpha blend params, common to all render-targets)
        /// @remarks The same BlendParams can (and should) be used to build multiple GraphicsPipeline instances (if needed).
        class BlendParams final {
        public:
          /// @brief Create default blend params: source=one, dest=zero, op=add (writeMask=all).
          inline BlendParams() noexcept
            : BlendParams((BlendFactor)D3D11_BLEND_ONE, (BlendFactor)D3D11_BLEND_ZERO, (BlendOp)D3D11_BLEND_OP_ADD, 
                          (BlendFactor)D3D11_BLEND_ONE, (BlendFactor)D3D11_BLEND_ZERO, (BlendOp)D3D11_BLEND_OP_ADD) {}
          /// @brief Create default blend params with custom status.
          inline BlendParams(bool isEnabled) noexcept
            : BlendParams((BlendFactor)D3D11_BLEND_ONE, (BlendFactor)D3D11_BLEND_ZERO, (BlendOp)D3D11_BLEND_OP_ADD, 
                          (BlendFactor)D3D11_BLEND_ONE, (BlendFactor)D3D11_BLEND_ZERO, (BlendOp)D3D11_BLEND_OP_ADD) { enable(isEnabled); }
          
          /// @brief Initialize blend config params (writeMask=all)
          /// @param srcColorFactor  Pre-blend operation to perform on pixel shader output RGB value (any value)
          /// @param destColorFactor Pre-blend operation to perform on existing render-target RGB value (any value)
          /// @param colorBlendOp    Color blend operation (between srcColorFactor and destColorFactor)
          /// @param srcAlphaFactor  Pre-blend operation to perform on pixel shader output alpha value (one/zero/value containing "Alpha")
          /// @param destAlphaFactor Pre-blend operation to perform on existing render-target alpha value (one/zero/value containing "Alpha")
          /// @param alphaBlendOp    Alpha blend operation between (srcAlphaFactor and destAlphaFactor)
          /// @param mask            Bit-mask specifying which RGBA components are enabled for writing
          BlendParams(BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                      BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                      ColorComponentFlag mask = ColorComponentFlag::all) noexcept;

          BlendParams(const BlendParams&) = default;
          BlendParams& operator=(const BlendParams&) = default;
          ~BlendParams() noexcept = default;

          /// @brief Set pre-blend operation to perform on RGB value
          /// @param srcFactor   Pre-blend operation to perform on pixel shader output RGB value (any value)
          /// @param destFactor  Pre-blend operation to perform on existing render-target RGB value (any value)
          /// @param op          Color blend operation (between srcColorFactor and destColorFactor)
          inline BlendParams& colorBlend(BlendFactor srcFactor, BlendFactor destFactor, BlendOp op) noexcept {
            _params.RenderTarget->SrcBlend  = (D3D11_BLEND)srcFactor;
            _params.RenderTarget->DestBlend = (D3D11_BLEND)destFactor;
            _params.RenderTarget->BlendOp = (D3D11_BLEND_OP)op;
            return *this;
          }
          /// @brief Set pre-blend operation to perform on alpha value
          /// @param srcFactor   Pre-blend operation to perform on pixel shader output alpha value (one/zero/value containing "Alpha")
          /// @param destFactor  Pre-blend operation to perform on existing render-target alpha value (one/zero/value containing "Alpha")
          /// @param op          Alpha blend operation between (srcAlphaFactor and destAlphaFactor)
          inline BlendParams& alphaBlend(BlendFactor srcFactor, BlendFactor destFactor, BlendOp op) noexcept {
            _params.RenderTarget->SrcBlendAlpha  = (D3D11_BLEND)srcFactor;
            _params.RenderTarget->DestBlendAlpha = (D3D11_BLEND)destFactor;
            _params.RenderTarget->BlendOpAlpha = (D3D11_BLEND_OP)op;
            return *this;
          }

          /// @brief Set constant factor (RGBA) for constant blend factors
          /// @remarks The constant factor is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          BlendParams& blendConstant(const ColorChannel constantFactorRgba[4]) noexcept;
          /// @brief Get constant factor (RGBA) used by constant blend factors
          inline const ColorChannel* blendConstant() const noexcept { return this->_blendConstant; }
          
          /// @brief Bit-mask specifying which RGBA components are enabled for writing
          inline BlendParams& targetWriteMask(ColorComponentFlag mask) noexcept {
            _params.RenderTarget->RenderTargetWriteMask=(UINT8)mask; return *this;
          }
          /// @brief Enable/disable blending
          inline BlendParams& enable(bool isEnabled) noexcept { _params.RenderTarget->BlendEnable = isEnabled ? TRUE : FALSE; return *this; }
          
          inline D3D11_BLEND_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_BLEND_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          BlendStateId computeId() const noexcept; ///< Compute single-target resource ID based on params -- Reserved for internal use
          
        private:
          D3D11_BLEND_DESC _params;
          ColorChannel _blendConstant[4]{ 1.f,1.f,1.f,1.f };
        };

        // ---

        /// @class BlendPerTargetParams
        /// @brief Blend operation state configuration per render-target (color/alpha blend params)
        /// @remarks - The same BlendParams can (and should) be used to build multiple GraphicsPipeline instances (if needed).
        ///          - To use the same params for all render-targets, use BlendParams instead.
        class BlendPerTargetParams final {
        public:
          /// @brief Create empty blend params (disabled for all targets).
          BlendPerTargetParams() noexcept;

          BlendPerTargetParams(const BlendPerTargetParams&) = default;
          BlendPerTargetParams& operator=(const BlendPerTargetParams&) = default;
          ~BlendPerTargetParams() noexcept = default;

          /// @brief Enable blending for a specific render-target
          /// @param targetIndex     Index of the target to specify (must be below 'Renderer.maxRenderTargets()' (usually 8))
          /// @param srcColorFactor  Pre-blend operation to perform on pixel shader output RGB value (any value)
          /// @param destColorFactor Pre-blend operation to perform on existing render-target RGB value (any value)
          /// @param colorBlendOp    Color blend operation (between srcColorFactor and destColorFactor)
          /// @param srcAlphaFactor  Pre-blend operation to perform on pixel shader output alpha value (one/zero/value containing "Alpha")
          /// @param destAlphaFactor Pre-blend operation to perform on existing render-target alpha value (one/zero/value containing "Alpha")
          /// @param alphaBlendOp    Alpha blend operation between (srcAlphaFactor and destAlphaFactor)
          /// @param mask            Bit-mask specifying which RGBA components are enabled for writing
          BlendPerTargetParams& setTargetBlend(uint32_t targetIndex,
                                               BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                                               BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                                               ColorComponentFlag mask = ColorComponentFlag::all) noexcept;
          /// @brief Disable blending for a specific render-target
          /// @param targetIndex  Index of the target to disable (must be below 'Renderer.maxRenderTargets()' (usually 8))
          inline BlendPerTargetParams& disableTargetBlend(uint32_t targetIndex) noexcept {
            assert(targetIndex < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
            _params.RenderTarget[targetIndex].BlendEnable = FALSE;
            return *this;
          }

          /// @brief Set constant factor (RGBA) for constant blend factors
          /// @remarks The constant factor is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          BlendPerTargetParams& blendConstant(const ColorChannel constantFactorRgba[4]) noexcept;
          /// @brief Get constant factor (RGBA) used by constant blend factors
          inline const ColorChannel* blendConstant() const noexcept { return this->_blendConstant; }

          inline D3D11_BLEND_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_BLEND_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          BlendStatePerTargetId computeId() const noexcept; ///< Compute multi-target resource ID based on params -- Reserved for internal use
          
        private:
          D3D11_BLEND_DESC _params;
          ColorChannel _blendConstant[4]{ 1.f,1.f,1.f,1.f };
        };


        // ---------------------------------------------------------------------
        // graphics pipeline builder
        // ---------------------------------------------------------------------

        /// @class GraphicsPipeline
        /// @brief Graphics system pipeline stages - shader program & rendering state to execute on GPU
        /// @warning - Can only be used for display shaders (vertex, fragment, geometry, tessellation).
        ///            For compute shaders, a computation pipeline must be created instead!
        ///          - Must be built using GraphicsPipeline::Builder.
        /// @remarks - To use the pipeline shaders/states, bind it to the associated Renderer instance.
        ///          - Direct3D11 rendering states are dynamic by default, which means that
        ///            direct state changes are allowed (viewport, scissor test, depth-stencil test, blending).
        ///            Note that other renderers (such as Vulkan) must explicitly mark these states as dynamic to allow it.
        ///          - Direct3D11 rasterizer settings are also dynamic. Some other renderers (such as Vulkan)
        ///            do not support such changes, so avoid them for cross-API projects!
        ///          - Try to limit the number of dynamic state changes to improve efficiency.
        class GraphicsPipeline final {
        public:
          using Handle = GraphicsPipelineHandle; ///< Bindable graphics pipeline handle

          /// @brief Empty pipeline -- not usable (only useful to store variable not immediately initialized)
          /// @remarks Use GraphicsPipeline::Builder to create an initialized pipeline.
          GraphicsPipeline() = default;
          GraphicsPipeline(const GraphicsPipeline&) = delete;
          GraphicsPipeline(GraphicsPipeline&& rhs) noexcept = default;
          GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
          GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept = default;
          ~GraphicsPipeline() noexcept { release(); }

          void release() noexcept; ///< Destroy pipeline object

          /// @brief Get native pipeline handle -- reserved for internal use
          inline Handle handle() const noexcept { return this->_pipeline; }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (this->_pipeline == nullptr); }

          // ---

          /// @class GraphicsPipeline.Builder
          /// @brief Graphics pipeline setter and builder
          /// @warning The latest Viewport and ScissorRectangle arrays must be kept alive as long as the Builder is used.
          ///          To avoid unnecessary copies and processing, their lifetime is NOT guaranteed by the Builder instance!
          class Builder final {
          public:
            /// @brief Create pipeline builder
            /// @throws logic_error if renderer is NULL.
            Builder(std::shared_ptr<Renderer> renderer);
            Builder(const Builder&) = delete;
            Builder(Builder&&) noexcept = default;
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&&) noexcept = default;
            ~Builder() noexcept = default;

            // -- vertex input layout --

            /// @brief Bind vertex shader input layout description (optional)
            /// @warning Required if vertex shader uses input locations.
            /// @remarks Pass an empty InputLayout object to reset.
            inline Builder& setInputLayout(InputLayout inputLayout) { this->_params.inputLayout = std::move(inputLayout); return *this; }

            /// @brief Set vertex polygon topology for input stage
            inline Builder& setVertexTopology(VertexTopology topology) noexcept { this->_params.topology = topology; return *this; }
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              /// @brief Set vertex patch topology for input stage (required for tessellation shaders)
              /// @param controlPoints  Number of patch control points: between 1 and 32 (other values will be clamped).
              Builder& setPatchTopology(uint32_t controlPoints) noexcept;
#           endif

            // -- shader stages --

            /// @brief Bind shader stages to pipeline (required)
            /// @warning - If tessellation stages are specified, a tessellation patch topology must be set (setPatchTopology).
            ///          - Vertex shaders with input data require an input layout description (setInputLayout).
            Builder& setShaderStages(const Shader shaders[], size_t shaderCount);
            /// @brief Remove all shader stages
            inline Builder& clearShaderStages() noexcept {
              for (auto* it = &(this->_params.shaderStages[__P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX]); it >= this->_params.shaderStages; --it)
                it->release();
              return *this;
            }

            /// @brief Bind (or replace) shader module for a specific stage (at least vertex+fragment or compute required)
            /// @warning - If tessellation stages are specified, a tessellation patch topology must be set (setPatchTopology).
            ///          - Vertex shaders with input data require an input layout description (setInputLayout).
            Builder& attachShaderStage(const Shader& shaderModule);
            /// @brief Remove a shader stage
            inline Builder& detachShaderStage(ShaderType stage) noexcept {
              assert((unsigned int)stage <= __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX);
              this->_params.shaderStages[(unsigned int)stage].release();
              return *this;
            }

            // -- rendering pipeline states --

            /// @brief Bind rasterization state (required)
            /// @throws runtime_error on creation failure
            Builder& setRasterizerState(const RasterizerParams& state);
            /// @brief Bind depth/stencil test state (required)
            /// @warning Required to use a depth/stencil buffer when rendering.
            /// @throws runtime_error on creation failure
            Builder& setDepthStencilState(const DepthStencilParams& state);
            /// @brief Bind color/alpha blending state -- common to all render-targets (one of the 2 methods required)
            /// @param constantColorRgba  Only used if the blend state uses BlendFactor::constantColor/constantInvColor
            ///                           (defaults to white if set to NULL).
            /// @throws runtime_error on creation failure
            Builder& setBlendState(const BlendParams& state);
            /// @brief Bind color/alpha blending state -- customized per render-target (one of the 2 methods required)
            /// @param constantColorRgba  Only used if the blend state uses BlendFactor::constantColor/constantInvColor
            ///                           (defaults to white if set to NULL).
            /// @throws runtime_error on creation failure
            Builder& setBlendState(const BlendPerTargetParams& state);

            // -- viewports & scissor tests --

            /// @brief Set viewports and scissor-test rectangles (optional: dynamic viewports if ignored)
            /// @param viewports       Pointer to viewport (if 'viewportCount' is 1) or array of viewports.
            ///                        Set to NULL to use dynamic viewports (a call to Renderer.setViewport(s) is then mandatory).
            /// @param viewportCount   Number of viewports (array size of 'viewports', if not NULL).
            ///                        Must be the same as 'scissorCount' if 'isDynamicCount' is false.
            /// @param scissorTests    Pointer to scissor-test rectangle (if 'scissorCount' is 1) or array of scissor-test rectangles.
            ///                        Set to NULL to use dynamic scissor-tests (with Renderer.setScissorRectangle(s)).
            /// @param scissorCount    Number of viewports/scissor-tests (and array size of 'viewports' and 'scissorTests' (if not NULL)).
            ///                        Must be the same as 'viewportCount' if 'isDynamicCount' is false.
            /// @param useDynamicCount Allow viewport/scissor-test count to be different (always supported with Direct3D11).
            /// @remarks The value of viewportCount and scissorCount can't exceed Renderer.maxViewports().
            /// @warning The current Viewport and ScissorRectangle arrays must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, their lifetime is NOT guaranteed by the Builder instance!
            Builder& setViewports(const Viewport viewports[], size_t viewportCount,
                                  const ScissorRectangle scissorTests[], size_t scissorCount,
                                  bool useDynamicCount = true) noexcept;


            // -- pipeline build --

            /// @brief Provide render-targets to determine the number of targets, their respective format and multisampling (required)
            /// @param renderTargets  Pointer to render-target (if 'targetCount' is 1) or array of render-targets. Can't be NULL!
            /// @param targetCount    Array size of 'renderTargets'.
            /// @param sampleCount    Sample count for multisampling (anti-aliasing). Use 0 or 1 to disable multisampling.
            /// @warning The pipeline will need to be used with compatible render-targets only.
            /// @throws - invalid_argument if no render target description is provided;
            ///         - logic_error if some required states/stages haven't been set;
            Builder& setRenderTargetFormat(void* renderTargets, size_t targetCount, uint32_t sampleCount = 1) {
              //if (renderTargets == nullptr || renderTargetCount == 0)
              //  throw std::invalid_argument("GraphicsPipeline: no render target provided");
              //... use render targets + MSAA params -> generate MSAA targets
              return *this;
            }

            /// @brief Build a graphics pipeline (based on current params)
            /// @throws logic_error if some required states/stages haven't been set;
            GraphicsPipeline build() { return GraphicsPipeline(*this); }


            // -- Direct3D pipeline state factory --

            /// @brief Create rasterizer state driver resource - can be used to change rasterizer state (setRasterizerState)
            /// @warning Not available in other APIs -> do not use for cross-API projects
            /// @throws runtime_error on creation failure
            RasterizerState createRasterizerState(const RasterizerParams& params);
            /// @brief Create depth/stencil testing state driver resource - can be used to set depth/stencil mode (setDepthStencilState)
            /// @warning Not available in other APIs -> do not use for cross-API projects
            /// @throws runtime_error on creation failure
            DepthStencilState createDepthStencilState(const DepthStencilParams& params);

            /// @brief Create blend state driver resource (common to all render-targets)
            /// @warning Not available in other APIs -> do not use for cross-API projects
            /// @throws runtime_error on creation failure
            BlendState createBlendState(const BlendParams& params);
            /// @brief Create blend state driver resource (with different params for each render-target)
            /// @warning Not available in other APIs -> do not use for cross-API projects
            /// @throws runtime_error on creation failure
            BlendState createBlendState(const BlendPerTargetParams& params);

          private:
            _DxPipelineStages _params;
            std::shared_ptr<Renderer> _renderer = nullptr;
            friend class GraphicsPipeline;

            const Viewport* _viewports = nullptr;
            size_t _viewportCount = 0;
            const ScissorRectangle* _scissorTests = nullptr;
            size_t _scissorCount = 0;
            static uint64_t _lastViewportScissorId;
          };
          
        private:
          // Create pipeline object -- reserved for internal use or advanced usage
          // -> throws: logic_error if some required states/stages haven't been set.
          GraphicsPipeline(GraphicsPipeline::Builder& builder);
        
        private:
          std::shared_ptr<_DxPipelineStages> _pipeline = nullptr;
          std::shared_ptr<Renderer> _renderer = nullptr;
        };
      }
    }
  }
# ifndef __MINGW32__
#   pragma warning(pop)
# endif
#endif
