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
--------------------------------------------------------------------------------
Vulkan - RasterizerParams / DepthStencilParams / BlendParams / BlendPerTargetParams
       - GraphicsPipeline / GraphicsPipeline::Builder
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
          /// @param sampleCount       Sample count for multisampling. Use 1 to disable multisampling.
          ///                          Call Renderer.max{Color/Depth/Stencil}SampleCount to make sure the value is supported.
          RasterizerParams(CullMode cull, FillMode fill = FillMode::fill, bool isFrontClockwise = true,
                           bool depthClipping = false, bool scissorClipping = false, uint32_t sampleCount = 1u) noexcept;
          
          RasterizerParams(const RasterizerParams&) noexcept = default;
          RasterizerParams& operator=(const RasterizerParams& rhs) = default;
          ~RasterizerParams() noexcept = default;
          
          /// @brief Choose vertex order of front-facing polygons (true = clockwise / false = counter-clockwise)
          RasterizerParams& vertexOrder(bool isFrontClockwise) noexcept;
          /// @brief Identify polygons to hide: back-facing, front-facing, none.
          /// @remarks To be able to change the cull-mode and/or vertex-order dynamically, set isDynamic=true
          ///          (required extension VK_EXT_extended_dynamic_state, or Vulkan 1.3+).
          inline RasterizerParams& cullMode(CullMode cull, bool isDynamic = false) noexcept {
            _params.cullMode = (VkCullModeFlags)cull; _useDynamicCulling = isDynamic; return *this;
          }
          /// @brief Set filled/wireframe polygon rendering
          inline RasterizerParams& fillMode(FillMode fill) noexcept { _params.polygonMode = (VkPolygonMode)fill; return *this; }
          /// @brief Set sample count for multisampling (anti-aliasing)
          /// @param count       Sample count for multisampling. Use 1 to disable multisampling.
          ///                    Call Renderer.is{Color/Depth/Stencil}SampleCountAvailable to make sure the value is supported.
          /// @param minShading  Minimum fraction of sample shading (only used if 'sampleCount' > 1).
          ///                    A value closer to 1.0 results in smoother shading (typical value example: 0.2).
          ///                    Use 0 to disable sample rate shading.
          inline RasterizerParams& sampleCount(uint32_t count, float minShading = 0.f) noexcept {
            _sampleCount = count; _minSampleShading = minShading; return *this;
          }
          
          /// @brief Enable clipping based on distance
          inline RasterizerParams& depthClipping(bool isEnabled) noexcept { _useDepthClipping = isEnabled; return *this; }
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
          
          inline VkPipelineRasterizationStateCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native vulkan descriptor
          inline const VkPipelineRasterizationStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native descriptor
          inline uint32_t _getSampleCount() const noexcept { return this->_sampleCount; }
          inline float _getMinSampleShading() const noexcept { return this->_minSampleShading; }
          inline bool _isDepthClippingEnabled() const noexcept { return this->_useDepthClipping; }
          inline bool _isScissorClippingEnabled() const noexcept { return this->_useScissorClipping; }
          inline bool _isDynamicCullingEnabled() const noexcept { return this->_useDynamicCulling; }

        private:
          VkPipelineRasterizationStateCreateInfo _params{};
          uint32_t _sampleCount = 1u;
          float _minSampleShading = 0.f;
          bool _useDepthClipping = false;
          bool _useScissorClipping = false;
          bool _useDynamicCulling = false;
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

          /// @brief Enable/disable depth-test
          /// @remarks To be able to change the depth test dynamically, set isDynamic=true
          ///          (required extension VK_EXT_extended_dynamic_state, or Vulkan 1.3+).
          inline DepthStencilParams& enableDepthTest(bool isEnabled, bool isDynamic = false) noexcept {
            _params.depthTestEnable = isEnabled ? TRUE : FALSE; _useDynamicDepthTest = isDynamic;  return *this;
          }
          /// @brief Enable/disable stencil-test
          /// @remarks To be able to change the stencil test dynamically, set isDynamic=true
          ///          (required extension VK_EXT_extended_dynamic_state, or Vulkan 1.3+).
          inline DepthStencilParams& enableStencilTest(bool isEnabled, bool isDynamic = false) noexcept {
            _params.stencilTestEnable = isEnabled ? TRUE : FALSE; _useDynamicStencilTest = isDynamic; return *this;
          }


          // -- depth/stencil test operations --

          /// @brief Set depth-test comparison
          inline DepthStencilParams& depthTest(StencilCompare comp) noexcept { _params.depthCompareOp = (VkCompareOp)comp; return *this; }

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
          inline bool _isDynamicDepthTestEnabled() const noexcept { return this->_useDynamicDepthTest; }
          inline bool _isDynamicStencilTestEnabled() const noexcept { return this->_useDynamicStencilTest; }
          
        private:
          void _init(VkBool32 enableDepth, VkBool32 enableStencil, VkCompareOp depthComp, 
                     VkCompareOp frontFaceComp, VkCompareOp backFaceComp) noexcept;
        
        private:
          VkPipelineDepthStencilStateCreateInfo _params{};
          bool _useDynamicDepthTest = false;
          bool _useDynamicStencilTest = false;
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
          /// @remarks - The constant factor is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          ///          - To be able to change the constant factor dynamically, set isDynamic=true.
          BlendParams& blendConstant(const ColorChannel constantFactorRgba[4], bool isDynamic = false) noexcept;
          /// @brief Get constant factor (RGBA) used by constant blend factors
          inline const ColorChannel* blendConstant() const noexcept { return _params.blendConstants; }
          
          /// @brief Bit-mask specifying which RGBA components are enabled for writing
          inline BlendParams& targetWriteMask(ColorComponentFlag mask) noexcept {
            _attachementState.colorWriteMask = (VkColorComponentFlags)mask; return *this;
          }
          /// @brief Enable/disable blending
          inline BlendParams& enable(bool isEnabled) noexcept { _attachementState.blendEnable = isEnabled ? VK_TRUE : VK_FALSE; return *this; }
          
          inline VkPipelineColorBlendStateCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native vulkan descriptor
          inline const VkPipelineColorBlendStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native descriptor
          inline const VkPipelineColorBlendAttachmentState& _attachDesc() const noexcept { return this->_attachementState; }
          inline bool _isDynamicConstantEnabled() const noexcept { return this->_useDynamicBlendConstants; }

        private:
          VkPipelineColorBlendStateCreateInfo _params{};
          VkPipelineColorBlendAttachmentState _attachementState{};
          bool _useDynamicBlendConstants = false;
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
          /// @remarks - The constant factor is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          ///          - To be able to change the constant factor dynamically, set isDynamic=true.
          BlendPerTargetParams& blendConstant(const ColorChannel constantFactorRgba[4], bool isDynamic = false) noexcept;
          /// @brief Get constant factor (RGBA) used by constant blend factors
          inline const ColorChannel* blendConstant() const noexcept { return _params.blendConstants; }

          inline VkPipelineColorBlendStateCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native vulkan descriptor
          inline const VkPipelineColorBlendStateCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native descriptor
          inline const pandora::memory::LightVector<VkPipelineColorBlendAttachmentState>& _attachDesc() const noexcept {
            return this->_attachementsPerTarget;
          }
          inline bool _isDynamicConstantEnabled() const noexcept { return this->_useDynamicBlendConstants; }
          
        private:
          VkPipelineColorBlendStateCreateInfo _params{};
          pandora::memory::LightVector<VkPipelineColorBlendAttachmentState> _attachementsPerTarget;
          bool _useDynamicBlendConstants = false;
        };


        // ---------------------------------------------------------------------
        // graphics pipeline builder
        // ---------------------------------------------------------------------

        /// @class GraphicsPipeline
        /// @brief Graphics system pipeline stages - shader program & rendering state to execute on GPU
        /// @warning - Can only be used for display shaders (vertex, fragment, geometry, tessellation).
        ///            For compute shaders, a computation pipeline must be created instead!
        ///          - Must be built using GraphicsPipeline::Builder.
        ///          - Must be destroyed BEFORE destroying associated Renderer instance!
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
          GraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo, Renderer* renderer,
                           SharedRenderPass renderPass, SharedResource<VkPipelineLayout> pipelineLayout, VkPipelineCache cache);

          GraphicsPipeline(const GraphicsPipeline&) = delete;
          GraphicsPipeline(GraphicsPipeline&& rhs) noexcept
            : _pipelineHandle(rhs._pipelineHandle), _renderer(rhs._renderer),
              _renderPass(std::move(rhs._renderPass)), _pipelineLayout(std::move(rhs._pipelineLayout)) {
            rhs._pipelineHandle = VK_NULL_HANDLE;
          }
          GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
          GraphicsPipeline& operator=(GraphicsPipeline&& rhs) noexcept {
            this->_pipelineHandle=rhs._pipelineHandle; this->_renderer=rhs._renderer;
            this->_renderPass = std::move(rhs._renderPass); this->_pipelineLayout = std::move(rhs._pipelineLayout);
            rhs._pipelineHandle = VK_NULL_HANDLE;
            return *this;
          }
          ~GraphicsPipeline() noexcept { release(); }

          void release() noexcept; ///< Destroy pipeline object

          /// @brief Get native pipeline handle -- reserved for internal use
          inline Handle handle() const noexcept { return this->_pipelineHandle; }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (this->_pipelineHandle == VK_NULL_HANDLE); }
          /// @brief Verify if pipeline is built for dynamic rendering (true) or render passes (false)
          inline bool isDynamicRendering() const noexcept { return (this->_renderPass == nullptr); }

          // ---

          /// @class GraphicsPipeline.Builder
          /// @brief Graphics pipeline setter and builder
          /// @warning The latest Viewport and ScissorRectangle arrays must be kept alive as long as the Builder is used.
          ///          To avoid unnecessary copies and processing, their lifetime is NOT guaranteed by the Builder instance!
          class Builder final {
          public:
            /// @brief Create pipeline builder
            Builder(Renderer& renderer) noexcept;
            Builder(const Builder&) = delete;
            Builder(Builder&&) noexcept = default;
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&&) noexcept = default;
            ~Builder() noexcept;

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
                this->_descriptor.pTessellationState = nullptr;
