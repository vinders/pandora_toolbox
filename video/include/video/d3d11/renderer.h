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
# include <hardware/display_monitor.h>
# include "./api/types.h"      // includes D3D11
# include "./renderer_state.h" // includes D3D11
# include "./scissor.h"        // includes D3D11
# include "./viewport.h"       // includes D3D11
# include "./shader.h"         // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class SwapChain;

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
          inline uint32_t dxgiLevel() const noexcept { return this->_dxgiLevel; }  ///< Get available DXGI level on current system (1-6)
          inline D3D_FEATURE_LEVEL featureLevel() const noexcept { return this->_deviceLevel; } ///< Get available feature level on current device (11.0/11.1+)
          
          /// @brief Read device adapter VRAM size
          /// @returns Read success
          bool getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept;
          /// @brief Convert color/depth/component data format to native format (usable in input layout description) -- see "video/d3d11/shader.h".
          static constexpr inline DXGI_FORMAT toLayoutFormat(DataFormat format) noexcept { return _getDataFormatComponents(format); }
          /// @brief Convert standard sRGB(A) color to device RGB(A)
          /// @remarks Should be called to obtain colors to use with 'clearView(s)', 'setCleanActiveRenderTarget(s)', 'RendererStateFactory.create<...>Filter'
          static void toGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept;
          
          
          // -- feature support --
          
          /// @brief Verify if HDR functionalities are supported on current system
          /// @warning That doesn't mean the display supports it (call 'isMonitorHdrCapable').
          inline bool isHdrAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          /// @brief Verify if a display monitor can display HDR colors
          /// @remarks Should be called to know if a HDR/SDR pipeline should be created.
          bool isMonitorHdrCapable(const pandora::hardware::DisplayMonitor& target) const noexcept;
          
          /// @brief "Flip" swap mode supported (more efficient) -> internal usage
          inline bool isFlipSwapAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          /// @brief Screen tearing supported (variable refresh rate display)
          bool isTearingAvailable() const noexcept;
          
          
          // -- render target operations --
          
          /// @brief Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
          void setViewports(const Viewport* viewports, size_t numberViewports) noexcept;
          /// @brief Replace rasterizer viewport (3D -> 2D projection rectangle)
          inline void setViewport(const Viewport& viewport) noexcept { this->_context->RSSetViewports(1u, viewport.descriptor()); }

          /// @brief Set rasterizer scissor-test rectangle(s)
          void setScissorRectangles(const ScissorRectangle* rectangles, size_t numberRectangles) noexcept;
          /// @brief Set rasterizer scissor-test rectangle
          inline void setScissorRectangle(const ScissorRectangle& rectangle) noexcept { this->_context->RSSetScissorRects(1u, rectangle.descriptor()); }
          
          // ---

          /// @brief Max number of simultaneous render-target views (swap-chains, texture targets...)
          static constexpr inline size_t maxRenderTargets() noexcept { return D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; }
          
          /// @brief Clear render-targets content + depth buffer: reset to 'clearColorRgba' and to depth 1
          /// @remarks - Recommended before drawing frames that don't cover the whole buffer (unless keeping 'dirty' previous data is desired).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.toGammaCorrectColor).
          void clearViews(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          /// @brief Clear render-target content + depth buffer: reset to 'clearColorRgba' and to depth 1
          /// @remarks - Recommended before drawing frames that don't cover the whole buffer (unless keeping 'dirty' previous data is desired).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.toGammaCorrectColor).
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
          ///          - Color should be gamma correct (see Renderer.toGammaCorrectColor).
          void setCleanActiveRenderTargets(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer, 
                                           const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          /// @brief Bind/replace active render-target in Renderer (single target) + clear render-target/buffer
          /// @remarks - If the render-target is a new buffer (or resized), this is the recommended method (to reset it before using it).
          ///          - If the color value is NULL, the default color is used (black).
          ///          - Color should be gamma correct (see Renderer.toGammaCorrectColor).
          void setCleanActiveRenderTarget(RenderTargetView view, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4] = nullptr) noexcept;
          
          
          // -- primitive binding --
          
          /// @brief Max slots (or array size from first slot) for vertex buffers
          static constexpr inline size_t maxVertexBufferSlots() noexcept { return D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; }
          
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
          /// @brief Set vertex polygon topology for input stage
          /// @param topology  Native topology value (createTopology / createPatchTopology / D3D11_PRIMITIVE_TOPOLOGY)
          inline void setVertexTopology(VertexTopology topology) noexcept { this->_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology); }
          /// @brief Set vertex patch topology for input stage (for vertex/tessellation shaders)
          /// @param controlPoints  Number of patch control points: between 1 and 32 (other values will be clamped).
          void setVertexPatchTopology(uint32_t controlPoints) noexcept;
          
          
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
          
          
          // -- pipeline status operations - shaders --

          /// @brief Bind input-layout object to the input-assembler stage
          /// @param inputLayout  Native handle (ShaderInputLayout.handle()) or NULL to disable input.
          inline void bindInputLayout(InputLayoutHandle inputLayout) noexcept { this->_context->IASetInputLayout((ID3D11InputLayout*)inputLayout); }
          /// @brief Bind vertex shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind vertex shader.
          inline void bindVertexShader(Shader::Handle shader) noexcept { this->_context->VSSetShader((ID3D11VertexShader*)shader, nullptr, 0); }
          /// @brief Bind tessellation-control/hull shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind control/hull shader.
          inline void bindTessCtrlShader(Shader::Handle shader) noexcept { this->_context->HSSetShader((ID3D11HullShader*)shader, nullptr, 0); }
          /// @brief Bind tessellation-evaluation/domain shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind evaluation/domain shader.
          inline void bindTessEvalShader(Shader::Handle shader) noexcept { this->_context->DSSetShader((ID3D11DomainShader*)shader, nullptr, 0); }
          /// @brief Bind geometry shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind geometry shader.
          inline void bindGeometryShader(Shader::Handle shader) noexcept { this->_context->GSSetShader((ID3D11GeometryShader*)shader, nullptr, 0); }
          /// @brief Bind fragment/pixel shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind fragment/pixel shader.
          inline void bindFragmentShader(Shader::Handle shader) noexcept { this->_context->PSSetShader((ID3D11PixelShader*)shader, nullptr, 0); }
          /// @brief Bind compute shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to unbind compute shader.
          inline void bindComputeShader(Shader::Handle shader) noexcept { this->_context->CSSetShader((ID3D11ComputeShader*)shader, nullptr, 0); }
          
          
          // -- pipeline status operations - constant/uniform buffers --
          
          /// @brief Max slots (or array size from first slot) for constant/uniform buffers
          static constexpr inline size_t maxUniformSlots() noexcept { return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; }
          
          /// @brief Bind constant/uniform buffer(s) to the vertex shader stage
          /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindVertexUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
            this->_context->VSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
          }
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
          /// @brief Bind constant/uniform buffer(s) to the geometry shader stage
          /// @remarks To unbind some buffer indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindGeometryUniforms(uint32_t firstSlotIndex, const BufferHandle* handles, size_t length) noexcept {
            this->_context->GSSetConstantBuffers((UINT)firstSlotIndex, (UINT)length, handles);
          }
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
          inline void clearTessCtrlUniforms() noexcept { ///< Reset all constant/uniform buffers in tessellation-control/hull shader stage
            ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
            this->_context->HSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
          }
          inline void clearTessEvalUniforms() noexcept { ///< Reset all constant/uniform buffers in tessellation-evaluation/domain shader stage
            ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
            this->_context->DSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
          }
          inline void clearGeometryUniforms() noexcept { ///< Reset all constant/uniform buffers in geometry shader stage
            ID3D11Buffer* empty[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] { nullptr };
            this->_context->GSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &empty[0]);
          }
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
          static constexpr inline size_t maxResourceSlots() noexcept { return D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; }
          
          /// @brief Bind texture resource(s) to the vertex shader stage
          /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindVertexTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
            this->_context->VSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
          }
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
          /// @brief Bind texture resource(s) to the geometry shader stage
          /// @remarks To unbind some indices, use NULL value at their index ('handles' array must not be NULL)
          inline void bindGeometryTextures(uint32_t firstSlotIndex, const TextureView* handles, size_t length) noexcept {
            this->_context->GSSetShaderResources((UINT)firstSlotIndex, (UINT)length, handles);
          }
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
          inline void clearTesselControlTextures() noexcept { ///< Reset all texture resource(s) in tessellation-control/hull shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          inline void clearTesselEvalTextures() noexcept { ///< Reset all texture resource(s) in tessellation-evaluation/domain shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          inline void clearGeometryTextures() noexcept { ///< Reset all texture resource(s) in geometry shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          inline void clearFragmentTextures() noexcept { ///< Reset all texture resource(s) in fragment/pixel shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          inline void clearComputeTextures() noexcept { ///< Reset all texture resource(s) in compute shader stage
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] { nullptr };
            this->_context->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &empty[0]);
          }
          
          
          // -- pipeline status operations - renderer states --
          
          /// @brief Change output merger depth/stencil state (depth and/or stencil testing)
          inline void setDepthStencilState(const DepthStencilState& state, uint32_t stencilRef = 1u) noexcept {
            this->_context->OMSetDepthStencilState(state.get(), (UINT)stencilRef);
          }
          
          /// @brief Change device rasterizer mode (culling, clipping, depth-bias, wireframe...)
          /// @remarks - The rasterizer should be configured at least once at the beginning of the program.
          ///          - If the rasterizer state has to be toggled regularly, keep the same RasterizerState instances to be more efficient.
          inline void setRasterizerState(const RasterizerState& state) noexcept { this->_context->RSSetState(state.get()); }
          
          /// @brief Change output merger blend state (color/alpha blending with render-target(s)), or empty state to reset (BlendState{})
          /// @remarks If the blend state uses BlendFactor::constantColor/constantInvColor, a default white color is used.
          ///          To customize the constant color, use 'setBlendState(state, constantColorRgba)' instead.
          inline void setBlendState(const BlendState& state) noexcept { this->_context->OMSetBlendState(state.get(), nullptr, 0xFFFFFFFFu); }
          /// @brief Change output merger blend state with constant factors (color/alpha blending with render-target(s))
          /// @remarks The constant color is only used if the blend state uses BlendFactor::constantColor/constantInvColor.
          inline void setBlendState(const BlendState& state, const ColorChannel constantColorRgba[4]) noexcept {
            this->_context->OMSetBlendState(state.get(), constantColorRgba, 0xFFFFFFFFu);
          }
          
          // ---
          
          /// @brief Max slots (or array size from first slot) for sample filters
          static constexpr inline size_t maxFilterStateSlots() noexcept { return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; }
          
          /// @brief Set array of sampler filters to the vertex shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setVertexFilterStates(uint32_t firstSlotIndex, const FilterStateArray::State* states, size_t length) noexcept {
            this->_context->VSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          /// @brief Set array of sampler filters to the tessellation-control/hull shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setTesselControlFilterStates(uint32_t firstSlotIndex, const FilterStateArray::State* states, size_t length) noexcept {
            this->_context->HSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          /// @brief Set array of sampler filters to the tessellation-evaluation/domain shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setTesselEvalFilterStates(uint32_t firstSlotIndex, const FilterStateArray::State* states, size_t length) noexcept {
            this->_context->DSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          /// @brief Set array of sampler filters to the geometry shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setGeometryFilterStates(uint32_t firstSlotIndex, const FilterStateArray::State* states, size_t length) noexcept {
            this->_context->GSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          /// @brief Set array of sampler filters to the fragment/pixel shader stage (standard)
          /// @remarks To remove some filters, use NULL value at their index
          inline void setFragmentFilterStates(uint32_t firstSlotIndex, const FilterStateArray::State* states, size_t length) noexcept {
            this->_context->PSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          /// @brief Set array of sampler filters to the compute shader stage
          /// @remarks To remove some filters, use NULL value at their index
          inline void setComputeFilterStates(uint32_t firstSlotIndex, const FilterStateArray::State* states, size_t length) noexcept {
            this->_context->CSSetSamplers((UINT)firstSlotIndex, (UINT)length, states);
          }
          
          inline void clearVertexFilterStates() noexcept { ///< Reset all sampler filters in vertex shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          inline void clearTesselControlFilterStates() noexcept { ///< Reset all sampler filters in tessellation-control/hull shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->HSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          inline void clearTesselEvalFilterStates() noexcept { ///< Reset all sampler filters in tessellation-evaluation/domain shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->DSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          inline void clearGeometryFilterStates() noexcept { ///< Reset all sampler filters in geometry shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->GSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          inline void clearFragmentFilterStates() noexcept { ///< Reset all sampler filters in fragment/pixel shader stage (standard)
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          inline void clearComputeFilterStates() noexcept { ///< Reset all sampler filters in compute shader stage
            ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
            this->_context->CSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
          }
          
          
        private:
          void _destroy() noexcept;
          friend class pandora::video::d3d11::SwapChain;
          
        private:
          DeviceHandle _device = nullptr;
          DeviceContext _context = nullptr;
          D3D_FEATURE_LEVEL _deviceLevel = D3D_FEATURE_LEVEL_11_0;
          
          void* _dxgiFactory = nullptr; // IDXGIFactory1*
          uint32_t _dxgiLevel = 1u;
        };
      }
    }
  }
# include "./swap_chain.h" // includes D3D11
#endif
