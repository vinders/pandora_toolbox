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
# include <memory/light_vector.h>
# include <hardware/display_monitor.h>
# include "./_private/_shared_resource.h" // includes D3D11
# include "./scissor.h"        // includes D3D11
# include "./viewport.h"       // includes D3D11
# include "./shader.h"         // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class SwapChain;
        class GraphicsPipeline;

        /// @class Renderer
        /// @brief Direct3D rendering device and context (specific to adapter)
        /// @warning - Renderer is the main Direct3D resource, and should be kept alive while the program runs.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new Renderer must be created.
        ///          - Accessors to native D3D11 resources should be reserved for internal usage or for advanced features.
        /// @remarks - To render on display output, create a SwapChain + setActiveRenderTarget with it and a depth buffer.
        ///          - To render to a texture, create a TextureTarget2D + setActiveRenderTarget with it and a depth buffer.
        ///          - Multiple active targets can be used simultaneously, to render the same image on multiple outputs/textures.
        ///          - Multi-window rendering (same adapter): alternate between different SwapChain instances on the same Renderer.
        ///          - Multi-window rendering (different adapters): use different Render instances with their own SwapChain.
        ///          - Split-screen rendering (same window): alternate between different Viewport instances on the same SwapChain.
        ///          - Optimization: minimize shader changes, state changes, drawing calls: sort by shader, then texture/material, then other states.
        ///          - Optimization: order of meshes and polygons: front to back (if a depth buffer is enabled).
        ///          - Optimization: align vertex buffer entries to a multiple of 16-byte per entry (add padding if necessary).
        class Renderer final {
        public:
          /// @brief Create Direct3D rendering device and context
          /// @param monitor       Target display monitor for the renderer: used to determine the adapter to choose.
          /// @param featureLevels Feature levels to support, ordered from highest to lowest (example: { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 }):
          ///                      - The system uses the highest available device level (based on 'maxLevel' and Cmake option CWORK_D3D11_VERSION);
          ///                      - If some feature level is not available, the level below is used (and so on);
          ///                      - If the minimum level is too high for current system, an exception occurs;
          ///                      - NULL value: use default feature level (max available).
          /// @param featureLevelCount  Number of values in 'featureLevels' (required if 'featureLevels' is not NULL).
          /// @throws - out_of_range: if minimum value in featureLevels is too high.
          ///         - runtime_error: creation failure.
          Renderer(const pandora::hardware::DisplayMonitor& monitor, const D3D_FEATURE_LEVEL* featureLevels = nullptr, size_t featureLevelCount = 0);
          /// @brief Destroy device and context resources
          ~Renderer() noexcept { _destroy(); }
          
          Renderer(const Renderer&) = delete;
          Renderer(Renderer&& rhs) noexcept;
          Renderer& operator=(const Renderer&) = delete;
          Renderer& operator=(Renderer&& rhs) noexcept;
          
          /// @brief Flush command buffer
          /// @remarks Should only be called: - just before a long CPU wait (ex: sleep)
          ///                                 - to wait for object destruction
          inline void flush() noexcept { this->_context->Flush(); }

          
          // -- accessors --
          
          inline DeviceHandle device() const noexcept { return this->_device; }    ///< Get Direct3D rendering device
          inline DeviceContext context() const noexcept { return this->_context; } ///< Get Direct3D device context
          inline DeviceResourceManager resourceManager() const noexcept { return this->_device; } ///< Get resource manager (to build resources such as shaders)
          inline uint32_t dxgiLevel() const noexcept { return this->_dxgiLevel; }  ///< Get available DXGI level on current system (1-6)
          inline D3D_FEATURE_LEVEL featureLevel() const noexcept { return this->_deviceLevel; } ///< Get available feature level on current device (11.0/11.1+)
          
          /// @brief Read device adapter VRAM size
          /// @returns Read success
          bool getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept;
          /// @brief Convert color/depth/component data format to native format (usable in input layout description) -- see "video/d3d11/shader.h".
          static constexpr inline DXGI_FORMAT toLayoutFormat(DataFormat format) noexcept { return _getDataFormatComponents(format); }

          /// @brief Convert standard sRGB(A) color to gamma-correct linear RGB(A)
          /// @remarks Should be called to obtain colors to use with 'clearView(s)', 'setCleanActiveRenderTarget(s)', 'RendererStateFactory.create<...>Filter'
          static void sRgbToGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept;
          
          
          // -- feature support --
          
          /// @brief Detect current color space used by display monitor -- available with Direct3D 11.1+ / DXGI 1.4+ / Windows 10+
          /// @remarks - Should be called to know if HDR can be properly displayed.
          ///          - May return values not listed in the 'ColorSpace' enum, if the monitor uses an uncommon color space
          ///            (can be cast to DXGI_COLOR_SPACE_TYPE values to verify the actual value).
          ///          - Returns "unknown" if the detection fails, for example if color spaces are not supported
          ///            (in that case, it's better to default to sRGB and let the user choose to enable HDR).
          ///          - Not supported by other APIs (Vulkan, OpenGL...): they'll return "unknown".
          ColorSpace getMonitorColorSpace(const pandora::hardware::DisplayMonitor& target) const noexcept;
          
          /// @brief Get max supported color sample count for multisampling (anti-aliasing)
          inline uint32_t maxColorSampleCount(DataFormat format) const noexcept { return _maxSampleCount(_getDataFormatComponents(format)); }
          /// @brief Get max supported depth sample count for multisampling (anti-aliasing)
          inline uint32_t maxDepthSampleCount(DataFormat format) const noexcept { return _maxSampleCount(_getDataFormatComponents(format)); }
          /// @brief Get max supported stencil sample count for multisampling (anti-aliasing)
          inline uint32_t maxStencilSampleCount(DataFormat format) const noexcept { return _maxSampleCount(_getDataFormatComponents(format)); }
          /// @brief Screen tearing supported (variable refresh rate display)
          bool isTearingAvailable() const noexcept;
          
          
          // -- pipeline status operations - shaders / states --

          /// @brief Bind graphics pipeline to the rendering device
          ///        (topology, input-assembler stage, shader stages, pipeline states, viewport/scissor descriptors)
          /// @param pipeline  Valid graphics pipeline
          void bindGraphicsPipeline(GraphicsPipelineHandle pipeline) noexcept;
          /// @brief Bind compute shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind compute shader.
          inline void bindComputeShader(Shader::Handle shader) noexcept { this->_context->CSSetShader((ID3D11ComputeShader*)shader, nullptr, 0); }


          // -- render target operations --

          /// @brief Max number of simultaneous viewports/scissor-test rectangles per pipeline
          constexpr inline size_t maxViewports() noexcept { return D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE; }
          
          /// @brief Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
          /// @warning Should only be used if the GraphicsPipeline was configured with dynamic viewports.
          void setViewports(const Viewport* viewports, size_t numberViewports) noexcept;
          /// @brief Replace rasterizer viewport (3D -> 2D projection rectangle)
          /// @warning Should only be used if the GraphicsPipeline was configured with dynamic viewports.
          inline void setViewport(const Viewport& viewport) noexcept {
            this->_context->RSSetViewports(1u, (const D3D11_VIEWPORT*)&viewport);
            this->_currentViewportScissorId = 0;
          }

          /// @brief Set rasterizer scissor-test rectangle(s)
          /// @warning Should only be used if the GraphicsPipeline was configured with dynamic scissor-test.
          void setScissorRectangles(const ScissorRectangle* rectangles, size_t numberRectangles) noexcept;
          /// @brief Set rasterizer scissor-test rectangle
          /// @warning Should only be used if the GraphicsPipeline was configured with dynamic scissor-test.
          inline void setScissorRectangle(const ScissorRectangle& rectangle) noexcept {
            this->_context->RSSetScissorRects(1u, (const D3D11_RECT*)&rectangle);
            this->_currentViewportScissorId = 0;
          }
          
          // ---

          /// @brief Max number of simultaneous render-target views (swap-chains, texture targets...)
          constexpr inline size_t maxRenderTargets() noexcept { return D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; }
          
          /// @brief Clear render-targets content + depth buffer: reset to 'clearColorRgba' and to depth 1
          /// @remarks - Recommended before drawing frames that don't cover the whole buffer (unless keeping 'dirty' previous data is desired).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.sRgbToGammaCorrectColor).
          void clearViews(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          /// @brief Clear render-target content + depth buffer: reset to 'clearColorRgba' and to depth 1
          /// @remarks - Recommended before drawing frames that don't cover the whole buffer (unless keeping 'dirty' previous data is desired).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.sRgbToGammaCorrectColor).
          void clearView(RenderTargetView view, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          
          /// @brief Bind/replace active render-target(s) in Renderer (multi-target)
          /// @warning Binding multiple targets simultaneously is only possible if:
          ///          - their width/height is the same;
          ///          - their number of frame-buffers is the same;
          ///          - their component format is the same;
          ///          - their MSAA options are the same.
          /// @remarks - This call allows draw/render operations to fill SwapChain back-buffers and/or TextureBuffer instances.
          ///          - It should be called before the first iteration of the program loop.
          ///          - It should be called everytime the rendering needs to target a different resource (ex: render to texture, then swap-chain).
          ///          - It should be called again after deleting/resizing any SwapChain or TextureBuffer.
          ///          - Multiple render-targets can be used simultaneously: pass an array as 'views' and its size as 'numberViews'.
          ///          - Calling it with 0 views (or a NULL view) disables active render-targets.
          void setActiveRenderTargets(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer = nullptr) noexcept;
          /// @brief Bind/replace active render-target in Renderer (single target)
          /// @remarks - This call allows draw/render operations to fill SwapChain back-buffers and/or TextureBuffer instances.
          ///          - It should be called before the first iteration of the program loop.
          ///          - It should be called everytime the rendering needs to target a different resource (ex: render to texture, then swap-chain).
          ///          - It should be called again after deleting/resizing any SwapChain or TextureBuffer.
          ///          - Calling it with a NULL view disables active render-targets.
          inline void setActiveRenderTarget(RenderTargetView view, DepthStencilView depthBuffer = nullptr) noexcept {
            this->_context->OMSetRenderTargets((UINT)1u, &view, depthBuffer);
          }
          
          /// @brief Bind/replace active render-target(s) in Renderer (multi-target) + clear render-targets/buffers
          /// @remarks - If the render-targets contain new buffers (or resized), this is the recommended method (to reset them before using them).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.sRgbToGammaCorrectColor).
          void setCleanActiveRenderTargets(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer, 
                                           const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          /// @brief Bind/replace active render-target in Renderer (single target) + clear render-target/buffer
          /// @remarks - If the render-target is a new buffer (or resized), this is the recommended method (to reset it before using it).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.sRgbToGammaCorrectColor).
          void setCleanActiveRenderTarget(RenderTargetView view, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          
          
          // -- primitive binding --
          
          /// @brief Max slots (or array size from first slot) for vertex buffers
          constexpr inline size_t maxVertexBufferSlots() noexcept { return D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; }
          
          /// @brief Bind active vertex array buffer to input stage slot
          /// @param slotIndex      Input stage slot to set/replace with vertex array buffer
          /// @param vertexBuffer   Handle of vertex array buffer (StaticBuffer.handle() / DynamicBuffer.handle() or ID3D11Buffer*) or NULL to clear slot
          /// @param strideByteSize Size of one vertex entry (stride)
          /// @param byteOffset     Byte offset of first vertex info to use in buffer (0 to start from beginning)
          inline void bindVertexArrayBuffer(uint32_t slotIndex, BufferHandle vertexArrayBuffer, 
                                            unsigned int strideByteSize, unsigned int byteOffset = 0u) noexcept {
            this->_context->IASetVertexBuffers((UINT)slotIndex, 1u, &vertexArrayBuffer, &strideByteSize, &byteOffset);
          }
          /// @brief Bind multiple vertex array buffers to consecutive input stage slots (example: vertex buffer + instance buffer)
          /// @param firstSlotIndex  First input stage slot to set/replace with first vertex array buffer (next buffers will use next slots)
          /// @param length          Size of arrays 'vertexBuffers', 'strideByteSizes' and 'offsets' (same size required)
          /// @param vertexBuffers   Array of vertex array buffers (size defined by 'length')
          /// @param strideByteSizes Array of "vertex entry size (stride)" for each buffer
          /// @param byteOffsets     Array of "byte offset" for each buffer
          inline void bindVertexArrayBuffers(uint32_t firstSlotIndex, size_t length, const BufferHandle* vertexArrayBuffers, 
                                             unsigned int* strideByteSizes, unsigned int* byteOffsets) noexcept {
            this->_context->IASetVertexBuffers((UINT)firstSlotIndex, (UINT)length, vertexArrayBuffers, strideByteSizes, byteOffsets);
          }
          /// @brief Bind active vertex index buffer (indexes for vertex array buffer) to input stage
          /// @param indexBuffer  Handle of vertex index buffer (StaticBuffer.handle() / DynamicBuffer.handle() or ID3D11Buffer*)
          /// @param indexFormat  Index data type (unsigned int 32 / 64)
          /// @param offset       Byte offset of first vertex index to use in buffer (0 to start from beginning)
          inline void bindVertexIndexBuffer(BufferHandle indexBuffer, VertexIndexFormat indexFormat, uint32_t byteOffset = 0u) noexcept {
            this->_context->IASetIndexBuffer(indexBuffer, (DXGI_FORMAT)indexFormat, (UINT)byteOffset);
          }
          /// @brief Set dynamic vertex polygon topology for input stage
          inline void setVertexTopology(VertexTopology topology) noexcept { this->_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology); }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            /// @brief Set dynamic vertex patch topology for input stage (for vertex/tessellation shaders)
            /// @param controlPoints  Number of patch control points: between 1 and 32 (other values will be clamped).
            void setVertexPatchTopology(uint32_t controlPoints) noexcept;
#         endif
          
          
          // -- primitive drawing --
          
          /// @brief Draw active vertex buffer (not indexed)
          /// @param vertexCount  Number of vertices to draw (elements)
          /// @param vertexOffset Number of vertices to skip in buffer (elements)
          inline void draw(uint32_t vertexCount, uint32_t vertexOffset = 0u) noexcept { 
            this->_context->Draw((UINT)vertexCount, (UINT)vertexOffset); 
          }
          /// @brief Draw active vertex buffer (indexed: active index buffer)
          /// @param indexCount            Number of indexes to draw (elements)
          /// @param indexOffset           Number of indexes to skip in buffer (elements)
          /// @param vertexOffsetFromIndex Value added to each index (to read different vertices)
          inline void drawIndexed(uint32_t indexCount, uint32_t indexOffset = 0u, int32_t vertexOffsetFromIndex = 0u) noexcept {
            this->_context->DrawIndexed((UINT)indexCount, (UINT)indexOffset, (INT)vertexOffsetFromIndex);
          }
          
          /// @brief Draw multiple instances of active vertex buffer (not indexed)
          /// @param instanceCount          Number of instances to draw
          /// @param instanceOffset         Number of instances to skip
          /// @param vertexCountPerInstance Number of vertices to draw (elements) - for each instance
          /// @param vertexOffset           Number of vertices to skip in buffer (elements) - for each instance
          /// @remarks Useful to efficiently duplicate meshes with different params (color, position...) -- example: trees, leaves...
          inline void drawInstances(uint32_t instanceCount, uint32_t instanceOffset, 
                                    uint32_t vertexCountPerInstance, uint32_t vertexOffset = 0u) noexcept {
            this->_context->DrawInstanced((UINT)vertexCountPerInstance, (UINT)instanceCount, (UINT)vertexOffset, (UINT)instanceOffset);
          }
          /// @brief Draw multiple instances of active vertex buffer (indexed: active index buffer)
          /// @param instanceCount         Number of instances to draw
          /// @param instanceOffset        Number of instances to skip
          /// @param indexCountPerInstance Number of indexes to draw (elements) - for each instance
          /// @param indexOffset           Number of indexes to skip in buffer (elements) - for each instance
          /// @param vertexOffsetFromIndex Value added to each index (to read different vertices)
          /// @remarks Useful to efficiently duplicate meshes with different params (color, position...) -- example: trees, leaves...
          inline void drawInstancesIndexed(uint32_t instanceCount, uint32_t instanceOffset, uint32_t indexCountPerInstance, 
                                           uint32_t indexOffset = 0u, int32_t vertexOffsetFromIndex = 0u) noexcept {
            this->_context->DrawIndexedInstanced((UINT)indexCountPerInstance, (UINT)instanceCount, (UINT)indexOffset, 
                                                 (INT)vertexOffsetFromIndex, (UINT)instanceOffset);
          }
          

          // -- pipeline status operations - constant/uniform buffers --
          
          /// @brief Max slots (or array size from first slot) for constant/uniform buffers
          constexpr inline size_t maxUniformSlots() noexcept { return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; }
          
          /// @brief Bind constant/uniform buffer(s) to the vertex shader stage
          /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindVertexUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
            this->_context->VSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
          }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            /// @brief Bind constant/uniform buffer(s) to the tessellation-control/hull shader stage
            /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
            inline void bindTessCtrlUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
              this->_context->HSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
            }
            /// @brief Bind constant/uniform buffer(s) to the tessellation-evaluation/domain shader stage
            /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
            inline void bindTessEvalUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
              this->_context->DSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
            }
#         endif
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            /// @brief Bind constant/uniform buffer(s) to the geometry shader stage
            /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
            inline void bindGeometryUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
              this->_context->GSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
            }