#             endif
              return *this;
            }
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              /// @brief Set vertex patch topology for input stage (required for tessellation shaders)
              /// @param controlPoints  Number of patch control points: between 1 and 32 (other values will be clamped).
              inline Builder& setPatchTopology(uint32_t controlPoints) noexcept {
                this->_inputTopology.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
                this->_tessellationTopology.patchControlPoints = controlPoints;
                this->_descriptor.pTessellationState = &(this->_tessellationTopology);
                return *this;
              }
#           endif

            // -- shader stages --

            /// @brief Bind shader stages to pipeline (required)
            /// @warning - If tessellation stages are specified, a tessellation patch topology must be set (setPatchTopology).
            ///          - Vertex shaders with input data require an input layout description (setInputLayout).
            /// @throws invalid_argument if unsupported shaders are provided (compute shaders, or shaders disabled by cmake options).
            Builder& setShaderStages(const Shader shaders[], size_t shaderCount);
            /// @brief Remove all shader stages
            Builder& clearShaderStages() noexcept;

            /// @brief Bind (or replace) shader module for a specific stage (at least vertex+fragment or compute required)
            /// @warning - If tessellation stages are specified, a tessellation patch topology must be set (setPatchTopology).
            ///          - Vertex shaders with input data require an input layout description (setInputLayout).
            /// @throws invalid_argument if unsupported shaders are provided (compute shaders, or shaders disabled by cmake options).
            Builder& attachShaderStage(const Shader& shaderStage);
            /// @brief Remove a shader stage
            Builder& detachShaderStage(ShaderType stage) noexcept;

            // -- rendering pipeline states --

            /// @brief Bind rasterization state (required)
            /// @throws runtime_error if dynamic culling/order is requested but not supported by current Renderer
            ///                       (verify with Renderer.isExtendedDynamicStateSupported).
            Builder& setRasterizerState(const RasterizerParams& state);
            /// @brief Bind depth/stencil test state (required if depth buffer used)
            /// @warning Required to use a depth/stencil buffer when rendering.
            /// @throws runtime_error if dynamic septh/stencil test is requested but not supported by current Renderer
            ///                       (verify with Renderer.isExtendedDynamicStateSupported).
            Builder& setDepthStencilState(const DepthStencilParams& state);
            /// @brief Remove depth/stencil test state (if no depth buffer is used)
            inline Builder& clearDepthStencilState() noexcept {
              this->_descriptor.pDepthStencilState = nullptr; this->_useDynamicDepthTest = false; return *this;
            }

            /// @brief Bind color/alpha blending state -- common to all render-targets (one of the 2 methods required)
            Builder& setBlendState(const BlendParams& state) noexcept;
            /// @brief Bind color/alpha blending state -- customized per render-target (one of the 2 methods required)
            Builder& setBlendState(const BlendPerTargetParams& state) noexcept;

            // -- viewports & scissor tests --

            /// @brief Set viewports and scissor-test rectangles (optional: dynamic viewports if ignored)
            /// @param viewports       Pointer to viewport (if 'viewportCount' is 1) or array of viewports.
            ///                        Set to NULL with viewportCount > 0 to use dynamic viewports (with Renderer.setViewport(s)).
            /// @param viewportCount   Number of viewports (array size of 'viewports', if not NULL).
            ///                        Even if 'viewports' is NULL, this value must be set if 'useDynamicCount' is false.
            ///                        The count can't be 0, unless 'useDynamicCount' is true.
            /// @param scissorTests    Pointer to scissor-test rectangle (if 'scissorCount' is 1) or array of scissor-test rectangles.
            ///                        Set to NULL with scissorCount > 0 to use dynamic scissor-tests (with Renderer.setScissorRectangle(s)).
            /// @param scissorCount    Number of viewports/scissor-tests (and array size of 'viewports' and 'scissorTests' (if not NULL)).
            ///                        Even if 'scissorTests' is NULL, this value must be set if 'useDynamicCount' is false.
            ///                        The count can't be 0, unless 'useDynamicCount' is true.
            /// @param useDynamicCount Allow different viewport/scissor-test counts to be set during dynamic bindings:
            ///                        only possible if VulkanLoader.isDynamicViewportCountSupported is true
            ///                        and if Renderer.isExtendedDynamicStateSupported is true.
            /// @remarks The value of viewportCount and scissorCount can't exceed Renderer.maxViewports().
            /// @throws runtime_error if dynamic count is requested but not supported by driver.
            /// @warning The current Viewport and ScissorRectangle arrays must be kept alive as long as the Builder is used.
            ///          To avoid unnecessary copies and processing, their lifetime is NOT guaranteed by the Builder instance!
            Builder& setViewports(const Viewport viewports[], size_t viewportCount,
                                  const ScissorRectangle scissorTests[], size_t scissorCount,
                                  bool useDynamicCount = false);


            // -- pipeline build --

            /// @brief Provide pipeline layout for globals: uniforms, storages, samplers... (optional)
            /// @param layout  Pipeline layout description object.
            /// @remarks 'layout' can be built with 'createGlobalLayout'.
            /// @warning - Compatible uniforms/storages/samplers will need to be set.
            ///          - For cross-API projects, note that this is only compatible with other low-level APIs (D3D12...).
            ///            Higher-level APIs (D3D11, OpenGL) do not have any concept of pipeline layouts.
            Builder& setGlobalLayout(GlobalLayout layout) noexcept {
              _pipelineLayoutObj = std::move(layout);
              return *this;
            }
            /// @brief Provide render pass definition (format/inputs/dependencies) + number of render-targets (required)
            /// @param renderTargetCount  Number of render-targets used with this pipeline (framebuffers, texture targets...).
            /// @param renderPass         Render-pass definition object.
            /// @remarks 'renderPass' can be build with 'createRenderPass'.
            /// @warning - The pipeline will need to be used with compatible render-targets only.
            ///          - For cross-API projects, note that this is only compatible with other low-level APIs (D3D12...).
            ///            Higher-level APIs (D3D11, OpenGL) do not have any concept of render passes.
            inline Builder& setRenderPass(SharedRenderPass renderPass) noexcept {
              _maxColorAttachmentCount = (renderPass != nullptr) ? renderPass->maxColorAttachmentCount : 1;
              _renderPassObj = std::move(renderPass);
              _descriptor.pNext = nullptr;
              return *this;
            }
