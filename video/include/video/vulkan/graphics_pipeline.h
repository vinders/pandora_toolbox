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

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cstddef>
# include <cstdint>
# include <memory>
# include <memory/light_vector.h>
# include "video/vulkan/_private/_shared_resource.h" // includes vulkan
# include "./viewport.h"  // includes vulkan
# include "./scissor.h"   // includes vulkan
# include "./shader.h"    // includes vulkan
# include "./renderer.h"  // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
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
          
          RasterizerParams(const RasterizerParams&) noexcept = default;
          RasterizerParams& operator=(const RasterizerParams& rhs) = default;
          ~RasterizerParams() noexcept = default;
          
          /// @brief Choose vertex order of front-facing polygons (true = clockwise / false = counter-clockwise)
          inline RasterizerParams& vertexOrder(bool isFrontClockwise) noexcept {
            _params.frontFace = isFrontClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE; return *this;
          }
          /// @brief Identify polygons to hide: back-facing, front-facing, none.
          inline RasterizerParams& cullMode(CullMode cull) noexcept { _params.cullMode = (VkCullModeFlags)cull; return *this; }
          /// @brief Set filled/wireframe polygon rendering
          RasterizerParams& fillMode(FillMode fill) noexcept;
          
          /// @brief Enable clipping based on distance
          RasterizerParams& depthClipping(bool isEnabled) noexcept {
            _depthClipping.depthClipEnable = isEnabled ? VK_TRUE : VK_FALSE; return *this;
          }
          /// @brief Enable scissor-rectangle clipping
          inline RasterizerParams& scissorClipping(bool isEnabled) noexcept { _useScissorClipping = isEnabled; return *this; }
          
          /// @brief Set depth offset factor (used to improve shadow maps + to avoid depth artifacts and self-shadowing)
          ///< @param depthBias            Scalar factor controlling the constant depth value added to each fragment
          ///< @param depthBiasClamp       Maximum (or minimum) depth bias of a fragment.
          ///< @param depthBiasSlopeFactor Scalar factor applied to a fragment’s slope. 
          inline RasterizerParams& depthBias(int32_t depthBias, float depthBiasClamp = 0.f, float depthBiasSlopeFactor = 0.f) noexcept { 
            _params.depthBiasEnable = (depthBias || depthBiasClamp || depthBiasSlopeFactor) ? VK_TRUE : VK_FALSE;
            _params.depthBiasConstantFactor = (float)depthBias;
            _params.depthBiasClamp = depthBiasClamp;
            _params.depthBiasSlopeFactor = depthBiasSlopeFactor;
            return *this;
          }
          
          inline VkPipelineRasterizationStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native vulkan descriptor
          inline VkPipelineRasterizationDepthClipStateCreateInfoEXT& _depthClippingDesc() const noexcept { return this->_depthClipping; }
          inline VkPipelineRasterizationLineStateCreateInfoEXT& _lineStateDesc() const noexcept { return this->_lineRasterization; }
       
        private:
          void _copy(const RasterizerParams& rhs) noexcept;

        private:
          mutable VkPipelineRasterizationStateCreateInfo _params{};
          mutable VkPipelineRasterizationDepthClipStateCreateInfoEXT _depthClipping{};
          mutable VkPipelineRasterizationLineStateCreateInfoEXT _lineRasterization{};
          bool _useScissorClipping = false;
        };
        
        
        // -- depth/stencil testing state params -- ----------------------------
        
        /// @class DepthStencilParams
        /// @brief Depth/stencil testing state configuration
        /// @remarks The same DepthStencilParams can (and should) be used to build multiple GraphicsPipeline instances (if needed).
        class DepthStencilParams final {
        public:
          /// @brief Create default depth-test params: "less" -> "keep" / "incrementWrap"(front fail) / "decrementWrap" (back fail)).
          inline DepthStencilParams() noexcept {
            _init(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_ALWAYS);
            frontFaceOp((StencilOp)VK_STENCIL_OP_KEEP, (StencilOp)VK_STENCIL_OP_INCREMENT_AND_WRAP, (StencilOp)VK_STENCIL_OP_KEEP);
            backFaceOp((StencilOp)VK_STENCIL_OP_KEEP, (StencilOp)VK_STENCIL_OP_DECREMENT_AND_WRAP, (StencilOp)VK_STENCIL_OP_KEEP);
            stencilReference(1u);
          }
          /// @brief Create default depth/stencil-test params, with custom status for depth/stencil tests.
          inline DepthStencilParams(bool isDepthEnabled, bool isStencilEnabled) noexcept {
            _init(isDepthEnabled ? VK_TRUE : VK_FALSE, isStencilEnabled ? VK_TRUE : VK_FALSE,
                  VK_COMPARE_OP_LESS, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_ALWAYS);
            frontFaceOp((StencilOp)VK_STENCIL_OP_KEEP, (StencilOp)VK_STENCIL_OP_INCREMENT_AND_WRAP, (StencilOp)VK_STENCIL_OP_KEEP);
            backFaceOp((StencilOp)VK_STENCIL_OP_KEEP, (StencilOp)VK_STENCIL_OP_DECREMENT_AND_WRAP, (StencilOp)VK_STENCIL_OP_KEEP);
            stencilReference(1u);
          }
          /// @brief Initialize depth-test params (no stencil-test).
          /// @param depthComp              Depth-test comparison.
          /// @param frontFaceDepthFailedOp Operation on front depth/stencil pixel when depth-test fails.
          /// @param frontFacePassedOp      Operation on front stencil pixel when depth-test passes.
          /// @param backFaceDepthFailedOp  Operation on back depth/stencil pixel when depth-test fails.
          /// @param backFacePassedOp       Operation on back stencil pixel when depth-test passes.
          inline DepthStencilParams(StencilCompare depthComp, StencilOp frontFaceDepthFailedOp, StencilOp frontFacePassedOp, 
                                    StencilOp backFaceDepthFailedOp, StencilOp backFacePassedOp, uint32_t stencilRef = 1u) noexcept {
            _init(VK_TRUE, VK_FALSE, (VkCompareOp)depthComp, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_ALWAYS);
            frontFaceOp(frontFacePassedOp, frontFaceDepthFailedOp, frontFacePassedOp);
            backFaceOp(backFacePassedOp, backFaceDepthFailedOp, backFacePassedOp);
            stencilReference(stencilRef);
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
                                    StencilOp backFaceFailedOp, StencilOp backFacePassedOp, uint32_t stencilRef = 1u) noexcept {
            _init(VK_FALSE, VK_TRUE, VK_COMPARE_OP_ALWAYS, (VkCompareOp)frontFaceComp, (VkCompareOp)backFaceComp);
            frontFaceOp(frontFaceFailedOp, frontFaceFailedOp, frontFacePassedOp);
            backFaceOp(backFaceFailedOp, backFaceFailedOp, backFacePassedOp);
            stencilReference(stencilRef);
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
                                    uint32_t stencilRef = 1u) noexcept {
            _init(VK_TRUE, VK_TRUE, (VkCompareOp)depthComp, (VkCompareOp)frontFaceComp, (VkCompareOp)backFaceComp);
            frontFaceOp(frontFaceFailedOp, frontFaceDepthFailedOp, frontFacePassedOp);
            backFaceOp(backFaceFailedOp, backFaceDepthFailedOp, backFacePassedOp);
            stencilReference(stencilRef);
          }
          
          DepthStencilParams(const DepthStencilParams&) = default;
          DepthStencilParams& operator=(const DepthStencilParams&) = default;
          ~DepthStencilParams() noexcept = default;


          // -- depth/stencil test operations --

          /// @brief Enable/disable depth-test
          inline DepthStencilParams& enableDepthTest(bool isEnabled) noexcept { _params.depthTestEnable = isEnabled ? TRUE : FALSE; return *this; }
          /// @brief Set depth-test comparison
          inline DepthStencilParams& depthTest(StencilCompare comp) noexcept { _params.depthCompareOp = (VkCompareOp)comp; return *this; }

          /// @brief Enable/disable stencil-test
          inline DepthStencilParams& enableStencilTest(bool isEnabled) noexcept {
            _params.stencilTestEnable = isEnabled ? TRUE : FALSE; return *this;
          }
          /// @brief Set front stencil-test comparison
          inline DepthStencilParams& frontFaceStencilTest(StencilCompare comp) noexcept {
            _params.front.compareOp=(VkCompareOp)comp; return *this;
          }
          /// @brief Set operations to perform on buffer pixels after front depth/stencil-test
          inline DepthStencilParams& frontFaceOp(StencilOp failed, StencilOp depthFailed, StencilOp passed) noexcept {
            _params.front.failOp = (VkStencilOp)failed;
            _params.front.depthFailOp = (VkStencilOp)depthFailed;
            _params.front.passOp = (VkStencilOp)passed;
            return *this;
          }
          /// @brief Set back stencil-test comparison
          inline DepthStencilParams& backFaceStencilTest(StencilCompare comp) noexcept {
            _params.back.compareOp = (VkCompareOp)comp; return *this;
          }
          /// @brief Set operations to perform on buffer pixels after back depth/stencil-test
          inline DepthStencilParams& backFaceOp(StencilOp failed, StencilOp depthFailed, StencilOp passed) noexcept {
            _params.back.failOp = (VkStencilOp)failed;
            _params.back.depthFailOp = (VkStencilOp)depthFailed;
            _params.back.passOp = (VkStencilOp)passed;
            return *this;
          }

          /// @brief Set depth/stencil operation reference (default: 1)
          inline DepthStencilParams& stencilReference(uint32_t stencilRef) noexcept {
            _params.front.reference = _params.back.reference = stencilRef; return *this;
          }
          /// @brief Get depth/stencil operation reference
          inline uint32_t stencilReference() const noexcept { return _params.front.reference; }
          
          // -- depth/stencil masks --
          
          /// @brief Enable/disable pixel writing for depth-test (default: true)
          inline DepthStencilParams& depthMask(bool allowDepthWrite) noexcept {
            _params.depthWriteEnable = allowDepthWrite ? VK_TRUE : VK_FALSE; return *this;
          }
          /// @brief Set pixel read/write mask for stencil-test (default: 0xFF)
          inline DepthStencilParams& stencilMask(uint8_t stencilReadMask, uint8_t stencilWriteMask) noexcept { 
            _params.front.compareMask = _params.back.compareMask = stencilReadMask;
            _params.front.writeMask = _params.back.writeMask = stencilWriteMask;
            return *this;
          }

          inline VkPipelineDepthStencilStateCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native vulkan descriptor
          inline const VkPipelineDepthStencilStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native descriptor
          
        private:
          void _init(VkBool32 enableDepth, VkBool32 enableStencil, VkCompareOp depthComp, 
                     VkCompareOp frontFaceComp, VkCompareOp backFaceComp) noexcept;
        
        private:
          VkPipelineDepthStencilStateCreateInfo _params{};
        };
        
        
        // -- blend state params -- --------------------------------------------
        
        /// @class BlendParams
        /// @brief Blend operation state configuration (color/alpha blend params, common to all render-targets)
        /// @remarks The same BlendParams can (and should) be used to build multiple GraphicsPipeline instances (if needed).
        class BlendParams final {
        public:
          /// @brief Create default blend params: source=one, dest=zero, op=add (writeMask=all).
          inline BlendParams() noexcept
            : BlendParams((BlendFactor)VK_BLEND_FACTOR_ONE, (BlendFactor)VK_BLEND_FACTOR_ZERO, (BlendOp)VK_BLEND_OP_ADD, 
                          (BlendFactor)VK_BLEND_FACTOR_ONE, (BlendFactor)VK_BLEND_FACTOR_ZERO, (BlendOp)VK_BLEND_OP_ADD) {}
          /// @brief Create default blend params with custom status.
          inline BlendParams(bool isEnabled) noexcept
            : BlendParams((BlendFactor)VK_BLEND_FACTOR_ONE, (BlendFactor)VK_BLEND_FACTOR_ZERO, (BlendOp)VK_BLEND_OP_ADD, 
                          (BlendFactor)VK_BLEND_FACTOR_ONE, (BlendFactor)VK_BLEND_FACTOR_ZERO, (BlendOp)VK_BLEND_OP_ADD) { enable(isEnabled); }
          
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

          BlendParams(const BlendParams&) noexcept = default;
          BlendParams& operator=(const BlendParams& rhs) noexcept = default;
          ~BlendParams() noexcept = default;

          /// @brief Set pre-blend operation to perform on RGB value
          /// @param srcFactor   Pre-blend operation to perform on pixel shader output RGB value (any value)
          /// @param destFactor  Pre-blend operation to perform on existing render-target RGB value (any value)
          /// @param op          Color blend operation (between srcColorFactor and destColorFactor)
          inline BlendParams& colorBlend(BlendFactor srcFactor, BlendFactor destFactor, BlendOp op) noexcept {
            _attachementState.srcColorBlendFactor = (VkBlendFactor)srcFactor;
            _attachementState.dstColorBlendFactor = (VkBlendFactor)destFactor;
            _attachementState.colorBlendOp = (VkBlendOp)op;
            return *this;
          }
          /// @brief Set pre-blend operation to perform on alpha value
          /// @param srcFactor   Pre-blend operation to perform on pixel shader output alpha value (one/zero/value containing "Alpha")
          /// @param destFactor  Pre-blend operation to perform on existing render-target alpha value (one/zero/value containing "Alpha")
          /// @param op          Alpha blend operation between (srcAlphaFactor and destAlphaFactor)
          inline BlendParams& alphaBlend(BlendFactor srcFactor, BlendFactor destFactor, BlendOp op) noexcept {
            _attachementState.srcAlphaBlendFactor = (VkBlendFactor)srcFactor;
            _attachementState.dstAlphaBlendFactor = (VkBlendFactor)destFactor;
            _attachementState.alphaBlendOp = (VkBlendOp)op;
            return *this;
          }

          /// @brief Set constant factor (RGBA) for constant blend factors
          /// @remarks The constant factor is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          BlendParams& blendConstant(const ColorChannel constantFactorRgba[4]) noexcept;
          /// @brief Get constant factor (RGBA) used by constant blend factors
          inline const ColorChannel* blendConstant() const noexcept { return _params.blendConstants; }
          
          /// @brief Bit-mask specifying which RGBA components are enabled for writing
          inline BlendParams& targetWriteMask(ColorComponentFlag mask) noexcept {
            _attachementState.colorWriteMask = (VkColorComponentFlags)mask; return *this;
          }
          /// @brief Enable/disable blending
          inline BlendParams& enable(bool isEnabled) noexcept { _attachementState.blendEnable = isEnabled ? VK_TRUE : VK_FALSE; return *this; }
          
          inline VkPipelineColorBlendStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native descriptor
          inline const VkPipelineColorBlendAttachmentState& _attachDesc() const noexcept { return this->_attachementState; }

        private:
          mutable VkPipelineColorBlendStateCreateInfo _params{};
          VkPipelineColorBlendAttachmentState _attachementState{};
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
          BlendPerTargetParams(BlendPerTargetParams&&) noexcept = default;
          BlendPerTargetParams& operator=(const BlendPerTargetParams&) = default;
          BlendPerTargetParams& operator=(BlendPerTargetParams&&) noexcept = default;
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
          BlendPerTargetParams& disableTargetBlend(uint32_t targetIndex) noexcept;

          /// @brief Set constant factor (RGBA) for constant blend factors
          /// @remarks The constant factor is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          BlendPerTargetParams& blendConstant(const ColorChannel constantFactorRgba[4]) noexcept;
          /// @brief Get constant factor (RGBA) used by constant blend factors
          inline const ColorChannel* blendConstant() const noexcept { return _params.blendConstants; }

          inline VkPipelineColorBlendStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native descriptor
          inline const pandora::memory::LightVector<VkPipelineColorBlendAttachmentState>& _attachDesc() const noexcept {
            return this->_attachementsPerTarget;
          }
          
        private:
          mutable VkPipelineColorBlendStateCreateInfo _params{};
          pandora::memory::LightVector<VkPipelineColorBlendAttachmentState> _attachementsPerTarget;
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
        ///          - On Vulkan, pipelines are immutable by default (unlike other renderers):
        ///            to allow direct state changes (viewport, scissor test, depth-stencil test, blending),
        ///            each state that can change must be marked as dynamic.
        ///          - Try to limit the number of dynamic state changes to improve efficiency.
        class GraphicsPipeline final {
        public:
          using Handle = VkPipeline; ///< Bindable graphics pipeline handle

          /// @brief Empty pipeline -- not usable (only useful to store variable not immediately initialized)
          /// @remarks Use GraphicsPipeline::Builder to create an initialized pipeline.
          GraphicsPipeline() = default;
          /// @brief Create pipeline object -- reserved for internal use or advanced usage (prefer GraphicsPipeline::Builder)
          // -> throws: - logic_error if some required states/stages haven't been set.
          //            - runtime_error if pipeline creation fails.
          GraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo, std::shared_ptr<Renderer> renderer, VkPipelineCache cache);

          GraphicsPipeline(const GraphicsPipeline&) = delete;
          GraphicsPipeline(GraphicsPipeline&& rhs) noexcept
            : _pipelineHandle(rhs._pipelineHandle), _renderer(std::move(_renderer)) {
            rhs._pipelineHandle = VK_NULL_HANDLE;
          }
          GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
          GraphicsPipeline& operator=(GraphicsPipeline&& rhs) noexcept {
            this->_pipelineHandle=rhs._pipelineHandle; this->_renderer=std::move(rhs._renderer);
            rhs._pipelineHandle = VK_NULL_HANDLE;
            return *this;
          }
          ~GraphicsPipeline() noexcept { release(); }

          void release() noexcept; ///< Destroy pipeline object

          /// @brief Get native pipeline handle -- reserved for internal use
          inline Handle handle() const noexcept { return this->_pipelineHandle; }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (this->_pipelineHandle == VK_NULL_HANDLE); }

          /// @brief Bindable render target description (swap-chain / texture-target signature)
          /*struct RenderTargetDescription final {
            DataFormat backBufferFormat;
            TargetOutputFlag outputFlags;
          };*/

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
            Builder& setInputLayout(InputLayout inputLayout) noexcept;
            /// @brief Set custom vertex shader input layout description (optional) -- only for Vulkan
            /// @remarks - More efficient than Shader::Builder.createInputLayout (no dynamic alloc, no copy), but specific to Vulkan.
            ///          - For cross-API projects, prefer setInputLayout(Shader::Builder.createInputLayout(...)).
            /// @warning The current inputBindings/layoutAttributes must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, its lifetime is NOT guaranteed by the Builder instance!
            Builder& setInputLayout(VkVertexInputBindingDescription* inputBindings, size_t bindingsLength,
                                    VkVertexInputAttributeDescription* layoutAttributes, size_t attributesLength) noexcept;

            /// @brief Set vertex polygon topology for input stage
            inline Builder& setVertexTopology(VertexTopology topology) noexcept {
              this->_inputTopology.topology = (VkPrimitiveTopology)topology;
#             ifndef __P_DISABLE_TESSELLATION_STAGE
                this->_globalPipelineParams.pTessellationState = nullptr;
#             endif
              return *this;
            }
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              /// @brief Set vertex patch topology for input stage (required for tessellation shaders)
              /// @param controlPoints  Number of patch control points: between 1 and 32 (other values will be clamped).
              inline Builder& setPatchTopology(uint32_t controlPoints) noexcept {
                this->_inputTopology.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
                this->_tessellationState.patchControlPoints = controlPoints;
                this->_globalPipelineParams.pTessellationState = &(this->_tessellationState);
                return *this;
              }