#         endif
          /// @brief Bind constant/uniform buffer(s) to the fragment shader stage
          /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindFragmentUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
            this->_context->PSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
          }
          /// @brief Bind constant/uniform buffer(s) to the compute shader stage
          /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindComputeUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
            this->_context->CSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
          }
          
          inline void clearVertexUniforms() noexcept { ///< Reset all constant/uniform buffers in vertex shader stage
            ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
            this->_context->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
          }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            inline void clearTessCtrlUniforms() noexcept { ///< Reset all constant/uniform buffers in tessellation-control/hull shader stage
              ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
              this->_context->HSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
            }
            inline void clearTessEvalUniforms() noexcept { ///< Reset all constant/uniform buffers in tessellation-evaluation/domain shader stage
              ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
              this->_context->DSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
            }
#         endif
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            inline void clearGeometryUniforms() noexcept { ///< Reset all constant/uniform buffers in geometry shader stage
              ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
              this->_context->GSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
            }
#         endif
          inline void clearFragmentUniforms() noexcept { ///< Reset all constant/uniform buffers in fragment/pixel shader stage (standard)
            ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
            this->_context->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
          }
          inline void clearComputeUniforms() noexcept { ///< Reset all constant/uniform buffers in compute shader stage
            ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
            this->_context->CSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
          }
          
          
          // -- pipeline status operations - textures / shader resources --
          
          /// @brief Max slots (or array size from first slot) for shader resources / textures
          constexpr inline size_t maxResourceSlots() noexcept { return D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; }
          
          /// @brief Bind texture resource(s) to the vertex shader stage
          /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindVertexTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
            this->_context->VSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
          }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            /// @brief Bind texture resource(s) to the tessellation-control/hull shader stage
            /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
            inline void bindTesselControlTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
              this->_context->HSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
            }
            /// @brief Bind texture resource(s) to the tessellation-evaluation/domain shader stage
            /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
            inline void bindTesselEvalTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
              this->_context->DSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
            }