#           if defined(VK_HEADER_VERSION) && VK_HEADER_VERSION >= 197
              /// @brief Provide dynamic rendering definition (formats only)
              ///        The Renderer must be built either for Vulkan 1.3+ or with extension "VK_KHR_dynamic_rendering",
              ///        with argument extensions.allowDynamicRendering==true.
              ///        Verify support for dynamic rendering with Renderer.isDynamicRenderingSupported.
              /// @throws runtime_error if dynamic rendering is not supported by renderer.
              /// @warning - The pipeline will need to be used with compatible render-targets only.
              ///          - The current VkPipelineRenderingCreateInfoKHR must be kept alive as long as the Builder is used.
              ///            To avoid unnecessary copies and processing, its lifetime is NOT guaranteed by the Builder instance!
              Builder& setRenderPass(const VkPipelineRenderingCreateInfoKHR& dynamicRenderingInfo);
#           endif

            /// @brief Build a graphics pipeline (based on current params)
            /// @param parentCache  Pipeline cache to use for creation -- specific to vulkan (do not fill param for cross-API projects)
            /// @throws - logic_error if some required states/stages haven't been set;
            ///         - runtime_error if pipeline creation fails.
            GraphicsPipeline build(VkPipelineCache parentCache = VK_NULL_HANDLE);


            // -- Vulkan pipeline description factory --

            /// @brief Create pipeline layout for globals: uniforms, storages, samplers...
            /// @remarks The layout object can be used for multiple pipelines, as long as they share the same Renderer instance.
            /// @warning For cross-API projects, note that this is only compatible with other low-level APIs (D3D12...).
            /// @throws runtime_error on creation failure
            GlobalLayout createGlobalLayout(const VkPipelineLayoutCreateInfo& params);
            /// @brief Create render pass definition: targets, formats, inputs, dependencies...
            /// @remarks The render pass object can be used for multiple pipelines, as long as they share the same Renderer instance.
            /// @warning For cross-API projects, note that this is only compatible with other low-level APIs (D3D12...).
            /// @throws runtime_error on creation failure
            SharedRenderPass createRenderPass(const VkRenderPassCreateInfo& params);


            // -- Vulkan pipeline settings --

            /// @brief Native pipeline creation info -- should only be used to customize advanced settings: flags, cache...
            inline VkGraphicsPipelineCreateInfo& descriptor() noexcept { return this->_descriptor; }

          private:
            uint32_t _setDynamicState(VkDynamicState* dynamicState) noexcept;

          private:
            VkPipelineShaderStageCreateInfo _shaderStagesDesc[__P_MAX_DISPLAY_SHADER_NUMBER]{};
            Shader _shaderStagesObj[__P_MAX_DISPLAY_SHADER_NUMBER]{};

            VkPipelineRasterizationStateCreateInfo _rasterizationDesc{};
            VkPipelineDepthStencilStateCreateInfo _depthStencilDesc{};
            VkPipelineMultisampleStateCreateInfo _multisampleDesc{};
            VkPipelineColorBlendStateCreateInfo _blendDesc{};
            VkPipelineRasterizationLineStateCreateInfoEXT _lineRasterDesc{};

            VkPipelineViewportStateCreateInfo _viewportsDesc{};
            VkPipelineVertexInputStateCreateInfo _inputLayoutDesc{};
            VkPipelineInputAssemblyStateCreateInfo _inputTopology{};
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              VkPipelineTessellationStateCreateInfo _tessellationTopology{};
#           endif
            VkPipelineRasterizationDepthClipStateCreateInfoEXT _depthClippingDesc{};
            InputLayout _inputLayoutObj = nullptr;

            VkGraphicsPipelineCreateInfo _descriptor{}; // main descriptor
            Renderer* _renderer = nullptr;

            SharedRenderPass _renderPassObj;
            GlobalLayout _pipelineLayoutObj;
            DynamicArray<VkPipelineColorBlendAttachmentState> _blendAttachmentsPerTarget;
            const Viewport* _viewports = nullptr;
            const ScissorRectangle* _scissorTests = nullptr;
            uint32_t _maxColorAttachmentCount = 0;
            bool _useBlendPerTarget = false;
            bool _useDynamicCulling = false;
            bool _useDynamicDepthTest = false;
            bool _useDynamicStencilTest = false;
            bool _useDynamicBlendConstants = false;
            bool _useDynamicViewportCount = false;
          };
          
        private:
          GraphicsPipeline::Handle _pipelineHandle = VK_NULL_HANDLE;
          Renderer* _renderer = nullptr;
          SharedRenderPass _renderPass = nullptr;                     // stored to guarantee lifetime & destruction
          SharedResource<VkPipelineLayout> _pipelineLayout = nullptr; // stored to guarantee lifetime & destruction
        };
      }
    }
  }
#endif
