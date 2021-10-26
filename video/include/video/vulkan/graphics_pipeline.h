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
# include "../common_types.h"
# include "./api/types.h" // includes vulkan
# include "./viewport.h"  // includes vulkan
# include "./scissor.h"   // includes vulkan
# include "./shader.h"    // includes vulkan
# include "./renderer.h"  // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        /// @class GraphicsPipeline
        /// @brief GPU graphics pipeline stages - shader bindings & rendering state
        /// @remarks - To use the pipeline shaders/states, bind the pipeline to the associated renderer.
        ///          - On Vulkan, pipelines are immutable by default (unlike other renderers):
        ///            to allow direct state changes (viewport, scissor test, depth-stencil test, blending),
        ///            each state that can change must be marked as dynamic.
        ///          - Try to limit the number of dynamic state changes to improve efficiency.
        class GraphicsPipeline final {
        public:
          using Handle = VkPipeline;

          /// @brief Create usable shader stage object -- reserved for internal use or advanced usage
          /// @remarks Prefer GraphicsPipeline::Builder for standard usage.
          /// @throws runtime_error if pipeline creation fails.
          GraphicsPipeline(std::shared_ptr<Renderer> renderer, const VkGraphicsPipelineCreateInfo& pipelineInfo,
                           VkPipelineCache parentCache = VK_NULL_HANDLE);

          GraphicsPipeline() = default; ///< Empty pipeline -- not usable (only useful to store variable not immediately initialized)
          GraphicsPipeline(const GraphicsPipeline&) = delete;
          GraphicsPipeline(GraphicsPipeline&& rhs) noexcept
            : _pipeline(rhs._pipeline), _renderer(std::move(_renderer)) {
            rhs._pipeline = VK_NULL_HANDLE;
          }
          GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
          GraphicsPipeline& operator=(GraphicsPipeline&& rhs) noexcept {
            this->_pipeline=rhs._pipeline; this->_renderer=rhs._renderer;
            rhs._pipeline = VK_NULL_HANDLE;
            return *this;
          }
          ~GraphicsPipeline() noexcept { release(); }
          void release() noexcept;

          // -- accessors --

          /// @brief Get native pipeline handle -- for internal use or advanced features
          inline Handle handle() const noexcept { return this->_pipeline; }
          inline bool isEmpty() const noexcept { return (this->_pipeline == VK_NULL_HANDLE); } ///< Verify if initialized (false) or empty/moved/released (true)


          // -- create pipeline --

          /// @brief Bindable render target description (swap-chain / texture-target signature)
          struct RenderTargetDescription final {
            DataFormat backBufferFormat;
            TargetOutputFlag outputFlags;
          };

          // ---

          /// @class GraphicsPipeline.Builder
          /// @brief Graphics pipeline settings and build.
          class Builder final {
          public:
            Builder() noexcept;

            // -- pipeline stages --

            /// @brief Bind vertex shader input layout + set vertex topology (optional)
            /// @param inputLayout  Vertex shader input layout description handle. This can't be NULL if the vertex shader has input params.
            /// @param topology     Topology of input vertices (points, lines, triangles...)
            /// @warning Required if vertex shader uses input locations.
            Builder& setInputLayout(InputLayoutHandle inputLayout, VertexTopology topology = VertexTopology::triangles) noexcept;
            /// @brief Bind shader stages to pipeline (required)
            /// @param shaders 
            /// @param shaderCount 
            /// @warning If tessellation stages are specified, 'setTessellationState' must be called too.
            Builder& setShaderStages(const Shader shaders[], size_t shaderCount);
            /// @brief Set tessellation stage params (optional)
            /// @warning Required if some tessellation stages were specified in 'setShaderStages'.
            Builder& setTessellationState(const VkPipelineTessellationStateCreateInfo* state) noexcept {
              this->_graphicsPipeline.pTessellationState = state; return *this;
            }

            /// @brief Set viewports and scissor-test rectangles (required)
            /// @param viewports      Viewport pointer (if 'viewportCount' is 1) or array of viewports.
            ///                       Set to NULL to use dynamic viewports (a call to Renderer.setViewport(s) is mandatory).
            /// @param scissorTests   Scissor-test rectangle pointer (if 'viewportCount' is 1) or array of scissor-test rectangles.
            ///                       Set to NULL to use dynamic scissor-tests (a call to Renderer.setScissorRectangle(s) is mandatory).
            /// @param viewportCount  Number of viewports/scissor-tests (and array size of 'viewports' and 'scissorTests' (if not NULL)).
            ///                       Dynamic viewports/scissor-tests must use the same count if 'isDynamicCount' is false.
            ///                       To make this number dynamic too, use 0 for 'viewportCount'.
            /// @param isDynamicCount Allow viewport/scissor-test count to change dynamically:
            ///                       only possible if VulkanLoader.isDynamicViewportCountSupported is true.
            Builder& setViewports(const Viewport::Descriptor viewports[], const ScissorRectangle::Descriptor scissorTests[],
                                  size_t viewportCount, bool isDynamicCount = false) noexcept;

            // -- rendering states --

            /// @brief Bind rasterization state (required)
            Builder& setRasterizerState(const VkPipelineRasterizationStateCreateInfo& state) noexcept {
              this->_graphicsPipeline.pRasterizationState = &state; return *this;
            }
            /// @brief Bind color blending state (required)
            Builder& setBlendState(const VkPipelineColorBlendStateCreateInfo& state) noexcept {
              this->_graphicsPipeline.pColorBlendState = &state; return *this;
            }
            /// @brief Bind depth/stencil test state (optional)
            /// @warning Required to use a depth/stencil buffer when rendering.
            Builder& setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state) noexcept {
              this->_graphicsPipeline.pDepthStencilState = &state; return *this;
            }
            /// @brief Bind multisampling state (optional)
            Builder& setMultisampleState(const VkPipelineMultisampleStateCreateInfo& state) noexcept {
              this->_graphicsPipeline.pMultisampleState = &state; return *this;
            }

            /// @brief Set Vulkan-specific pipeline flags -- warning: not available in other APIs -> do not use for cross-API projects
            Builder& setFlags(VkPipelineCreateFlagBits flags, GraphicsPipeline::Handle basePipeline = VK_NULL_HANDLE, 
                              int32_t basePipelineIndex = -1, VkPipelineCache parentCache = VK_NULL_HANDLE) noexcept;

            // -- pipeline build --

            /// @brief Build a graphics pipeline (based on current params)
            /// @throws - invalid_argument if renderer is NULL or no render target description is provided;
            ///         - logic_error if some required states/stages haven't been set;
            ///         - runtime_error if pipeline creation fails.
            GraphicsPipeline create(std::shared_ptr<Renderer> renderer,
                                    GraphicsPipeline::RenderTargetDescription renderTargets[], size_t renderTargetCount);
            
          private:
            uint32_t _setDynamicState(VkInstance instance, VkDynamicState* dynamicStates) noexcept;

          private:
            VkPipelineVertexInputStateCreateInfo _inputLayout{};
            VkPipelineInputAssemblyStateCreateInfo _inputTopology{};
            DynamicArray<VkPipelineShaderStageCreateInfo> _shaderStages;
            VkPipelineViewportStateCreateInfo _viewport{};
            VkPipelineCache _parentCache = VK_NULL_HANDLE;

            VkGraphicsPipelineCreateInfo _graphicsPipeline{};
            bool _useDynamicCulling = false;
            bool _useBlendConstants = false;
            bool _useDynamicDepthTest = false;
            bool _useDynamicStencilTest = false;
            bool _useDynamicViewportCount = false;
          };


        private:
          std::shared_ptr<Renderer> _renderer = nullptr;
          GraphicsPipeline::Handle _pipeline = VK_NULL_HANDLE;
          VkRenderPass _renderPass = VK_NULL_HANDLE;
          VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        };
      }
    }
  }
#endif