#         endif
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            /// @brief Bind texture resource(s) to the geometry shader stage
            /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
            inline void bindGeometryTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
              this->_context->GSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
            }
#         endif
          /// @brief Bind texture resource(s) (or texture(s)) to the fragment shader stage
          /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindFragmentTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
            this->_context->PSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
          }
          /// @brief Bind texture resource(s) (or texture(s)) to the compute shader stage
          /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindComputeTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
            this->_context->CSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
          }
          
          inline void clearVertexTextures() noexcept { ///< Reset all texture resource(s) in vertex shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            inline void clearTesselControlTextures() noexcept { ///< Reset all texture resource(s) in tessellation-control/hull shader stage
              ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
              this->_context->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
            }
            inline void clearTesselEvalTextures() noexcept { ///< Reset all texture resource(s) in tessellation-evaluation/domain shader stage
              ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
              this->_context->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
            }
#         endif
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            inline void clearGeometryTextures() noexcept { ///< Reset all texture resource(s) in geometry shader stage
              ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
              this->_context->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
            }
#         endif
          inline void clearFragmentTextures() noexcept { ///< Reset all texture resource(s) in fragment/pixel shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          inline void clearComputeTextures() noexcept { ///< Reset all texture resource(s) in compute shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          
          
          // -- pipeline status operations - renderer states --
          
          /// @brief Change device rasterizer mode (culling, clipping, depth-bias, wireframe...)
          /// @remarks - The rasterizer state will already be configured through a GraphicPipeline object. Try to limit dynamic changes.
          ///          - Setting rasterizer state dynamically is not possible in other APIs (Vulkan...). Avoid it for cross-API projects.
          ///          - If the rasterizer state has to be toggled regularly, keep the same RasterizerState instances to be more efficient.
          inline void setRasterizerState(const RasterizerState& state) noexcept {
            this->_context->RSSetState(state.get());
            this->_currentRasterizerState = state.get();
          }
          /// @brief Change output merger depth/stencil state (depth and/or stencil testing)
          /// @remarks The depth/stencil state will already be configured through a GraphicPipeline object. Try to limit dynamic changes.
          inline void setDepthStencilState(const DepthStencilState& state, uint32_t stencilRef = 1u) noexcept {
            this->_context->OMSetDepthStencilState(state.get(), (UINT)stencilRef);
            this->_currentDepthStencilState = (this->_attachedPipeline != nullptr
                                            && stencilRef == this->_attachedPipeline->stencilRef) ? state.get() : nullptr;
          }
          /// @brief Change output merger blend state with constant factors (color/alpha blending with render-target(s))
          /// @remarks - The blend state will already be configured through a GraphicPipeline object. Try to limit dynamic changes.
          ///          - The constant color is only used if the blend state uses BlendFactor::constantColor/constantInvColor
          ///            (defaults to factor 1.0 if 'constantColorRgba' is NULL).
          inline void setBlendState(const BlendState& state, const ColorChannel constantColorRgba[4] = nullptr) noexcept {
            this->_context->OMSetBlendState(state.get(), constantColorRgba, 0xFFFFFFFFu);
            this->_currentBlendState = nullptr; // color may differ -> always report as different
          }
          
          // ---
          
          /// @brief Max slots (or array size from first slot) for samplers/filters
          constexpr inline size_t maxSamplerStateSlots() noexcept { return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; }
          
          /// @brief Set array of sampler filters to the vertex shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setVertexSamplerStates(uint32_t firstSlotIndex, const SamplerStateArray::Resource* states, size_t length) noexcept {
            this->_context->VSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            /// @brief Set array of sampler filters to the tessellation-control/hull shader stage
            /// @remarks To remove some filters, use NULL value at their index
            inline void setTesselControlSamplerStates(uint32_t firstSlotIndex, const SamplerStateArray::Resource* states, size_t length) noexcept {
              this->_context->HSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
            }
            /// @brief Set array of sampler filters to the tessellation-evaluation/domain shader stage
            /// @remarks To remove some filters, use NULL value at their index
            inline void setTesselEvalSamplerStates(uint32_t firstSlotIndex, const SamplerStateArray::Resource* states, size_t length) noexcept {
              this->_context->DSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
            }
#         endif
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            /// @brief Set array of sampler filters to the geometry shader stage
            /// @remarks To remove some filters, use NULL value at their index
            inline void setGeometrySamplerStates(uint32_t firstSlotIndex, const SamplerStateArray::Resource* states, size_t length) noexcept {
              this->_context->GSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
            }
#         endif
          /// @brief Set array of sampler filters to the fragment/pixel shader stage (standard)
          /// @remarks To remove some filters, use NULL value at their index
          inline void setFragmentSamplerStates(uint32_t firstSlotIndex, const SamplerStateArray::Resource* states, size_t length) noexcept {
            this->_context->PSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          /// @brief Set array of sampler filters to the compute shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setComputeSamplerStates(uint32_t firstSlotIndex, const SamplerStateArray::Resource* states, size_t length) noexcept {
            this->_context->CSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          
          inline void clearVertexSamplerStates() noexcept { ///< Reset all sampler filters in vertex shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
#         ifndef __P_DISABLE_TESSELLATION_STAGE
            inline void clearTesselControlSamplerStates() noexcept { ///< Reset all sampler filters in tessellation-control/hull shader stage
              ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
              this->_context->HSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
            }
            inline void clearTesselEvalSamplerStates() noexcept { ///< Reset all sampler filters in tessellation-evaluation/domain shader stage
              ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
              this->_context->DSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
            }
#         endif
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            inline void clearGeometrySamplerStates() noexcept { ///< Reset all sampler filters in geometry shader stage
              ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
              this->_context->GSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
            }
#         endif
          inline void clearFragmentSamplerStates() noexcept { ///< Reset all sampler filters in fragment/pixel shader stage (standard)
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          inline void clearComputeSamplerStates() noexcept { ///< Reset all sampler filters in compute shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->CSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          
          
        private:
          void _destroy() noexcept;
          inline bool _areColorSpacesAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          inline bool _isFlipSwapAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          bool _isSampleCountSupported(DataFormat format, uint32_t sampleCount, uint32_t& outMaxQualityLevel) const noexcept;
          uint32_t _maxSampleCount(DXGI_FORMAT format) const noexcept;

          void _addRasterizerState(const RasterizerStateId& id, const RasterizerState& handle);
          void _addDepthStencilState(const DepthStencilStateId& id, const DepthStencilState& handle);
          void _addBlendState(const BlendStateId& id, const BlendState& handle);
          void _addBlendStatePerTarget(const BlendStatePerTargetId& id, const BlendState& handle);

          void _removeRasterizerState(const RasterizerStateId& id) noexcept;
          void _removeDepthStencilState(const DepthStencilStateId& id) noexcept;
          void _removeBlendState(const BlendStateId& id) noexcept;
          void _removeBlendStatePerTarget(const BlendStatePerTargetId& id) noexcept;

          bool _findRasterizerState(const RasterizerStateId& id, RasterizerState& out) const noexcept;
          bool _findDepthStencilState(const DepthStencilStateId& id, DepthStencilState& out) const noexcept;
          bool _findBlendState(const BlendStateId& id, BlendState& out) const noexcept;
          bool _findBlendStatePerTarget(const BlendStatePerTargetId& id, BlendState& out) const noexcept;

          friend class pandora::video::d3d11::SwapChain;
          friend class pandora::video::d3d11::GraphicsPipeline;
          
        private:
          DeviceHandle _device = nullptr;
          DeviceContext _context = nullptr;
          D3D_FEATURE_LEVEL _deviceLevel = D3D_FEATURE_LEVEL_11_0;
          void* _dxgiFactory = nullptr; // IDXGIFactory1*
          uint32_t _dxgiLevel = 1u;

          pandora::memory::LightVector<RasterizerStateCache> _rasterizerStateCache;
          pandora::memory::LightVector<DepthStencilStateCache> _depthStencilStateCache;
          pandora::memory::LightVector<BlendStateCache> _blendStateCache;
          pandora::memory::LightVector<BlendStatePerTargetCache> _blendStatePerTargetCache;

          std::shared_ptr<_DxPipelineStages> _attachedPipeline = nullptr;
          // storage for dynamic state changes
          ID3D11RasterizerState* _currentRasterizerState = nullptr;
          ID3D11DepthStencilState* _currentDepthStencilState = nullptr;
          ID3D11BlendState* _currentBlendState = nullptr;
          uint64_t _currentViewportScissorId = 0;
        };
      }
    }
  }
# include "./swap_chain.h" // includes D3D11
# include "./graphics_pipeline.h" // includes D3D11
#endif