#           endif

            // -- shader stages --

            /// @brief Bind shader stages to pipeline (required)
            /// @warning - If tessellation stages are specified, a tessellation patch topology must be set (setPatchTopology).
            ///          - Vertex shaders with input data require an input layout description (setInputLayout).
            /// @throws invalid_argument if unsupported shaders are provided (compute shaders, or shaders disabled by cmake options).
            /// @warning The current Shader objects must be kept alive (and not released) as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, their lifetime is NOT guaranteed by the Builder instance!
            Builder& setShaderStages(const Shader shaders[], size_t shaderCount);
            /// @brief Remove all shader stages
            Builder& clearShaderStages() noexcept;

            /// @brief Bind (or replace) shader module for a specific stage (at least vertex+fragment or compute required)
            /// @warning - If tessellation stages are specified, a tessellation patch topology must be set (setPatchTopology).
            ///          - Vertex shaders with input data require an input layout description (setInputLayout).
            /// @throws invalid_argument if unsupported shaders are provided (compute shaders, or shaders disabled by cmake options).
            /// @warning The current Shader objects must be kept alive (and not released) as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, their lifetime is NOT guaranteed by the Builder instance!
            Builder& attachShaderStage(const Shader& shaderModule);
            /// @brief Remove a shader stage
            Builder& detachShaderStage(ShaderType stage) noexcept;

            // -- rendering pipeline states --

            /// @brief Bind rasterization state (required)
            /// @warning The current RasterizerParams must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, its lifetime is NOT guaranteed by the Builder instance!
            Builder& setRasterizerState(const RasterizerParams& state) noexcept;
            /// @brief Bind depth/stencil test state (required if depth buffer used)
            /// @warning Required to use a depth/stencil buffer when rendering.
            /// @warning The current DepthStencilParams must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, its lifetime is NOT guaranteed by the Builder instance!
            Builder& setDepthStencilState(const DepthStencilParams& state) noexcept {
              this->_globalPipelineParams.pDepthStencilState = &(state.descriptor()); return *this;
            }
            /// @brief Remove depth/stencil test state (if no depth buffer is used)
            Builder& clearDepthStencilState() noexcept {
              this->_globalPipelineParams.pDepthStencilState = nullptr; return *this;
            }

            /// @brief Bind color/alpha blending state -- common to all render-targets (one of the 2 methods required)
            /// @param constantColorRgba  Only used if the blend state uses BlendFactor::constantColor/constantInvColor
            ///                           (defaults to white if set to NULL).
            /// @warning The current BlendParams must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, its lifetime is NOT guaranteed by the Builder instance!
            Builder& setBlendState(const BlendParams& state) noexcept;
            /// @brief Bind color/alpha blending state -- customized per render-target (one of the 2 methods required)
            /// @param constantColorRgba  Only used if the blend state uses BlendFactor::constantColor/constantInvColor
            ///                           (defaults to white if set to NULL).
            /// @warning The current BlendPerTargetParams must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, its lifetime is NOT guaranteed by the Builder instance!
            Builder& setBlendState(const BlendPerTargetParams& state) noexcept;

            // -- viewports & scissor tests --

            /// @brief Set viewports and scissor-test rectangles (optional: dynamic viewports if ignored)
            /// @param viewports       Pointer to viewport (if 'viewportCount' is 1) or array of viewports.
            ///                        Set to NULL to use dynamic viewports (a call to Renderer.setViewport(s) is then mandatory).
            /// @param viewportCount   Number of viewports (array size of 'viewports', if not NULL).
            ///                        Even if 'viewports' is NULL, this value must be set if 'useDynamicCount' is false.
            /// @param scissorTests    Pointer to scissor-test rectangle (if 'scissorCount' is 1) or array of scissor-test rectangles.
            ///                        Set to NULL to use dynamic scissor-tests (with Renderer.setScissorRectangle(s)).
            /// @param scissorCount    Number of viewports/scissor-tests (and array size of 'viewports' and 'scissorTests' (if not NULL)).
            ///                        Even if 'scissorTests' is NULL, this value must be set if 'useDynamicCount' is false.
            /// @param useDynamicCount Allow different viewport/scissor-test counts to be set during dynamic bindings:
            ///                        only possible if VulkanLoader.isDynamicViewportCountSupported is true.
            /// @remarks The value of viewportCount and scissorCount can't exceed Renderer.maxViewports().
            Builder& setViewports(const Viewport viewports[], size_t viewportCount,
                                  const ScissorRectangle scissorTests[], size_t scissorCount,
                                  bool useDynamicCount = false) noexcept;


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
            /// @param parentCache  Pipeline cache to use for creation -- specific to vulkan (do not fill param for cross-API projects)
            /// @throws - logic_error if some required states/stages haven't been set;
            ///         - runtime_error if pipeline creation fails.
            GraphicsPipeline build(VkPipelineCache parentCache = VK_NULL_HANDLE);


            // -- Vulkan pipeline settings --

            /// @brief Native pipeline creation info -- should only be used to customize advanced settings: flags, parentCache...
            inline VkGraphicsPipelineCreateInfo& descriptor() noexcept { return this->_globalPipelineParams; }

          private:
            uint32_t _setDynamicState(VkInstance instance, VkDynamicState* dynamicStates) noexcept;

          private:
            VkPipelineShaderStageCreateInfo _shaderStages[__P_MAX_DISPLAY_SHADER_NUMBER]{};
            VkPipelineMultisampleStateCreateInfo _multisampleState{};
            VkPipelineViewportStateCreateInfo _viewportState{};
            VkPipelineVertexInputStateCreateInfo _inputState{};
            VkPipelineInputAssemblyStateCreateInfo _inputTopology{};
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              VkPipelineTessellationStateCreateInfo _tessellationState{};
#           endif
            InputLayout _inputLayout = nullptr;
            DynamicArray<VkViewport> _viewports;
            DynamicArray<VkRect2D> _scissors;

            VkGraphicsPipelineCreateInfo _globalPipelineParams{};
            std::shared_ptr<Renderer> _renderer = nullptr;
            bool _copyCommonBlending = true;
            bool _useDynamicCulling = false;
            bool _useBlendConstants = false;
            bool _useDynamicDepthTest = false;
            bool _useDynamicStencilTest = false;
            bool _useDynamicViewportCount = false;
          };
          
        private:
          GraphicsPipeline::Handle _pipelineHandle = VK_NULL_HANDLE;
          std::shared_ptr<Renderer> _renderer = nullptr;
          VkRenderPass _renderPass = VK_NULL_HANDLE; // stored to guarantee lifetime
          VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE; // stored to guarantee lifetime
        };
      }
    }
  }
#endif
