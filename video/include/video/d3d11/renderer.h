/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <hardware/display_monitor.h>
# include "./_private/_swap_chain_config.h"
# include "../component_format.h"
# include "../render_options.h"
# include "../viewport.h"
# include "../window_handle.h"
# include "./renderer_state.h"
# include "./shader.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class SwapChain;
        class DepthStencilBuffer;
        
        /// @class Renderer
        /// @brief Direct3D rendering device and context (specific to adapter)
        /// @warning - Renderer is the main Direct3D resource, and should be kept alive while the program runs.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new Renderer must be created.
        ///          - Accessors to native D3D11 resources should be reserved for internal usage or for advanced features.
        /// @remarks - To render on display output, create a SwapChain + setActiveRenderTarget with it and a depth buffer.
        ///          - To render to a texture, create a TextureBuffer + setActiveRenderTarget with it and a depth buffer.
        ///          - Multiple active targets can be used simultaneously, to render the same image on multiple outputs/textures.
        ///          - Multi-window rendering (same adapter): alternate between different SwapChain instances on the same Renderer.
        ///          - Multi-window rendering (different adapters): use different Render instances with their own SwapChain.
        ///          - Split-screen rendering (same window): alternate between different Viewport instances on the same SwapChain.
        class Renderer final {
        public:
          using DeviceHandle = void*;    // ID3D11Device*
          using DeviceContext = void*;   // ID3D11DeviceContext*
          using Texture2dHandle = void*; // ID3D11Texture2D*
          using RenderTargetViewHandle = void*;// ID3D11RenderTargetView*
          using DepthStencilViewHandle = void*;// ID3D11DepthStencilView*
          using SwapChain = pandora::video::d3d11::SwapChain; // aliases for renderer templatization
          using ViewportBuilder = pandora::video::TopBasedViewportBuilder;
          using DepthStencilState = pandora::video::d3d11::DepthStencilState;
          using RasterizerState = pandora::video::d3d11::RasterizerState;
          using FilterStates = pandora::video::d3d11::FilterStates;
          
          /// @brief Direct3D feature level
          enum class DeviceLevel : uint32_t {
            direct3D_11_0 = 0, ///< 11.0
            direct3D_11_1 = 1  ///< 11.1/11.3/11.4
          };
          
          // ---
        
          /// @brief Create Direct3D rendering device and context
          /// @param monitor   Target display monitor for the renderer: used to determine the adapter to choose.
          /// @param minLevel  The system uses the highest available device level (based on 'maxLevel' and Cmake/cwork option _DEFAULT_D3D11_MAX_VERSION).
          ///                  If some feature level is not available, the level below is used (and so on).
          ///                  Argument 'minLevel' specifies the minimum level allowed (-> exception if higher than max available level).
          /// @param maxLevel  Maximum feature level: useful to target a specific feature level, lower than the highest supported level.
          /// @throws - out_of_range: if minLevel is too high.
          ///         - runtime_error: creation failure.
          Renderer(const pandora::hardware::DisplayMonitor& monitor, DeviceLevel minLevel = DeviceLevel::direct3D_11_0, 
                   DeviceLevel maxLevel = DeviceLevel::direct3D_11_1);
          /// @brief Destroy device and context resources
          ~Renderer() noexcept { _destroy(); }
          
          Renderer(const Renderer&) = delete;
          Renderer(Renderer&& rhs) noexcept;
          Renderer& operator=(const Renderer&) = delete;
          Renderer& operator=(Renderer&& rhs) noexcept;
          
          // -- accessors --
          
          inline DeviceHandle device() const noexcept { return this->_device; } ///< Get Direct3D rendering device (cast to 'ID3D11Device*')
          inline DeviceContext context() const noexcept { return this->_context; } ///< Get Direct3D device context (cast to 'ID3D11DeviceContext*')
          
          inline uint32_t dxgiLevel() const noexcept { return this->_dxgiLevel; } ///< Get available DXGI level on current system (1-6)
          inline DeviceLevel featureLevel() const noexcept { return this->_deviceLevel; } ///< Get available feature level on current device (11.0/11.1+)
          static size_t maxSimultaneousRenderViews() noexcept; ///< Max number of simultaneous render views (swap-chains, texture targets...)
          size_t activeRenderViews() noexcept { return this->_activeTargetCount; } ///< Current number of render views (swap-chains, texture targets...)
          
          /// @brief Read device adapter VRAM size
          /// @returns Read success
          bool getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept;
          /// @brief Convert portable color/depth/component format to DXGI_FORMAT (cast result to DXGI_FORMAT)
          /// @remarks Useful to fill input layout descriptions with portable format values (see "video/d3d11/shader.h").
          static int32_t toDxgiFormat(pandora::video::ComponentFormat format) noexcept;
          
          
          // -- feature support --
          
          /// @brief Verify if all HDR functionalities are supported
          /// @warning That doesn't mean the display supports it (call 'isMonitorHdrCapable').
          inline bool isHdrAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          /// @brief Verify if a display monitor can display HDR colors
          /// @remarks Should be called to know if a HDR/SDR pipeline should be created.
          bool isMonitorHdrCapable(const pandora::hardware::DisplayMonitor& target) const noexcept;
          
          /// @brief Screen tearing supported (variable refresh rate display)
          inline bool isTearingAvailable() const noexcept { return (this->_dxgiLevel >= 5u); }
          /// @brief "Flip" swap mode supported -> for internal usage
          inline bool isFlipSwapAvailable() const noexcept { return (this->_dxgiLevel >= 4u); }
          /// @brief Restricting to local displays supported (no screen sharing or printing)
          inline bool isLocalDisplayRestrictionAvailable() const noexcept { return (this->_dxgiLevel >= 3u); }
          
          
          // -- resource builder --
          
          /// @brief Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          DepthStencilState createDepthTestState(const DepthOperationGroup& frontFaceOp, const DepthOperationGroup& backFaceOp,
                                                 DepthComparison depthTest = DepthComparison::less, bool writeMaskAll = true);
          /// @brief Create stencil test state (disable depth test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          DepthStencilState createStencilTestState(const DepthStencilOperationGroup& frontFaceOp, const DepthStencilOperationGroup& backFaceOp, 
                                                   uint8_t readMask = 0xFF, uint8_t writeMask = 0xFF);
          /// @brief Create depth/stencil test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
          DepthStencilState createDepthStencilTestState(const DepthStencilOperationGroup& frontFaceOp, const DepthStencilOperationGroup& backFaceOp, 
                                                        DepthComparison depthTest = DepthComparison::less, bool depthWriteMaskAll = true, 
                                                        uint8_t stencilReadMask = 0xFF, uint8_t stencilWriteMask = 0xFF);
          
          /// @brief Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
          RasterizerState createRasterizerState(pandora::video::CullMode culling, bool isFrontClockwise, 
                                                const pandora::video::DepthBias& depth, bool scissorClipping = false);

          /// @brief Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateContainer  RAII container in which to insert/append new state item.
          /// @param minFilter          Filter to use for minification (downscaling).
          /// @param magFilter          Filter to use for magnification (upscaling).
          /// @param texAddressUVW      Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param borderColor        Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0].
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias            Offset from calculated mip-map level (added).
          /// @param index              Insert position in outStateContainer (-1 to append).
          /// @throws - out_of_range: full container or index out of range
          ///         - runtime_error: creation failure.
          void createFilter(FilterStates& outStateContainer, MinificationFilter minFilter, MagnificationFilter magFilter, 
                            const TextureAddressMode texAddressUVW[3], float lodMin = 0.0, float lodMax = 0.0, 
                            float lodBias = 0.0, const float borderColor[4] = _blackFilterBorder(), int32_t index = -1);
          /// @brief Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateContainer  RAII container in which to insert/append new state item.
          /// @param minFilter          Filter to use for minification (downscaling).
          /// @param magFilter          Filter to use for magnification (upscaling).
          /// @param texAddressUVW      Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param compare            Depth comparison mode with existing reference pixels.
          /// @param borderColor        Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0].
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias            Offset from calculated mip-map level (added).
          /// @param index              Insert position in outStateContainer (-1 to append).
          /// @throws - out_of_range: full container or index out of range.
          ///         - runtime_error: creation failure.
          void createComparedFilter(FilterStates& outStateContainer, MinificationFilter minFilter, 
                                    MagnificationFilter magFilter, const TextureAddressMode texAddressUVW[3], 
                                    DepthComparison compare, float lodMin = 0.0, float lodMax = 0.0, float lodBias = 0.0,
                                    const float borderColor[4] = _blackFilterBorder(), int32_t index = -1);
          
          /// @brief Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateContainer  RAII container in which to insert/append new state item.
          /// @param maxAnisotropy      Clamping anisotropy value - range: [1;maxAnisotropy()].
          /// @param texAddressUVW      Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param borderColor        Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0].
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias            Offset from calculated mip-map level (added).
          /// @param index              Insert position in outStateContainer (-1 to append).
          /// @throws - out_of_range: full container or index out of range.
          ///         - runtime_error: creation failure.
          void createAnisotropicFilter(FilterStates& outStateContainer, uint32_t maxAnisotropy, 
                                       const TextureAddressMode texAddressUVW[3], float lodMin = 0.0, float lodMax = 0.0, 
                                       float lodBias = 0.0, const float borderColor[4] = _blackFilterBorder(), int32_t index = -1);
          /// @brief Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
          /// @param outStateCont  RAII container in which to insert/append new state item.
          /// @param maxAniso      Clamping anisotropy value - range: [1;maxAnisotropy()].
          /// @param txAddrUVW     Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          /// @param compare       Depth comparison mode with existing reference pixels.
          /// @param borderColor   Border color for clamping (color[4]: R/G/B/A) - value range == [0.0;1.0].
          /// @param lodMin             Minimum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodMax             Maximum level of detail (mip-map level): 0==highest / custom value: lowest to highest == [1;D3D11_FLOAT32_MAX].
          /// @param lodBias       Offset from calculated mip-map level (added).
          /// @param index         Insert position in outStateContainer (-1 to append).
          /// @throws - out_of_range: full container or index out of range.
          ///         - runtime_error: creation failure.
          void createComparedAnisotropicFilter(FilterStates& outStateCont, uint32_t maxAniso, const TextureAddressMode txAddrUVW[3], 
                                               DepthComparison compare, float lodMin = 0.0, float lodMax = 0.0, float lodBias = 0.0, 
                                               const float borderColor[4] = _blackFilterBorder(), int32_t index = -1);
          
          uint32_t maxAnisotropy() const noexcept; ///< Max anisotropy value (usually 16)
          size_t maxFilterStates() const noexcept; ///< Max array size for sample filters
          
          
          // -- pipeline status operations --

          /// @brief Bind input-layout object to the input-assembler stage
          /// @param inputLayout  Native handle (ShaderInputLayout.handle()) or NULL to disable input.
          void bindInputLayout(ShaderInputLayout::Handle inputLayout) noexcept;
          /// @brief Bind vertex shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to remove vertex shader.
          void bindVertexShader(Shader::Handle shader) noexcept;
          /// @brief Bind tessellation-control/hull shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to remove control/hull shader.
          void bindTesselControlShader(Shader::Handle shader) noexcept;
          /// @brief Bind tessellation-evaluation/domain shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to remove evaluation/domain shader.
          void bindTesselEvalShader(Shader::Handle shader) noexcept;
          /// @brief Bind geometry shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to remove geometry shader.
          void bindGeometryShader(Shader::Handle shader) noexcept;
          /// @brief Bind fragment/pixel shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to remove fragment/pixel shader.
          void bindFragmentShader(Shader::Handle shader) noexcept;
          /// @brief Bind compute shader stage to the device
          /// @param shader  Native handle (Shader.handle()) or NULL to remove compute shader.
          void bindComputeShader(Shader::Handle shader) noexcept;
          
          /// @brief Change output merger depth/stencil state (depth and/or stencil testing)
          void setDepthStencilState(const DepthStencilState& state, uint32_t stencilRef = 1u) noexcept;
          /// @brief Change device rasterizer mode (culling, clipping, depth-bias, wireframe...)
          /// @remarks - The rasterizer should be configured at least once at the beginning of the program.
          ///          - If the rasterizer state has to be toggled regularly, keep the same RasterizerState instances to be more efficient.
          void setRasterizerState(const RasterizerState& state) noexcept;
          
          /// @brief Set array of sampler filters to the vertex shader stage
          /// @remarks To remove some filters, use NULL value at their index
          void setVertexFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept;
          /// @brief Set array of sampler filters to the tessellation-control/hull shader stage
          /// @remarks To remove some filters, use NULL value at their index
          void setTesselControlFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept;
          /// @brief Set array of sampler filters to the tessellation-evaluation/domain shader stage
          /// @remarks To remove some filters, use NULL value at their index
          void setTesselEvalFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept;
          /// @brief Set array of sampler filters to the geometry shader stage
          /// @remarks To remove some filters, use NULL value at their index
          void setGeometryFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept;
          /// @brief Set array of sampler filters to the fragment/pixel shader stage (standard)
          /// @remarks To remove some filters, use NULL value at their index
          void setFragmentFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept;
          /// @brief Set array of sampler filters to the compute shader stage
          /// @remarks To remove some filters, use NULL value at their index
          void setComputeFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept;
          
          /// @brief Reset all sampler filters in vertex shader stage
          void clearVertexFilterStates() noexcept;
          /// @brief Reset all sampler filters in tessellation-control/hull shader stage
          void clearTesselControlFilterStates() noexcept;
          /// @brief Reset all sampler filters in tessellation-evaluation/domain shader stage
          void clearTesselEvalFilterStates() noexcept;
          /// @brief Reset all sampler filters in geometry shader stage
          void clearGeometryFilterStates() noexcept;
          /// @brief Reset all sampler filters in fragment/pixel shader stage (standard)
          void clearFragmentFilterStates() noexcept;
          /// @brief Reset all sampler filters in compute shader stage
          void clearComputeFilterStates() noexcept;
          
          
          // -- render target operations --
          
          /// @brief Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
          void setViewports(const pandora::video::Viewport* viewports, size_t numberViewports) noexcept;
          /// @brief Replace rasterizer viewport (3D -> 2D projection rectangle)
          void setViewport(const pandora::video::Viewport& viewport) noexcept;
          
          /// @brief Clear render-targets content + depth buffer: reset to 'clearColorRgba' and to depth 1
          /// @remarks Recommended before drawing frames that don't cover the whole buffer (unless keeping 'dirty' previous data is desired).
          void clearViews(RenderTargetViewHandle* views, size_t numberViews, DepthStencilViewHandle depthBuffer, 
                                  const pandora::video::ComponentVector128& clearColorRgba) noexcept;
          /// @brief Clear render-target content + depth buffer: reset to 'clearColorRgba' and to depth 1
          /// @remarks Recommended before drawing frames that don't cover the whole buffer (unless keeping 'dirty' previous data is desired).
          void clearView(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer, 
                                 const pandora::video::ComponentVector128& clearColorRgba) noexcept;
          
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
          void setActiveRenderTargets(RenderTargetViewHandle* views, size_t numberViews, 
                                      DepthStencilViewHandle depthBuffer = nullptr) noexcept;
          /// @brief Bind/replace active render-target in Renderer (single target)
          /// @remarks - This call allows draw/render operations to fill SwapChain back-buffers and/or TextureBuffer instances.
          ///          - It should be called before the first iteration of the program loop.
          ///          - It should be called everytime the rendering needs to target a different resource (ex: render to texture, then swap-chain).
          ///          - It should be called again after deleting/resizing any SwapChain or TextureBuffer.
          ///          - Calling it with a NULL view disables active render-targets.
          inline void setActiveRenderTarget(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer = nullptr) noexcept {
            setActiveRenderTargets(&view, size_t{1u}, depthBuffer);
          }
          
          /// @brief Bind/replace active render-target(s) in Renderer (multi-target) + clear render-targets/buffer
          /// @remarks If the render-targets contain new buffers (or resized), this is the recommended method (to reset them before using them).
          void setActiveRenderTargets(RenderTargetViewHandle* views, size_t numberViews, DepthStencilViewHandle depthBuffer, 
                                      const pandora::video::ComponentVector128& clearColorRgba) noexcept;
          /// @brief Bind/replace active render-target in Renderer (single target) + clear render-target/buffer
          /// @remarks If the render-target is a new buffer (or resized), this is the recommended method (to reset it before using it).
          void setActiveRenderTarget(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer, 
                                     const pandora::video::ComponentVector128& clearColorRgba) noexcept;
          
        private:
          void _destroy() noexcept;
          void _refreshDxgiFactory(); // throws
          static inline const float* _blackFilterBorder() noexcept { static float color[] = { 0.,0.,0.,1. }; return color; }
          void* _createSwapChain(const _SwapChainConfig& config, pandora::video::WindowHandle window,
                                 uint32_t rateNumerator, uint32_t rateDenominator, 
                                 DeviceLevel& outSwapChainLevel); // throws
          friend class pandora::video::d3d11::SwapChain;
          
        private:
          void* _dxgiFactory = nullptr;     // IDXGIFactory1*
          DeviceHandle _device = nullptr;   // ID3D11Device*
          DeviceContext _context = nullptr; // ID3D11DeviceContext*
          size_t _activeTargetCount = 0;
          DeviceLevel _deviceLevel = DeviceLevel::direct3D_11_1;
          uint32_t _dxgiLevel = 1;
        };
      }
    }
  }

# include "./depth_stencil_buffer.h"
# include "./swap_chain.h"
#endif
