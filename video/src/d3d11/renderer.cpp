/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implements: renderer.h / renderer_state_factory.h / swap_chain.h /
            static_buffer.h / dynamic_buffer.h / depth_stencil_buffer.h /
            shader.h / _d3d_resource.h / camera.h
            (regrouped to improve compiler optimizations + greatly reduce executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <cstring>
# include <string>
# include <stdexcept>

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# define NOGDICAPMASKS
# define NOVIRTUALKEYCODES
# define NOWINSTYLES
# define NOSYSMETRICS
# define NOMENUS
# define NOICONS
# define NOKEYSTATES
# define NOSHOWWINDOW
# define NOSCROLL
# define NOSOUND
# define NOKANJI
# define NOHELP
# define NOPROFILER
# define NODEFERWINDOWPOS
# define NOTAPE

# include <system/api/windows_api.h>
# include "video/d3d11/api/d3d_11.h"
# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
#   include <dxgi1_6.h>
# elif defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#   include <dxgi1_5.h>
# else
#   include <dxgi1_2.h>
# endif
# ifdef _DEBUG
#   include <initguid.h>
#   include <dxgidebug.h>
# endif
# include <DirectXMath.h>
# include <D3DCompiler.h>

# include "video/d3d11/renderer.h"
# include "video/d3d11/swap_chain.h"
# include "video/d3d11/renderer_state_factory.h"
# include "video/d3d11/_private/_d3d_resource.h"

# include "video/d3d11/shader.h"
# include "video/d3d11/depth_stencil_buffer.h"
# include "video/d3d11/dynamic_buffer.h"
# include "video/d3d11/static_buffer.h"
# include "video/d3d11/camera.h"

  using namespace pandora::video::d3d11;
  using namespace pandora::video;


// -----------------------------------------------------------------------------
// renderer.h
// -----------------------------------------------------------------------------

// -- display adapter detection -- ---------------------------------------------

  // Verify if a hardware adapter outputs on a monitor
  static bool __isHardwareAdapterForMonitor(IDXGIAdapter1* adapter, const std::wstring& targetMonitorId) {
    D3dResource<IDXGIOutput> output;
    DXGI_OUTPUT_DESC monitorDescription;
    for (UINT index = 0; adapter->EnumOutputs(index, output.address()) == S_OK && output; ++index) {
      if (output->GetDesc(&monitorDescription) == S_OK && targetMonitorId == monitorDescription.DeviceName)
        return true; // hardware adapter uses the target output -> exit loop
    }
    return false;
  }

  // Find primary hardware adapter
  static D3dResource<IDXGIAdapter1> __getHardwareAdapter(IDXGIFactory1* factory, const pandora::hardware::DisplayMonitor& target) {
    // try to find adapter associated with monitor (if not empty monitor)
    std::wstring targetDeviceString;
    if (!target.attributes().id.empty())
      targetDeviceString = target.adapterName();

    D3dResource<IDXGIAdapter1> adapter;
    D3dResource<IDXGIAdapter1> preferredAdapter;
#   if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
      auto factoryV6 = D3dResource<IDXGIFactory6>::tryFromInterface(factory);
      if (factoryV6) {
        for (UINT i = 0; SUCCEEDED(factoryV6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, 
                                                                        IID_PPV_ARGS(adapter.address()))); ++i) {
          DXGI_ADAPTER_DESC1 description;
          if (SUCCEEDED(adapter->GetDesc1(&description)) && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) { // not "basic render" driver
            if (target.attributes().id.empty()  // no target provided -> use first result
            || targetDeviceString == description.Description          // adapter name available -> verify it
            || __isHardwareAdapterForMonitor(adapter.get(), target.attributes().id)) // no name or invalid -> verify if monitor connected to adapter
              break; // hardware adapter found -> exit loop

            if (!preferredAdapter) // store the highest perf. adapter, in case no result is found
              preferredAdapter = std::move(adapter);
          }
          adapter.destroy(); // do not return current value if not appropriate
        }
        if (adapter)
          return adapter;
      }
#   endif

    for (UINT i = 0; SUCCEEDED(factory->EnumAdapters1(i, adapter.address())); ++i) {
      DXGI_ADAPTER_DESC1 description;
      if (SUCCEEDED(adapter->GetDesc1(&description)) && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) { // not "basic render" driver
        if (target.attributes().id.empty()  // no target provided -> use first result
        || targetDeviceString == description.Description          // adapter name available -> verify it
        || __isHardwareAdapterForMonitor(adapter.get(), target.attributes().id)) // no name or invalid -> verify if monitor connected to adapter
          break; // hardware adapter found -> exit loop
        
        if (!preferredAdapter) // store first adapter, in case no result is found
          preferredAdapter = std::move(adapter);
      }
      adapter.destroy(); // do not return current value if not appropriate
    }
    
    if (!adapter)
      adapter = std::move(preferredAdapter);
#   if defined(_CPP_REVISION) && _CPP_REVISION == 14
      return std::move(adapter);
#   else
      return adapter;
#   endif
  }


// -- device/context creation -- -----------------------------------------------
  
# ifdef _DEBUG
    // Verify availability of debugging layers
    static inline bool __isDebugSdkAvailable() noexcept {
      HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_NULL, nullptr, D3D11_CREATE_DEVICE_DEBUG,
                                     nullptr, 0, D3D11_SDK_VERSION, nullptr, nullptr, nullptr);
      return SUCCEEDED(hr);
    }
# endif

  // Get max DXGI factory level
  static uint32_t __getDxgiFactoryLevel(IDXGIFactory1* dxgiFactory) {
#   if (defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2) || (defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#     if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
        if (D3dResource<IDXGIFactory6>::tryFromInterface(dxgiFactory)) return 6u;
#     endif
      if (D3dResource<IDXGIFactory5>::tryFromInterface(dxgiFactory)) return 5u;
      if (D3dResource<IDXGIFactory4>::tryFromInterface(dxgiFactory)) return 4u;
      if (D3dResource<IDXGIFactory3>::tryFromInterface(dxgiFactory)) return 3u;
#   endif
    if (D3dResource<IDXGIFactory2>::tryFromInterface(dxgiFactory)) return 2u;
    return 1u;
  }

  // Create DXGI factory + configure DXGI debugger
  static void __createDxgiFactory(D3dResource<IDXGIFactory1>& outDxgiFactory) { // throws
    IDXGIFactory1* dxgiFactory = nullptr;
#   if defined(_DEBUG) && defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      IDXGIInfoQueue* dxgiInfoQueue;
      if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))) ) {
        if (SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory) )) && dxgiFactory != nullptr) {
          dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
          dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
          
          DXGI_INFO_QUEUE_MESSAGE_ID denied[]{ 80 }; // IDXGISwapChain::GetContainingOutput: swapchain adapter doesn't control output with window
          DXGI_INFO_QUEUE_FILTER filter = {};
          filter.DenyList.NumIDs = _countof(denied);
          filter.DenyList.pIDList = denied;
          dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
        dxgiInfoQueue->Release();
      }
      if (dxgiFactory == nullptr)
#   endif
    {
      auto result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
      if (FAILED(result) || dxgiFactory == nullptr)
        throwError(result, "Renderer: DXGI creation failed");
    }
    outDxgiFactory = dxgiFactory;
  }
  
  // Get current DXGI factory of device
  static inline void __getCurrentDxgiFactory(ID3D11Device* device, D3dResource<IDXGIFactory1>& outDxgiFactory) { // throws
    auto dxgiDevice = D3dResource<IDXGIDevice>::fromInterface(device, "Renderer: DXGI access failed");
    D3dResource<IDXGIAdapter> adapter;
    auto result = dxgiDevice->GetAdapter(adapter.address());
    if (FAILED(result) || !adapter.hasValue())
      throwError(result, "Renderer: adapter access failed");
    
    outDxgiFactory = D3dResource<IDXGIFactory1>::fromChild(adapter.get(), "Renderer: DXGI factory failure");
  }
  
  // If output information on DXGI factory is stale, try to create a new one
  static __forceinline void __refreshDxgiFactory(void* dxgiFactory) { // throws
    if (!((IDXGIFactory1*)dxgiFactory)->IsCurrent()) {
      D3dResource<IDXGIFactory1> newFactory;
      __createDxgiFactory(newFactory);
      dxgiFactory = (void*)newFactory.extract();
    }
  }
  
# ifdef _DEBUG
    // Configure device debugger
    static inline void __configureDeviceDebug(ID3D11Device* device) {
      auto d3dDebug = D3dResource<ID3D11Debug>::tryFromInterface(device);
      if (d3dDebug) {
        auto d3dInfoQueue = D3dResource<ID3D11InfoQueue>::tryFromInterface(d3dDebug.get());
        if (d3dInfoQueue) {
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
          
          D3D11_MESSAGE_ID disabled[] = { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS };
          D3D11_INFO_QUEUE_FILTER filter = {};
          filter.DenyList.NumIDs = _countof(disabled);
          filter.DenyList.pIDList = disabled;
          d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
      }
    }
# endif

  // ---

  // Create Direct3D rendering device and context
  Renderer::Renderer(const pandora::hardware::DisplayMonitor& monitor, const D3D_FEATURE_LEVEL* featureLevels, size_t featureLevelCount) { // throws
    UINT runtimeLayers = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#   ifdef _DEBUG
      if (__isDebugSdkAvailable())
        runtimeLayers |= D3D11_CREATE_DEVICE_DEBUG;
#   endif

#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      D3D_FEATURE_LEVEL defaultLevels[] { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
      if (featureLevels == nullptr) {
        featureLevels = &defaultLevels[0];
        featureLevelCount = size_t{ 2u };
      }
      else if (featureLevelCount == 0)
        throw std::out_of_range("Renderer: no feat level");
      else if ((int32_t)*featureLevels > (int32_t)D3D_FEATURE_LEVEL_11_1)
        throw std::out_of_range("Renderer: feat level above max supported");
#   else
      D3D_FEATURE_LEVEL defaultLevel[] { D3D_FEATURE_LEVEL_11_0 };
      if (featureLevels == nullptr) {
        featureLevels = &defaultLevel[0];
        featureLevelCount = size_t{ 1u };
      }
      else {
        while (featureLevelCount && (int32_t)*featureLevels > (int32_t)D3D_FEATURE_LEVEL_11_0) {
          ++featureLevels;
          --featureLevelCount;
        }
        if (featureLevelCount == 0)
          throw std::out_of_range("Renderer: feat level empty or above max supported");
      }
#   endif

    // create DXGI factory + get primary adapter (if not found, default adapter will be used instead)
    D3dResource<IDXGIFactory1> dxgiFactory;
    __createDxgiFactory(dxgiFactory); // throws
    D3dResource<IDXGIAdapter1> adapter = __getHardwareAdapter(dxgiFactory.get(), monitor); // may be NULL (not found)
    bool isDefaultAdapter = !adapter.hasValue();

    // create rendering device + context
    auto result = D3D11CreateDevice(adapter.get(), (adapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, 
                                    nullptr, runtimeLayers, &featureLevels[0], (UINT)featureLevelCount, D3D11_SDK_VERSION, 
                                    &(this->_device), &(this->_deviceLevel), &(this->_context));
    if (FAILED(result) || this->_device == nullptr || this->_context == nullptr)
      throwError(result, "Renderer: device/context creation failed"); // throws
    if (isDefaultAdapter && !dxgiFactory->IsCurrent()) // if adapter not provided, system may generate another factory
      __getCurrentDxgiFactory(this->_device, dxgiFactory); // throws
    
    // feature level detection
    this->_dxgiLevel = __getDxgiFactoryLevel(dxgiFactory.get());
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (this->_dxgiLevel < 2u) {
        this->_deviceLevel = D3D_FEATURE_LEVEL_11_0;
        this->_dxgiLevel = 1u;
      }
#   else
      this->_deviceLevel = D3D_FEATURE_LEVEL_11_0;
      this->_dxgiLevel = 1u;
#   endif
    this->_dxgiFactory = (void*)dxgiFactory.extract();

#   ifdef _DEBUG
      __configureDeviceDebug(this->_device);
#   endif
  }


// -- device/context destruction/move -- ---------------------------------------

  // Destroy device and context resources
  void Renderer::_destroy() noexcept {
    try {
      // release device context
      if (this->_context) {
        this->_context->Flush();
        this->_context->Release();
      }
      // release device
      if (this->_device)
        this->_device->Release();
      if (this->_dxgiFactory)
        ((IDXGIFactory1*)this->_dxgiFactory)->Release();
    }
    catch (...) {}
  }

  Renderer::Renderer(Renderer&& rhs) noexcept
    : _dxgiFactory(rhs._dxgiFactory),
      _device(rhs._device),
      _context(rhs._context),
      _deviceLevel(rhs._deviceLevel),
      _dxgiLevel(rhs._dxgiLevel) {
    rhs._dxgiFactory = nullptr;
    rhs._device = nullptr;
    rhs._context = nullptr;
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    _destroy();
    this->_dxgiFactory = rhs._dxgiFactory;
    this->_device = rhs._device;
    this->_context = rhs._context;
    this->_deviceLevel = rhs._deviceLevel;
    this->_dxgiLevel = rhs._dxgiLevel;
    rhs._dxgiFactory = nullptr;
    rhs._device = nullptr;
    rhs._context = nullptr;
    return *this;
  }


// -- accessors -- -------------------------------------------------------------
  
  // Read device adapter VRAM size
  bool Renderer::getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept {
    bool isSuccess = false;
    try {
      auto dxgiDevice = D3dResource<IDXGIDevice>::tryFromInterface(this->_device);
      if (dxgiDevice) {
        D3dResource<IDXGIAdapter> adapter;
        if (SUCCEEDED(dxgiDevice->GetAdapter(adapter.address())) && adapter) {
          
          DXGI_ADAPTER_DESC adapterInfo;
          if (adapter->GetDesc(&adapterInfo) == S_OK) {
            outDedicatedRam = (size_t)adapterInfo.DedicatedVideoMemory;
            outSharedRam = (size_t)adapterInfo.SharedSystemMemory;
            isSuccess = true;
          }
        }
      }
    }
    catch (...) {}
    return isSuccess;
  }


// -- feature support -- -------------------------------------------------------

  // Verify if a display monitor can display HDR colors
# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
    bool Renderer::isMonitorHdrCapable(const pandora::hardware::DisplayMonitor& target) const noexcept {
      try {
        auto dxgiDevice = D3dResource<IDXGIDevice>::tryFromInterface(this->_device);
        if (dxgiDevice) {
          D3dResource<IDXGIAdapter> adapter;
          if (SUCCEEDED(dxgiDevice->GetAdapter(adapter.address())) && adapter) {

            D3dResource<IDXGIOutput> output;
            DXGI_OUTPUT_DESC1 monitorDescription;
            for (UINT index = 0; adapter->EnumOutputs(index, output.address()) == S_OK; ++index) {
              auto outputV6 = D3dResource<IDXGIOutput6>::tryFromInterface(output.get());
              if (outputV6) {
                // if monitor found, verify color space
                if (outputV6->GetDesc1(&monitorDescription) == S_OK && target.attributes().id == monitorDescription.DeviceName)
                  return (monitorDescription.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
                         || monitorDescription.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709);
              }
            }
          }
        }
      } catch (...) {}
      return false; // not found
    }
# else
    bool Renderer::isMonitorHdrCapable(const pandora::hardware::DisplayMonitor&) const noexcept { return false; }
# endif
 

// -- render target operations -------------------------------------------------

  // Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
  void Renderer::setViewports(const Viewport* viewports, size_t numberViewports) noexcept {
    if (viewports != nullptr) {
      D3D11_VIEWPORT values[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
      if (numberViewports > D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
        numberViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
      ZeroMemory(&values[0], numberViewports*sizeof(D3D11_VIEWPORT));

      D3D11_VIEWPORT* out = &values[0];
      const Viewport* end = &viewports[numberViewports - 1u];
      for (const Viewport* it = &viewports[0]; it <= end; ++it, ++out) {
        out->TopLeftX = (float)it->x();
        out->TopLeftY = (float)it->y();
        out->Width = (float)it->width();
        out->Height = (float)it->height();
        out->MinDepth = (float)it->nearClipping();
        out->MaxDepth = (float)it->farClipping();
      }
      this->_context->RSSetViewports((UINT)numberViewports, &values[0]);
    }
  }
  // Replace rasterizer viewport (3D -> 2D projection rectangle)
  void Renderer::setViewport(const Viewport& viewport) noexcept {
    D3D11_VIEWPORT data{};
    data.TopLeftX = (float)viewport.x();
    data.TopLeftY = (float)viewport.y();
    data.Width = (float)viewport.width();
    data.Height = (float)viewport.height();
    data.MinDepth = (float)viewport.nearClipping();
    data.MaxDepth = (float)viewport.farClipping();
    this->_context->RSSetViewports(1u, &data);
  }
  
  // ---

  static FLOAT __defaultBlackColor[] = { 0.f,0.f,0.f,1.f };
  
  // Clear render-targets + depth buffer: reset to 'clearColorRgba' and to depth 1
  void Renderer::clearViews(RenderTargetViewHandle* views, size_t numberViews, DepthStencilViewHandle depthBuffer, 
                            const FLOAT clearColorRgba[4]) noexcept {
    auto it = views;
    for (size_t i = 0; i < numberViews; ++i, ++it) {
      if (*it != nullptr)
        this->_context->ClearRenderTargetView(*it, (clearColorRgba != nullptr) ? &clearColorRgba[0] : __defaultBlackColor);
    }
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    this->_context->Flush();
  }
  // Clear render-target content + depth buffer: reset to 'clearColorRgba' and to depth 1
  void Renderer::clearView(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer, const FLOAT clearColorRgba[4]) noexcept {
    if (view != nullptr)
      this->_context->ClearRenderTargetView(view, (clearColorRgba != nullptr) ? &clearColorRgba[0] : __defaultBlackColor);
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    this->_context->Flush();
  }

  // Bind/replace active render-target(s) in Renderer (multi-target)
  void Renderer::setActiveRenderTargets(RenderTargetViewHandle* views, size_t numberViews, 
                                        DepthStencilViewHandle depthBuffer) noexcept {
    if (views != nullptr && numberViews > 0) { // set active view(s)
      this->_context->OMSetRenderTargets((UINT)numberViews, views, depthBuffer);
      this->_context->Flush();
      this->_activeTargetCount = (*views || numberViews > size_t{1u}) ? numberViews : size_t{0u};
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      this->_context->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
      this->_context->Flush();
      this->_activeTargetCount = size_t{0u};
    }
  }
  
  // Bind/replace active render-target(s) in Renderer (multi-target) + clear render-target/buffer
  void Renderer::setCleanActiveRenderTargets(RenderTargetViewHandle* views, size_t numberViews, 
                                        DepthStencilViewHandle depthBuffer, const FLOAT clearColorRgba[4]) noexcept {
    if (views != nullptr && numberViews > 0) { // set active view(s)
      auto it = views;
      for (size_t i = 0; i < numberViews; ++i, ++it) {
        if (*it != nullptr)
          this->_context->ClearRenderTargetView(*it, (clearColorRgba != nullptr) ? &clearColorRgba[0] : __defaultBlackColor);
      }
      if (depthBuffer != nullptr)
        this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
      this->_context->OMSetRenderTargets((UINT)numberViews, views, depthBuffer);
      this->_activeTargetCount = (*views || numberViews > size_t{1u}) ? numberViews : size_t{0u};
      this->_context->Flush();
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      this->_context->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
      this->_context->Flush();
      this->_activeTargetCount = size_t{0u};
    }
  }

  // Bind/replace active render-target in Renderer (single target) + clear render-target/buffer
  void Renderer::setCleanActiveRenderTarget(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer, 
                                       const FLOAT clearColorRgba[4]) noexcept {
    if (view != nullptr) {
      this->_activeTargetCount = size_t{1u};
      this->_context->ClearRenderTargetView(view, (clearColorRgba != nullptr) ? &clearColorRgba[0] : __defaultBlackColor);
    }
    else { this->_activeTargetCount = size_t{0u}; }
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            
    this->_context->OMSetRenderTargets((UINT)1u, &view, depthBuffer);
    this->_context->Flush();
  }


// -- primitive binding -- -----------------------------------------------------

  // Create native topology flag - basic topologies
  Renderer::TopologyFlag Renderer::createTopology(VertexTopology type, bool useAdjacency) noexcept {
    D3D11_PRIMITIVE_TOPOLOGY flag;
    switch (type) {
      case VertexTopology::points:
        flag = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break;
      case VertexTopology::lines:
        flag = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ : D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break;
      case VertexTopology::lineStrips:
        flag = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ : D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
      case VertexTopology::triangles:
        flag = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
      case VertexTopology::triangleStrip:
        flag = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
      default: flag = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED; break;
    }
    return (Renderer::TopologyFlag)flag;
  }
  // Create native topology flag - patch topologies
  Renderer::TopologyFlag Renderer::createPatchTopology(uint32_t controlPoints) noexcept {
    if (controlPoints != 0u) {
      --controlPoints;
      if (controlPoints >= 32u)
        controlPoints = 31u;
    }
    int32_t topologyValue = (int32_t)D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (int32_t)controlPoints;
    return (Renderer::TopologyFlag)topologyValue;
  }


// -----------------------------------------------------------------------------
// renderer_state_factory.h
// -----------------------------------------------------------------------------

// -- renderer state factory - depth/stencil -- --------------------------------

  // Convert portable depth/stencil comparison enum to Direct3D comparison enum
  static D3D11_COMPARISON_FUNC __toDepthComparison(DepthComparison compare) noexcept {
    switch (compare) {
      case DepthComparison::never:        return D3D11_COMPARISON_NEVER;
      case DepthComparison::less:         return D3D11_COMPARISON_LESS;
      case DepthComparison::lessEqual:    return D3D11_COMPARISON_LESS_EQUAL;
      case DepthComparison::equal:        return D3D11_COMPARISON_EQUAL;
      case DepthComparison::notEqual:     return D3D11_COMPARISON_NOT_EQUAL;
      case DepthComparison::greaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
      case DepthComparison::greater:      return D3D11_COMPARISON_GREATER;
      case DepthComparison::always:       return D3D11_COMPARISON_ALWAYS;
      default: return D3D11_COMPARISON_ALWAYS;
    }
  }
  // Convert portable depth/stencil operation enum to Direct3D operation enum
  static D3D11_STENCIL_OP __toDepthStencilOperation(DepthStencilOperation op) noexcept {
    switch (op) {
      case DepthStencilOperation::keep:           return D3D11_STENCIL_OP_KEEP;
      case DepthStencilOperation::setZero:        return D3D11_STENCIL_OP_ZERO;
      case DepthStencilOperation::replace:        return D3D11_STENCIL_OP_REPLACE;
      case DepthStencilOperation::invert:         return D3D11_STENCIL_OP_INVERT;
      case DepthStencilOperation::incrementClamp: return D3D11_STENCIL_OP_INCR_SAT;
      case DepthStencilOperation::decrementClamp: return D3D11_STENCIL_OP_DECR_SAT;
      case DepthStencilOperation::incrementWrap:  return D3D11_STENCIL_OP_INCR;
      case DepthStencilOperation::decrementWrap:  return D3D11_STENCIL_OP_DECR;
      default: return D3D11_STENCIL_OP_KEEP;
    }
  }
  
  // ---

  // Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createDepthTestState(const DepthOperationGroup& frontFaceOp, 
                                                   const DepthOperationGroup& backFaceOp,
                                                   DepthComparison depthTest, bool writeMaskAll) { // throws
    D3D11_DEPTH_STENCIL_DESC depthStDescriptor;
    ZeroMemory(&depthStDescriptor, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // depth test
    depthStDescriptor.DepthEnable = true;
    depthStDescriptor.DepthWriteMask = writeMaskAll ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStDescriptor.DepthFunc = __toDepthComparison(depthTest);
    // stencil test
    depthStDescriptor.StencilEnable = false;
    depthStDescriptor.StencilReadMask = 0xFF;
    depthStDescriptor.StencilWriteMask = 0xFF;
    // front-facing/back-facing operations
    depthStDescriptor.FrontFace.StencilFailOp = depthStDescriptor.FrontFace.StencilDepthFailOp = __toDepthStencilOperation(frontFaceOp.failureOp);
    depthStDescriptor.FrontFace.StencilPassOp = __toDepthStencilOperation(frontFaceOp.passOp);
    depthStDescriptor.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStDescriptor.BackFace.StencilFailOp = depthStDescriptor.BackFace.StencilDepthFailOp = __toDepthStencilOperation(backFaceOp.failureOp);
    depthStDescriptor.BackFace.StencilPassOp = __toDepthStencilOperation(backFaceOp.passOp);
    depthStDescriptor.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&depthStDescriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: depth state failure");
    return DepthStencilState(stateData);
  }
  
  // Create stencil test state (disable depth test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createStencilTestState(const DepthStencilOperationGroup& frontFaceOp, 
                                                     const DepthStencilOperationGroup& backFaceOp, 
                                                     uint8_t readMask, uint8_t writeMask) { // throws
    D3D11_DEPTH_STENCIL_DESC depthStDescriptor;
    ZeroMemory(&depthStDescriptor, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // depth test
    depthStDescriptor.DepthEnable = false;
    depthStDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStDescriptor.DepthFunc = D3D11_COMPARISON_ALWAYS;
    // stencil test
    depthStDescriptor.StencilEnable = true;
    depthStDescriptor.StencilReadMask = (UINT8)readMask;
    depthStDescriptor.StencilWriteMask = (UINT8)writeMask;
    // front-facing/back-facing operations
    depthStDescriptor.FrontFace.StencilFailOp = __toDepthStencilOperation(frontFaceOp.failureOp);
    depthStDescriptor.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStDescriptor.FrontFace.StencilPassOp = __toDepthStencilOperation(frontFaceOp.passOp);
    depthStDescriptor.FrontFace.StencilFunc = __toDepthComparison(frontFaceOp.stencilTest);
    depthStDescriptor.BackFace.StencilFailOp = __toDepthStencilOperation(backFaceOp.failureOp);
    depthStDescriptor.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStDescriptor.BackFace.StencilPassOp = __toDepthStencilOperation(backFaceOp.passOp);
    depthStDescriptor.BackFace.StencilFunc = __toDepthComparison(backFaceOp.stencilTest);

    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&depthStDescriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: stencil state failure");
    return DepthStencilState(stateData);
  }
  
  // Create depth/stencil test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createDepthStencilTestState(const DepthStencilOperationGroup& frontFaceOp, 
                                                          const DepthStencilOperationGroup& backFaceOp, 
                                                          DepthComparison depthTest, bool depthWriteMaskAll, 
                                                          uint8_t stencilReadMask, uint8_t stencilWriteMask) { // throws
    D3D11_DEPTH_STENCIL_DESC depthStDescriptor;
    ZeroMemory(&depthStDescriptor, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // depth test
    depthStDescriptor.DepthEnable = true;
    depthStDescriptor.DepthWriteMask = depthWriteMaskAll ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStDescriptor.DepthFunc = __toDepthComparison(depthTest);
    // stencil test
    depthStDescriptor.StencilEnable = true;
    depthStDescriptor.StencilReadMask = (UINT8)stencilReadMask;
    depthStDescriptor.StencilWriteMask = (UINT8)stencilWriteMask;
    // front-facing/back-facing operations
    depthStDescriptor.FrontFace.StencilFailOp = __toDepthStencilOperation(frontFaceOp.failureOp);
    depthStDescriptor.FrontFace.StencilDepthFailOp = __toDepthStencilOperation(frontFaceOp.depthFailureOp);
    depthStDescriptor.FrontFace.StencilPassOp = __toDepthStencilOperation(frontFaceOp.passOp);
    depthStDescriptor.FrontFace.StencilFunc = __toDepthComparison(frontFaceOp.stencilTest);
    depthStDescriptor.BackFace.StencilFailOp = __toDepthStencilOperation(backFaceOp.failureOp);
    depthStDescriptor.BackFace.StencilDepthFailOp = __toDepthStencilOperation(backFaceOp.depthFailureOp);
    depthStDescriptor.BackFace.StencilPassOp = __toDepthStencilOperation(backFaceOp.passOp);
    depthStDescriptor.BackFace.StencilFunc = __toDepthComparison(backFaceOp.stencilTest);

    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&depthStDescriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: depth/stencil failure");
    return DepthStencilState(stateData);
  }


// -- renderer state factory - rasterizer -- -----------------------------------

  // Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
  RasterizerState RendererStateFactory::createRasterizerState(CullMode culling, bool isFrontClockwise, 
                                                  const pandora::video::DepthBias& depth,
                                                  bool scissorClipping) { // throws
    D3D11_RASTERIZER_DESC rasterizerState;
    ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
    
    switch (culling) {
      case CullMode::wireFrame: rasterizerState.FillMode = D3D11_FILL_WIREFRAME; rasterizerState.CullMode = D3D11_CULL_NONE; break;
      case CullMode::cullBack:  rasterizerState.FillMode = D3D11_FILL_SOLID; rasterizerState.CullMode = D3D11_CULL_BACK; break;
      case CullMode::cullFront: rasterizerState.FillMode = D3D11_FILL_SOLID; rasterizerState.CullMode = D3D11_CULL_FRONT; break;
      default: rasterizerState.FillMode = D3D11_FILL_SOLID; rasterizerState.CullMode = D3D11_CULL_NONE; break;
    }
    rasterizerState.FrontCounterClockwise = isFrontClockwise ? FALSE : TRUE;
    
    rasterizerState.DepthBias = (INT)depth.depthBias;
    rasterizerState.DepthBiasClamp = (FLOAT)depth.depthBiasClamp;
    rasterizerState.SlopeScaledDepthBias = (FLOAT)depth.depthBiasSlopeScale;
    rasterizerState.DepthClipEnable = depth.isClipped ? TRUE : FALSE;

    rasterizerState.MultisampleEnable = FALSE;
    rasterizerState.AntialiasedLineEnable = FALSE;
    rasterizerState.ScissorEnable = scissorClipping ? TRUE : FALSE;

    ID3D11RasterizerState* stateData = nullptr;
    auto result = this->_device->CreateRasterizerState(&rasterizerState, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: raster state failure");
    return RasterizerState(stateData);
  }


// -- renderer state factory - blending -- -------------------------------------

  // Convert portable grouped blend factors to Direct3D blend factors
  static void __toBlendFactor(BlendFactor factor, D3D11_BLEND& outColorBlend, D3D11_BLEND& outAlphaBlend) noexcept {
    switch (factor) {
      case BlendFactor::zero:           outColorBlend = outAlphaBlend = D3D11_BLEND_ZERO; break;
      case BlendFactor::one:            outColorBlend = outAlphaBlend = D3D11_BLEND_ONE; break;
      case BlendFactor::sourceColor:     outColorBlend = D3D11_BLEND_SRC_COLOR;      outAlphaBlend = D3D11_BLEND_SRC_ALPHA; break;
      case BlendFactor::sourceInvColor:  outColorBlend = D3D11_BLEND_INV_SRC_COLOR;  outAlphaBlend = D3D11_BLEND_INV_SRC_ALPHA; break;
      case BlendFactor::destColor:       outColorBlend = D3D11_BLEND_DEST_COLOR;     outAlphaBlend = D3D11_BLEND_DEST_ALPHA; break;
      case BlendFactor::destInvColor:    outColorBlend = D3D11_BLEND_INV_DEST_COLOR; outAlphaBlend = D3D11_BLEND_INV_DEST_ALPHA; break;
      case BlendFactor::dualSrcColor:    outColorBlend = D3D11_BLEND_SRC1_COLOR;     outAlphaBlend = D3D11_BLEND_SRC1_ALPHA; break;
      case BlendFactor::dualSrcInvColor: outColorBlend = D3D11_BLEND_INV_SRC1_COLOR; outAlphaBlend = D3D11_BLEND_INV_SRC1_ALPHA; break;
      case BlendFactor::sourceAlpha:    outColorBlend = outAlphaBlend = D3D11_BLEND_SRC_ALPHA; break;
      case BlendFactor::sourceInvAlpha: outColorBlend = outAlphaBlend = D3D11_BLEND_INV_SRC_ALPHA; break;
      case BlendFactor::destAlpha:      outColorBlend = outAlphaBlend = D3D11_BLEND_DEST_ALPHA; break;
      case BlendFactor::destInvAlpha:   outColorBlend = outAlphaBlend = D3D11_BLEND_INV_DEST_ALPHA; break;
      case BlendFactor::sourceAlphaSat: outColorBlend = outAlphaBlend = D3D11_BLEND_SRC_ALPHA_SAT; break;
      case BlendFactor::dualSrcAlpha:    outColorBlend = outAlphaBlend = D3D11_BLEND_SRC1_ALPHA; break;
      case BlendFactor::dualSrcInvAlpha: outColorBlend = outAlphaBlend = D3D11_BLEND_INV_SRC1_ALPHA; break;
      case BlendFactor::constantColor:    outColorBlend = outAlphaBlend = D3D11_BLEND_BLEND_FACTOR; break;
      case BlendFactor::constantInvColor: outColorBlend = outAlphaBlend = D3D11_BLEND_INV_BLEND_FACTOR; break;
      default: outColorBlend = outAlphaBlend = D3D11_BLEND_ZERO; break;
    }
  }
  // Convert portable separate blend factors to Direct3D blend factors
  static D3D11_BLEND __toBlendFactor(BlendFactor factor, bool isAlpha) noexcept {
    switch (factor) {
      case BlendFactor::zero:           return D3D11_BLEND_ZERO;
      case BlendFactor::one:            return D3D11_BLEND_ONE;
      case BlendFactor::sourceColor:    return isAlpha ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_SRC_COLOR;
      case BlendFactor::sourceInvColor: return isAlpha ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_INV_SRC_COLOR;
      case BlendFactor::sourceAlpha:    return D3D11_BLEND_SRC_ALPHA;
      case BlendFactor::sourceInvAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
      case BlendFactor::destColor:      return isAlpha ? D3D11_BLEND_DEST_ALPHA : D3D11_BLEND_DEST_COLOR;
      case BlendFactor::destInvColor:   return isAlpha ? D3D11_BLEND_INV_DEST_ALPHA : D3D11_BLEND_INV_DEST_COLOR;
      case BlendFactor::destAlpha:      return D3D11_BLEND_DEST_ALPHA;
      case BlendFactor::destInvAlpha:   return D3D11_BLEND_INV_DEST_ALPHA;
      case BlendFactor::sourceAlphaSat: return D3D11_BLEND_SRC_ALPHA_SAT;
      case BlendFactor::dualSrcColor:    return isAlpha ? D3D11_BLEND_SRC1_ALPHA : D3D11_BLEND_SRC1_COLOR;
      case BlendFactor::dualSrcInvColor: return isAlpha ? D3D11_BLEND_INV_SRC1_ALPHA : D3D11_BLEND_INV_SRC1_COLOR;
      case BlendFactor::dualSrcAlpha:    return D3D11_BLEND_SRC1_ALPHA;
      case BlendFactor::dualSrcInvAlpha: return D3D11_BLEND_INV_SRC1_ALPHA;
      case BlendFactor::constantColor:    return D3D11_BLEND_BLEND_FACTOR;
      case BlendFactor::constantInvColor: return D3D11_BLEND_INV_BLEND_FACTOR;
      default: return D3D11_BLEND_ZERO;
    }
  }
  // Convert portable blend operators to Direct3D blend operators
  static D3D11_BLEND_OP __toBlendOperator(BlendOperator op) noexcept {
    switch (op) {
      case BlendOperator::add:         return D3D11_BLEND_OP_ADD;
      case BlendOperator::subtract:    return D3D11_BLEND_OP_SUBTRACT;
      case BlendOperator::revSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
      case BlendOperator::minimum:     return D3D11_BLEND_OP_MIN;
      case BlendOperator::maximum:     return D3D11_BLEND_OP_MAX;
      default: return D3D11_BLEND_OP_ADD;
    }
  }
  
  // Fill grouped blend state params
  static inline void __fillBlendStateParams(BlendFactor sourceFactor, BlendFactor destFactor, 
                                            BlendOperator op, D3D11_RENDER_TARGET_BLEND_DESC& outDescriptor) noexcept {
    outDescriptor.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (op != BlendOperator::none) {
      outDescriptor.BlendEnable = TRUE;
      __toBlendFactor(sourceFactor, outDescriptor.SrcBlend, outDescriptor.SrcBlendAlpha);
      __toBlendFactor(destFactor, outDescriptor.DestBlend, outDescriptor.DestBlendAlpha);
      outDescriptor.BlendOp = outDescriptor.BlendOpAlpha = __toBlendOperator(op);
    }
    else {
      outDescriptor.BlendEnable = FALSE;
      outDescriptor.SrcBlend = outDescriptor.SrcBlendAlpha = D3D11_BLEND_ONE;
      outDescriptor.DestBlend = outDescriptor.DestBlendAlpha = D3D11_BLEND_ZERO;
      outDescriptor.BlendOp = outDescriptor.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }
  }
  // Fill separate blend state params
  static inline void __fillBlendStateParams(BlendFactor sourceColorFactor, BlendFactor destColorFactor, BlendOperator colorOp, 
                                            BlendFactor sourceAlphaFactor, BlendFactor destAlphaFactor, BlendOperator alphaOp, 
                                            D3D11_RENDER_TARGET_BLEND_DESC& outDescriptor) noexcept {
    outDescriptor.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (colorOp != BlendOperator::none && alphaOp != BlendOperator::none) {
      outDescriptor.BlendEnable = TRUE;
      outDescriptor.SrcBlend = __toBlendFactor(sourceColorFactor, false);
      outDescriptor.SrcBlendAlpha = __toBlendFactor(sourceAlphaFactor, true);
      outDescriptor.DestBlend = __toBlendFactor(destColorFactor, false);
      outDescriptor.DestBlendAlpha = __toBlendFactor(destAlphaFactor, true);
      outDescriptor.BlendOp = __toBlendOperator(colorOp);
      outDescriptor.BlendOpAlpha = __toBlendOperator(alphaOp);
    }
    else {
      outDescriptor.BlendEnable = FALSE;
      outDescriptor.SrcBlend = outDescriptor.SrcBlendAlpha = D3D11_BLEND_ONE;
      outDescriptor.DestBlend = outDescriptor.DestBlendAlpha = D3D11_BLEND_ZERO;
      outDescriptor.BlendOp = outDescriptor.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }
  }
  
  // ---

  // Create general blend state (common to all render-targets)
  BlendState RendererStateFactory::createBlendState(BlendFactor sourceFactor, BlendFactor destFactor, BlendOperator op) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    __fillBlendStateParams(sourceFactor, destFactor, op, descriptor.RenderTarget[0]);
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: blend state failure");
    return BlendState(stateData);
  }
  // Create general blend state (common to all render-targets) - separate color/alpha params
  BlendState RendererStateFactory::createBlendState(BlendFactor sourceColorFactor, BlendFactor destColorFactor, BlendOperator colorOp, 
                                                    BlendFactor sourceAlphaFactor, BlendFactor destAlphaFactor, BlendOperator alphaOp) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    __fillBlendStateParams(sourceColorFactor, destColorFactor, colorOp, sourceAlphaFactor, destAlphaFactor, alphaOp, descriptor.RenderTarget[0]);
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: split blend failure");
    return BlendState(stateData);
  }

  // Create blend state with different grouped params per render-target (up to 'Renderer::maxRenderTargets()' targets (usually 8))
  BlendState RendererStateFactory::createBlendStatePerTarget(const TargetBlendingParams* perTargetParams, size_t arrayLength) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    descriptor.IndependentBlendEnable = TRUE;
    for (size_t i = 0; i < arrayLength; ++i, ++perTargetParams) {
      __fillBlendStateParams(perTargetParams->sourceFactor, perTargetParams->destFactor, perTargetParams->op, descriptor.RenderTarget[i]);
    }
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: per-target blend failure");
    return BlendState(stateData);
  }
  // Create blend state with different color/alpha params per render-target (up to 'Renderer::maxRenderTargets()' targets (usually 8))
  BlendState RendererStateFactory::createBlendStatePerTarget(const TargetBlendingSplitParams* perTargetParams, size_t arrayLength) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    descriptor.IndependentBlendEnable = TRUE;
    for (size_t i = 0; i < arrayLength; ++i, ++perTargetParams) {
      __fillBlendStateParams(perTargetParams->sourceColorFactor, perTargetParams->destColorFactor, perTargetParams->colorOp, 
                             perTargetParams->sourceAlphaFactor, perTargetParams->destAlphaFactor, perTargetParams->alphaOp, 
                             descriptor.RenderTarget[i]);
    }
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: per-target split blend failure");
    return BlendState(stateData);
  }


// -- renderer state factory - sampler -- --------------------------------------
  
  // Convert portable filter types to Direct3D filter type
  static D3D11_FILTER __toFilterType(MinificationFilter minFilter, MagnificationFilter magFilter) noexcept {
    bool isMagLinear = (magFilter == MagnificationFilter::linear);
    switch (minFilter) {
      case MinificationFilter::nearest_mipNearest:
      case MinificationFilter::nearest: return isMagLinear ? D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
      case MinificationFilter::linear_mipNearest:
      case MinificationFilter::linear:  return isMagLinear ? D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
      case MinificationFilter::nearest_mipLinear: return isMagLinear ? D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
      case MinificationFilter::linear_mipLinear:  return isMagLinear ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
      default: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    }
  }
  // Convert portable filter types to Direct3D filter type with comparison
  static D3D11_FILTER __toFilterComparedType(MinificationFilter minFilter, MagnificationFilter magFilter) noexcept {
    bool isMagLinear = (magFilter == MagnificationFilter::linear);
    switch (minFilter) {
      case MinificationFilter::nearest_mipNearest:
      case MinificationFilter::nearest:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
      case MinificationFilter::linear_mipNearest:
      case MinificationFilter::linear:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
      case MinificationFilter::nearest_mipLinear:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
      case MinificationFilter::linear_mipLinear:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
      default: return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    }
  }
  // Convert portable texture-addressing to Direct3D addressing enum
  static D3D11_TEXTURE_ADDRESS_MODE __toFilterTextureAddress(TextureAddressMode mode) noexcept {
    switch (mode) {
      case TextureAddressMode::border: return D3D11_TEXTURE_ADDRESS_BORDER;
      case TextureAddressMode::clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
      case TextureAddressMode::repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
      case TextureAddressMode::repeatMirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
      case TextureAddressMode::mirrorClamp:  return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
      default: return D3D11_TEXTURE_ADDRESS_WRAP;
    }
  }
  
  // Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createFilter(MinificationFilter minFilter, MagnificationFilter magFilter, 
                                                 const TextureAddressMode texAddressUVW[3], float lodMin, 
                                                 float lodMax, float lodBias, const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = __toFilterType(minFilter, magFilter);
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler failure");
    return FilterState(stateData);
  }
  // Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createComparedFilter(MinificationFilter minFilter, MagnificationFilter magFilter,
                                                         const TextureAddressMode texAddressUVW[3], DepthComparison compare, 
                                                         float lodMin, float lodMax, float lodBias, const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = __toFilterComparedType(minFilter, magFilter);
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = __toDepthComparison(compare);
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler-comp failure");
    return FilterState(stateData);
  }
  
  // Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createAnisotropicFilter(uint32_t maxAnisotropy, const TextureAddressMode texAddressUVW[3],
                                                            float lodMin, float lodMax, float lodBias, const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = (maxAnisotropy <= (uint32_t)D3D11_MAX_MAXANISOTROPY) ? maxAnisotropy : D3D11_MAX_MAXANISOTROPY;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: anisotropic sampler failure");
    return FilterState(stateData);
  }
  // Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createComparedAnisotropicFilter(uint32_t maxAnisotropy, const TextureAddressMode texAddressUVW[3],
                                                                    DepthComparison compare, float lodMin, float lodMax, float lodBias, 
                                                                    const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = (maxAnisotropy <= (uint32_t)D3D11_MAX_MAXANISOTROPY) ? maxAnisotropy : D3D11_MAX_MAXANISOTROPY;
    samplerDesc.ComparisonFunc = __toDepthComparison(compare);
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: anisotropic sampler-comp failure");
    return FilterState(stateData);
  }


// -----------------------------------------------------------------------------
// renderer.h/swap_chain.h : color management
// -----------------------------------------------------------------------------

  // Convert standard sRGB(A) color to device RGB(A)
  void Renderer::toGammaCorrectColor(const float colorRgba[4], FLOAT outRgba[4]) noexcept {
    DirectX::XMFLOAT3 colorArray(colorRgba);
    DirectX::XMStoreFloat3(&colorArray, DirectX::XMColorSRGBToRGB(DirectX::XMLoadFloat3(&colorArray))); // gamma-correct color
    outRgba[0] = colorArray.x;
    outRgba[1] = colorArray.y;
    outRgba[2] = colorArray.z;
    outRgba[3] = colorRgba[3];
  }

  // Convert portable component format to DXGI_FORMAT
  DXGI_FORMAT Renderer::toDxgiFormat(ComponentFormat format) noexcept {
    switch (format) {
      case ComponentFormat::rgba8_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      case ComponentFormat::rgba8_unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
      case ComponentFormat::rgba8_snorm: return DXGI_FORMAT_R8G8B8A8_SNORM;
      case ComponentFormat::rgba8_ui: return DXGI_FORMAT_R8G8B8A8_UINT;
      case ComponentFormat::rgba8_i: return DXGI_FORMAT_R8G8B8A8_SINT;
      
      case ComponentFormat::d32_f: return DXGI_FORMAT_D32_FLOAT;
      case ComponentFormat::d16_unorm: return DXGI_FORMAT_D16_UNORM;
      case ComponentFormat::d32_f_s8_ui: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
      case ComponentFormat::d24_unorm_s8_ui: return DXGI_FORMAT_D24_UNORM_S8_UINT;
      
      case ComponentFormat::rgba16_f_hdr_scRGB: return DXGI_FORMAT_R16G16B16A16_FLOAT;
      case ComponentFormat::rgba16_unorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
      case ComponentFormat::rgba16_snorm: return DXGI_FORMAT_R16G16B16A16_SNORM;
      case ComponentFormat::rgba16_ui: return DXGI_FORMAT_R16G16B16A16_UINT;
      case ComponentFormat::rgba16_i: return DXGI_FORMAT_R16G16B16A16_SINT;
      case ComponentFormat::rgb10a2_unorm_hdr10: return DXGI_FORMAT_R10G10B10A2_UNORM;
      case ComponentFormat::rgb10a2_ui: return DXGI_FORMAT_R10G10B10A2_UINT;
      case ComponentFormat::rgba32_f: return DXGI_FORMAT_R32G32B32A32_FLOAT;
      case ComponentFormat::rgba32_ui: return DXGI_FORMAT_R32G32B32A32_UINT;
      case ComponentFormat::rgba32_i: return DXGI_FORMAT_R32G32B32A32_SINT;
      
      case ComponentFormat::bc6h_uf: return DXGI_FORMAT_BC6H_UF16;
      case ComponentFormat::bc6h_f: return DXGI_FORMAT_BC6H_SF16;
      case ComponentFormat::bc7_sRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
      case ComponentFormat::bc7_unorm: return DXGI_FORMAT_BC7_UNORM;
      
      case ComponentFormat::bgra8_sRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      case ComponentFormat::bgra8_unorm: return DXGI_FORMAT_B8G8R8A8_UNORM;
      case ComponentFormat::r8_ui: return DXGI_FORMAT_R8_UINT;
      case ComponentFormat::r8_i: return DXGI_FORMAT_R8_SINT;
      case ComponentFormat::r8_unorm: return DXGI_FORMAT_R8_UNORM;
      case ComponentFormat::r8_snorm: return DXGI_FORMAT_R8_SNORM;
      case ComponentFormat::a8_unorm: return DXGI_FORMAT_A8_UNORM;
      case ComponentFormat::rg8_unorm: return DXGI_FORMAT_R8G8_UNORM;
      case ComponentFormat::rg8_snorm: return DXGI_FORMAT_R8G8_SNORM;
      case ComponentFormat::rg8_ui: return DXGI_FORMAT_R8G8_UINT;
      case ComponentFormat::rg8_i: return DXGI_FORMAT_R8G8_SINT;
      case ComponentFormat::rgb5a1_unorm: return DXGI_FORMAT_B5G5R5A1_UNORM;
      case ComponentFormat::r5g6b5_unorm: return DXGI_FORMAT_B5G6R5_UNORM;
      
      case ComponentFormat::rg16_f: return DXGI_FORMAT_R16G16_FLOAT;
      case ComponentFormat::r16_f: return DXGI_FORMAT_R16_FLOAT;
      case ComponentFormat::rg16_unorm: return DXGI_FORMAT_R16G16_UNORM;
      case ComponentFormat::r16_unorm: return DXGI_FORMAT_R16_UNORM;
      case ComponentFormat::rg16_snorm: return DXGI_FORMAT_R16G16_SNORM;
      case ComponentFormat::r16_snorm: return DXGI_FORMAT_R16_SNORM;
      case ComponentFormat::rg16_ui: return DXGI_FORMAT_R16G16_UINT;
      case ComponentFormat::r16_ui: return DXGI_FORMAT_R16_UINT;
      case ComponentFormat::rg16_i: return DXGI_FORMAT_R16G16_SINT;
      case ComponentFormat::r16_i: return DXGI_FORMAT_R16_SINT;
      case ComponentFormat::rgb32_f: return DXGI_FORMAT_R32G32B32_FLOAT;
      case ComponentFormat::rg32_f: return DXGI_FORMAT_R32G32_FLOAT;
      case ComponentFormat::r32_f: return DXGI_FORMAT_R32_FLOAT;
      case ComponentFormat::rgb32_ui: return DXGI_FORMAT_R32G32B32_UINT;
      case ComponentFormat::rg32_ui: return DXGI_FORMAT_R32G32_UINT;
      case ComponentFormat::r32_ui: return DXGI_FORMAT_R32_UINT;
      case ComponentFormat::rgb32_i: return DXGI_FORMAT_R32G32B32_SINT;
      case ComponentFormat::rg32_i: return DXGI_FORMAT_R32G32_SINT;
      case ComponentFormat::r32_i: return DXGI_FORMAT_R32_SINT;
      case ComponentFormat::rg11b10_f: return DXGI_FORMAT_R11G11B10_FLOAT;
      case ComponentFormat::rgb9e5_uf: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
      
#     if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
        case ComponentFormat::rgba4_unorm: return DXGI_FORMAT_B4G4R4A4_UNORM;
#     endif
      case ComponentFormat::unknown: 
      default: return DXGI_FORMAT_UNKNOWN;
    }
  }

# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
    // Find color space for a buffer format
    static inline DXGI_COLOR_SPACE_TYPE __getColorSpace(DXGI_FORMAT backBufferFormat) noexcept {
      switch (backBufferFormat) {
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
          return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020; // HDR-10
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32_SINT:
          return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709; // HDR-scRGB
        default: break;
      }
      return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709; // SDR-sRGB
    }
# endif
  
  // Set swap-chain color space
  // returns: color spaces supported (true) or not
  static bool __setColorSpace(IDXGISwapChain* swapChain, DXGI_FORMAT backBufferFormat, bool isHdrPreferred, DXGI_COLOR_SPACE_TYPE& outColorSpace) { // throws
    // verify HDR support
    outColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709; // SDR-sRGB
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#     if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
        if (isHdrPreferred) {
          D3dResource<IDXGIOutput> output;
          if (SUCCEEDED(swapChain->GetContainingOutput(output.address())) && output) {
            DXGI_OUTPUT_DESC1 outputInfo;
            auto outputV6 = D3dResource<IDXGIOutput6>::tryFromInterface(output.get());
            if (outputV6 && SUCCEEDED(outputV6->GetDesc1(&outputInfo))) {
              
              if (outputInfo.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
              ||  outputInfo.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709) {
                outColorSpace = __getColorSpace(backBufferFormat);
              }
            }
          }
        }
#     endif

      // apply color space
      auto swapChainV3 = D3dResource<IDXGISwapChain3>::tryFromInterface((IDXGISwapChain*)swapChain);
      if (swapChainV3) {
        UINT colorSpaceSupport = 0;
        if (SUCCEEDED(swapChainV3->CheckColorSpaceSupport(outColorSpace, &colorSpaceSupport))
        && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) {

          auto result = swapChainV3->SetColorSpace1(outColorSpace);
          if (FAILED(result))
            throwError(result, "SwapChain: color space failure");
          return true;
        }
      }
#   endif
    return false;
  }


// -----------------------------------------------------------------------------
// swap_chain.h
// -----------------------------------------------------------------------------

// -- swap-chain creation -- ---------------------------------------------------

  // Verify if buffer format is supported by flip-swap (-> outIsFlipSwapAllowed) + return swap-chain format to use
  static inline DXGI_FORMAT __verifyFlipSwapFormat(DXGI_FORMAT backBufferFormat, bool& outIsFlipSwapAllowed) noexcept {
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      switch (backBufferFormat) {
        case DXGI_FORMAT_R16G16B16A16_FLOAT: 
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UNORM:   outIsFlipSwapAllowed = true; return backBufferFormat;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: outIsFlipSwapAllowed = true; return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: outIsFlipSwapAllowed = true; return DXGI_FORMAT_B8G8R8A8_UNORM;
        default: break;
      }
#   endif
    outIsFlipSwapAllowed = false;
    return backBufferFormat;
  }

  // Convert portable params to Direct3D swap-chain params
  void SwapChain::_convertSwapChainParams(const Renderer& renderer, const pandora::video::SwapChainParams& params, 
                                          _SwapChainConfig& outConfig) noexcept {
    // convert buffer formats
    outConfig.backBufferFormat = (params.backBufferFormat() != pandora::video::ComponentFormat::custom)
                                 ? Renderer::toDxgiFormat(params.backBufferFormat())
                                 : (DXGI_FORMAT)params.customBackBufferFormat();
    outConfig.frameBufferCount = params.frameBufferCount();
    outConfig.isHdrPreferred = params.isHdrPreferred();
    
    // verify if flip-swap is applicable
    outConfig.useFlipSwap = (renderer.isFlipSwapAvailable() 
                        && params.renderTargetMode() == SwapChainTargetMode::uniqueOutput // only one render target view
                        && (renderer.isTearingAvailable()   // tearing OFF or supported with flip-swap
                           || (params.outputFlags() & SwapChainOutputFlag::variableRefresh) == SwapChainOutputFlag::none) );
    outConfig.swapChainFormat = (outConfig.useFlipSwap)     // supported color format
                              ? __verifyFlipSwapFormat((DXGI_FORMAT)outConfig.backBufferFormat, outConfig.useFlipSwap)
                              : outConfig.backBufferFormat;
    
    // buffer usage
    outConfig.bufferUsageMode = ((params.outputFlags() & SwapChainOutputFlag::shaderInput) == true)
                              ? (DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT)
                              : DXGI_USAGE_RENDER_TARGET_OUTPUT;
    outConfig.flags = params.outputFlags();
    if ((outConfig.flags & SwapChainOutputFlag::localOutput) == true && !renderer.isLocalDisplayRestrictionAvailable())
      outConfig.flags &= ~(SwapChainOutputFlag::localOutput);
  }
  
  // ---

  // Create DXGI swap-chain resource
  void* Renderer::_createSwapChain(const _SwapChainConfig& config, pandora::video::WindowHandle window,
                                   uint32_t rateNumerator, uint32_t rateDenominator, 
                                   D3D_FEATURE_LEVEL& outSwapChainLevel) { // throws
    void* swapChain = nullptr;
    __refreshDxgiFactory(this->_dxgiFactory);
    
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (this->_dxgiLevel >= 2u) {
        // Direct3D 11.1+
        auto dxgiFactoryV2 = D3dResource<IDXGIFactory2>::fromInterface((IDXGIFactory1*)this->_dxgiFactory, "Renderer: DXGI access failed");
        auto deviceV1 = D3dResource<ID3D11Device1>::fromInterface(this->_device, "Renderer: device access failed");
        outSwapChainLevel = D3D_FEATURE_LEVEL_11_1;
        
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscnDescriptor = {};
        ZeroMemory(&fullscnDescriptor, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
        DXGI_SWAP_CHAIN_DESC1 descriptor = {};
        ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC1));
        fullscnDescriptor.Windowed = ((config.flags & SwapChainOutputFlag::stereo) == true) ? FALSE : TRUE;
        descriptor.Width = config.width;
        descriptor.Height = config.height;
        descriptor.Format = config.swapChainFormat;
        descriptor.BufferCount = (UINT)config.frameBufferCount;
        descriptor.BufferUsage = config.bufferUsageMode;
        descriptor.Scaling = DXGI_SCALING_STRETCH;
        fullscnDescriptor.RefreshRate.Numerator = (UINT)rateNumerator;
        fullscnDescriptor.RefreshRate.Denominator = (UINT)rateDenominator;
        descriptor.SwapEffect = (config.useFlipSwap) ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
        descriptor.SampleDesc.Count = 1;
        descriptor.SampleDesc.Quality = 0;
        descriptor.Stereo = ((config.flags & SwapChainOutputFlag::stereo) == true) ? TRUE : FALSE;
        descriptor.Flags = (config.useFlipSwap && (config.flags & SwapChainOutputFlag::variableRefresh) == true) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        if ((config.flags & SwapChainOutputFlag::localOutput) == true)
          descriptor.Flags |= DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;

        auto result = dxgiFactoryV2->CreateSwapChainForHwnd(deviceV1.get(), (HWND)window, &descriptor, &fullscnDescriptor, nullptr, (IDXGISwapChain1**)&swapChain);
        if (FAILED(result) || swapChain == nullptr)
          throwError(result, "Renderer: swap-chain failure");
      }
      else
#   endif
    {
      // Direct3D 11.0
      outSwapChainLevel = D3D_FEATURE_LEVEL_11_0;
      DXGI_SWAP_CHAIN_DESC descriptor = {};
      ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC));
      descriptor.BufferDesc.Width = config.width;
      descriptor.BufferDesc.Height = config.height;
      descriptor.BufferDesc.Format = (DXGI_FORMAT)config.swapChainFormat;
      descriptor.BufferCount = (UINT)config.frameBufferCount;
      descriptor.BufferUsage = config.bufferUsageMode;
      descriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
      descriptor.BufferDesc.RefreshRate.Numerator = (UINT)rateNumerator;
      descriptor.BufferDesc.RefreshRate.Denominator = (UINT)rateDenominator;
      descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      descriptor.OutputWindow = (HWND)window;
      descriptor.Windowed = TRUE;
      descriptor.SampleDesc.Count = 1;
      descriptor.SampleDesc.Quality = 0;

      auto result = ((IDXGIFactory1*)this->_dxgiFactory)->CreateSwapChain(this->_device, &descriptor, (IDXGISwapChain**)&swapChain);
      if (FAILED(result) || swapChain == nullptr)
        throwError(result, "Renderer: swap-chain failure");
    }
    
    ((IDXGIFactory1*)this->_dxgiFactory)->MakeWindowAssociation((HWND)window, DXGI_MWA_NO_ALT_ENTER); // prevent DXGI from responding to the ALT+ENTER shortcut
    if (!config.useFlipSwap && (config.flags & SwapChainOutputFlag::variableRefresh) == true)
      ((IDXGISwapChain*)swapChain)->SetFullscreenState(TRUE, nullptr); // without flip-swap, screen tearing requires fullscreen state
    
    return swapChain;
  }
  
  // ---
  
  // Create rendering swap-chain for existing renderer
  SwapChain::SwapChain(std::shared_ptr<Renderer> renderer, const pandora::video::SwapChainParams& params, 
                       pandora::video::WindowHandle window, uint32_t width, uint32_t height)
    : _renderer(std::move(renderer)) { // throws
    if (this->_renderer == nullptr)
      throw std::invalid_argument("SwapChain: NULL renderer");
    if (window == nullptr)
      throw std::invalid_argument("SwapChain: NULL window");
    if (width == 0 || height == 0)
      throw std::invalid_argument("SwapChain: width/height is 0");
    
    // build swap-chain
    memset((void*)&this->_settings, 0, sizeof(_SwapChainConfig));
    this->_settings.width = width;
    this->_settings.height = height;
    this->_presentFlags = ((this->_settings.flags & SwapChainOutputFlag::variableRefresh) == true) ? DXGI_PRESENT_ALLOW_TEARING : 0;
    _convertSwapChainParams(*(this->_renderer), params, this->_settings);
    
    this->_swapChain = this->_renderer->_createSwapChain(this->_settings, window, params.rateNumerator(),
                                                         params.rateDenominator(), this->_swapChainLevel); // throws

    // find + set color space value (if supported)
    DXGI_COLOR_SPACE_TYPE colorSpace;
    __setColorSpace((IDXGISwapChain*)this->_swapChain, this->_settings.backBufferFormat,
                    this->_settings.isHdrPreferred, colorSpace);
    this->_settings.colorSpace = colorSpace;

    _createSwapChainTargetView(); // create render-target-view
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      auto contextV1 = D3dResource<ID3D11DeviceContext1>::tryFromInterface(this->_renderer->context());
      this->_deviceContext11_1 = contextV1.extract();
#   endif
  }


// -- swap-chain destruction/move -- -------------------------------------------

  // Destroy swap-chain
  void SwapChain::release() noexcept {
    if (this->_swapChain != nullptr) {
      try {
        if (this->_renderTargetView) {
          this->_renderer->setActiveRenderTarget(nullptr);
          this->_renderTargetView->Release();
          this->_renderTargetView = nullptr;
        }
#       if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
          if (this->_deviceContext11_1)
            this->_deviceContext11_1->Release();
#       endif
      } 
      catch (...) {}
        
      try {
        if (this->_swapChainLevel == D3D_FEATURE_LEVEL_11_1) {
          ((IDXGISwapChain1*)this->_swapChain)->SetFullscreenState(FALSE, nullptr); // fullscreen must be OFF, or Release will throw
          ((IDXGISwapChain1*)this->_swapChain)->Release();
        }
        else {
          ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(FALSE, nullptr); // fullscreen must be OFF, or Release will throw
          ((IDXGISwapChain*)this->_swapChain)->Release();
        }
      } 
      catch (...) {}
      this->_swapChain = nullptr;
      this->_renderer = nullptr;
    }
  }

  SwapChain::SwapChain(SwapChain&& rhs) noexcept
    : _renderer(std::move(rhs._renderer)),
      _renderTargetView(rhs._renderTargetView),
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        _deviceContext11_1(rhs._deviceContext11_1),
#     endif
      _swapChain(rhs._swapChain),
      _swapChainLevel(rhs._swapChainLevel),
      _presentFlags(rhs._presentFlags) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_SwapChainConfig));
    rhs._renderer = nullptr;
    rhs._swapChain = nullptr;
    rhs._renderTargetView = nullptr;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      rhs._deviceContext11_1 = nullptr;
#   endif
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    release();
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_SwapChainConfig));
    this->_renderer = std::move(rhs._renderer);
    this->_renderTargetView = rhs._renderTargetView;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      this->_deviceContext11_1 = rhs._deviceContext11_1;
#   endif
    this->_swapChain = rhs._swapChain;
    this->_swapChainLevel = rhs._swapChainLevel;
    this->_presentFlags = rhs._presentFlags;
    rhs._renderer = nullptr;
    rhs._swapChain = nullptr;
    rhs._renderTargetView = nullptr;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      rhs._deviceContext11_1 = nullptr;
#   endif
    return *this;
  }


// -- operations -- ------------------------------------------------------------

  // Change back-buffer(s) size + refresh color space
  bool SwapChain::resize(uint32_t width, uint32_t height) { // throws
    bool isResized = (width != this->_settings.width || height != this->_settings.height);
    if (isResized) {
      if (width == 0 || height == 0)
        throw std::invalid_argument("SwapChain: width/height is 0");
      
      // clear previous size-specific context
      this->_renderer->setActiveRenderTarget(nullptr);
      this->_renderTargetView->Release();
      this->_renderTargetView = nullptr;
      ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(FALSE, nullptr);
      this->_renderer->context()->Flush();
      
      // resize swap-chain
      DXGI_SWAP_CHAIN_FLAG flags = (this->_settings.useFlipSwap && (this->_settings.flags & SwapChainOutputFlag::variableRefresh) == true) 
                                 ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : (DXGI_SWAP_CHAIN_FLAG)0;
      auto result = ((IDXGISwapChain*)this->_swapChain)->ResizeBuffers((UINT)this->_settings.frameBufferCount, (UINT)width, (UINT)height, 
                                                                       (DXGI_FORMAT)this->_settings.swapChainFormat, flags);
      if (FAILED(result)) {
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_HUNG)
          throw std::domain_error("SwapChain: adapter changed: Renderer/SwapChain must be recreated");
        throwError(result, "SwapChain: resize failure");
      }
      
      this->_settings.width = width;
      this->_settings.height = height;
      // fullscreen required: tearing without flip-swap / stereo rendering
      if ((!this->_settings.useFlipSwap && (this->_settings.flags & SwapChainOutputFlag::variableRefresh) == true) 
      || (this->_settings.flags & SwapChainOutputFlag::stereo) == true)
        ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(TRUE, nullptr);
    }

    // find + set color space value (if supported)
    DXGI_COLOR_SPACE_TYPE colorSpace;
    __setColorSpace((IDXGISwapChain*)this->_swapChain, this->_settings.backBufferFormat,
                    this->_settings.isHdrPreferred, colorSpace);
    this->_settings.colorSpace = colorSpace;
    
    if (isResized) // create render-target-view
      _createSwapChainTargetView();
    return isResized;
  }
  
  // ---
  
  // Create swap-chain render-target-view
  void SwapChain::_createSwapChainTargetView() { // throws
    D3dResource<ID3D11Texture2D> renderTarget;
    auto targetResult = ((IDXGISwapChain*)this->_swapChain)->GetBuffer(0, IID_PPV_ARGS(renderTarget.address()));
    if (FAILED(targetResult) || !renderTarget.hasValue()) {
      throwError(targetResult, "SwapChain: render target not accessible");
      return;
    }
    
    CD3D11_RENDER_TARGET_VIEW_DESC viewDescriptor(D3D11_RTV_DIMENSION_TEXTURE2D, this->_settings.backBufferFormat);
    targetResult = this->_renderer->device()->CreateRenderTargetView(renderTarget.get(), &viewDescriptor, &(this->_renderTargetView));
    if (FAILED(targetResult) || this->_renderTargetView == nullptr)
      throwError(targetResult, "SwapChain: target view failure");
  }
  
  // ---
  
  // Throw appropriate exception for 'swap buffers' error
  void __processSwapBuffersError(HRESULT result) {
    switch (result) {
      // minor issues -> ignore
      case DXGI_ERROR_WAS_STILL_DRAWING:
      case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
      case DXGI_ERROR_NONEXCLUSIVE:
      case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: break;
      // device lost
      case DXGI_ERROR_DEVICE_REMOVED:
      case DXGI_ERROR_DEVICE_RESET: throw std::domain_error("SwapChain: device lost");
      // invalid option
      case DXGI_ERROR_CANNOT_PROTECT_CONTENT: throw std::invalid_argument("SwapChain: local display restriction not supported");
      default: throwError(result, "SwapChain: internal error"); break;
    }
  }

  // Swap back-buffer(s) and front-buffer, to display drawn content on screen
  void SwapChain::swapBuffers(bool useVsync) {
    auto result = ((IDXGISwapChain*)this->_swapChain)->Present(useVsync ? 1 : 0, this->_presentFlags);
    if (FAILED(result))
      __processSwapBuffersError(result);
    __refreshDxgiFactory(this->_renderer->_dxgiFactory);
  }
  
  // Swap back-buffer(s) and front-buffer, to display drawn content on screen + discard render-target content after it
  void SwapChain::swapBuffersDiscard(bool useVsync, Renderer::DepthStencilViewHandle depthBuffer) {
    auto result = ((IDXGISwapChain*)this->_swapChain)->Present(useVsync ? 1 : 0, this->_presentFlags);
    if (FAILED(result))
      __processSwapBuffersError(result);
    __refreshDxgiFactory(this->_renderer->_dxgiFactory);
    
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      // discard content of render target + depth/stencil buffer
      if (this->_deviceContext11_1) {
        ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView((ID3D11RenderTargetView*)this->_renderTargetView);
        if (depthBuffer != nullptr)
          ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView((ID3D11DepthStencilView*)depthBuffer);
      }
#   endif
  }


// -----------------------------------------------------------------------------
// _d3d_resource.h -- error messages
// -----------------------------------------------------------------------------

  void pandora::video::d3d11::throwError(HRESULT result, const char* messageContent) {
    auto message = std::string(messageContent) + ": D3D ";
    switch (result) {
      case E_UNEXPECTED: message += "E_UNEXPECTED"; break;
      case E_NOTIMPL: message += "E_NOTIMPL"; break;
      case E_OUTOFMEMORY: message += "E_OUTOFMEMORY"; break;
      case E_INVALIDARG: message += "E_INVALIDARG"; break;
      case E_NOINTERFACE: message += "E_NOINTERFACE"; break;
      case E_POINTER: message += "E_POINTER"; break;
      case E_HANDLE: message += "E_HANDLE"; break;
      case E_ABORT: message += "E_ABORT"; break;
      case E_FAIL: message += "E_FAIL"; break;
      case E_ACCESSDENIED: message += "E_ACCESSDENIED"; break;
      case E_PENDING: message += "E_PENDING"; break;
      case E_BOUNDS: message += "E_BOUNDS"; break;
      case E_CHANGED_STATE: message += "E_CHANGED_STATE"; break;
      case E_ILLEGAL_STATE_CHANGE: message += "E_ILLEGAL_STATE_CHANGE"; break;
      case E_ILLEGAL_METHOD_CALL: message += "E_ILLEGAL_METHOD_CALL"; break;
      case RO_E_METADATA_NAME_NOT_FOUND: message += "RO_E_METADATA_NAME_NOT_FOUND"; break;
      case RO_E_METADATA_NAME_IS_NAMESPACE: message += "RO_E_METADATA_NAME_IS_NAMESPACE"; break;
      case RO_E_METADATA_INVALID_TYPE_FORMAT: message += "RO_E_METADATA_INVALID_TYPE_FORMAT"; break;
      case RO_E_INVALID_METADATA_FILE: message += "RO_E_INVALID_METADATA_FILE"; break;
      case RO_E_CLOSED: message += "RO_E_CLOSED"; break;
      case RO_E_EXCLUSIVE_WRITE: message += "RO_E_EXCLUSIVE_WRITE"; break;
      case RO_E_ERROR_STRING_NOT_FOUND: message += "RO_E_ERROR_STRING_NOT_FOUND"; break;
      case E_STRING_NOT_NULL_TERMINATED: message += "E_STRING_NOT_NULL_TERMINATED"; break;
      case E_ILLEGAL_DELEGATE_ASSIGNMENT: message += "E_ILLEGAL_DELEGATE_ASSIGNMENT"; break;
      case E_ASYNC_OPERATION_NOT_STARTED: message += "E_ASYNC_OPERATION_NOT_STARTED"; break;
      case CO_E_INIT_ONLY_SINGLE_THREADED: message += "CO_E_INIT_ONLY_SINGLE_THREADED"; break;
      case CO_E_CANT_REMOTE: message += "E_ASYNC_OPERATIOCO_E_CANT_REMOTEN_NOT_STARTED"; break;
      case CO_E_LAUNCH_PERMSSION_DENIED: message += "CO_E_LAUNCH_PERMSSION_DENIED"; break;
      case CO_E_REMOTE_COMMUNICATION_FAILURE: message += "CO_E_REMOTE_COMMUNICATION_FAILURE"; break;
      case CO_E_IIDREG_INCONSISTENT: message += "CO_E_IIDREG_INCONSISTENT"; break;
      case CO_E_NOT_SUPPORTED: message += "CO_E_NOT_SUPPORTED"; break;
      case CO_E_RELOAD_DLL: message += "CO_E_RELOAD_DLL"; break;
      default: message += std::to_string((int32_t)result & 0xFFFF); break;
    }
    throw std::runtime_error(std::move(message));
  }
  
  void pandora::video::d3d11::throwShaderError(ID3DBlob* errorMessage, const char* messagePrefix, const char* shaderInfo) {
    std::string message(messagePrefix);
    message += " (";
    message += shaderInfo;
    message += "): ";
    if (errorMessage) {
      message += (const char*)errorMessage->GetBufferPointer();
      errorMessage->Release();
    }
    else
      message += "missing/empty shader file/content";
    throw std::runtime_error(std::move(message));
  }
  
  
// -----------------------------------------------------------------------------
// static_buffer.h
// -----------------------------------------------------------------------------

  static D3D11_BIND_FLAG __toBindFlag(pandora::video::DataBufferType type) {
    switch (type) {
      case pandora::video::DataBufferType::constant:    return D3D11_BIND_CONSTANT_BUFFER;
      case pandora::video::DataBufferType::vertexArray: return D3D11_BIND_VERTEX_BUFFER;
      case pandora::video::DataBufferType::vertexIndex: return D3D11_BIND_INDEX_BUFFER;
      default: return D3D11_BIND_SHADER_RESOURCE;
    }
  }

  // Create data buffer (to store data for shader stages)
  StaticBuffer::StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize) 
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation failed");
  }

  // Create data buffer (to store data for shader stages) with initial value
  StaticBuffer::StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                             size_t bufferByteSize, const void* initData, bool isImmutable)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    
    if (isImmutable) {
      if (initData == nullptr)
        throw std::invalid_argument("Buffer: immutable with NULL initData");
      bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
    }
    else
      bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    D3D11_SUBRESOURCE_DATA subResData;
    ZeroMemory(&subResData, sizeof(subResData));
    subResData.pSysMem = initData;
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, initData ? &subResData : nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation failed");
  }


// -----------------------------------------------------------------------------
// dynamic_buffer.h
// -----------------------------------------------------------------------------

  // Create data buffer (to store data for shader stages)
  DynamicBuffer::DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation failed");
  }

  // Create data buffer (to store data for shader stages) with initial value
  DynamicBuffer::DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                               size_t bufferByteSize, const void* initData)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    D3D11_SUBRESOURCE_DATA subResData;
    ZeroMemory(&subResData, sizeof(subResData));
    subResData.pSysMem = initData;
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, initData ? &subResData : nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation failed");
  }

  // ---

  // Write buffer data and discard previous data - recommended for first write of the buffer for a frame
  bool DynamicBuffer::writeDiscard(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto lockResult = renderer.context()->Map(this->_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(lockResult) || mappedResource.pData == nullptr)
      return false;

    memcpy(mappedResource.pData, sourceData, this->_bufferSize);
    renderer.context()->Unmap(this->_buffer, 0);
    return true;
  }
  // Vertex/index buffers: write buffer data with no overwrite - recommended for subsequent writes of the buffer within same frame.
  // Constant buffers: same as 'writeDiscard'.
  bool DynamicBuffer::write(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto writeMode = (this->_type != pandora::video::DataBufferType::constant) ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
    auto lockResult = renderer.context()->Map(this->_buffer, 0, writeMode, 0, &mappedResource);
    if (FAILED(lockResult) || mappedResource.pData == nullptr)
      return false;

    memcpy(mappedResource.pData, sourceData, this->_bufferSize);
    renderer.context()->Unmap(this->_buffer, 0);
    return true;
  }


// -----------------------------------------------------------------------------
// depth_stencil_buffer.h
// -----------------------------------------------------------------------------

  // Create depth/stencil buffer for existing renderer/render-target
  DepthStencilBuffer::DepthStencilBuffer(Renderer& renderer, pandora::video::ComponentFormat format, 
                                         uint32_t width, uint32_t height) { // throws
    if (width == 0 || height == 0)
      throw std::invalid_argument("DepthStencil: width/height is 0");
    
    // create compatible depth/stencil buffer
    D3D11_TEXTURE2D_DESC depthDescriptor;
    ZeroMemory(&depthDescriptor, sizeof(depthDescriptor));
    depthDescriptor.Width = (UINT)width;
    depthDescriptor.Height = (UINT)height;
    depthDescriptor.MipLevels = 1;
    depthDescriptor.ArraySize = 1;
    depthDescriptor.Format = (DXGI_FORMAT)Renderer::toDxgiFormat(format);
    depthDescriptor.SampleDesc.Count = 1;
    depthDescriptor.SampleDesc.Quality = 0;
    depthDescriptor.Usage = D3D11_USAGE_DEFAULT;
    depthDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    auto result = renderer.device()->CreateTexture2D(&depthDescriptor, nullptr, &(this->_depthStencilBuffer));
    if (FAILED(result) || this->_depthStencilBuffer == nullptr) {
      throwError(result, "DepthStencil: creation failed"); return;
    }
    
    // create depth/stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDescriptor;
    ZeroMemory(&depthViewDescriptor, sizeof(depthViewDescriptor));
    depthViewDescriptor.Format = depthDescriptor.Format;
    depthViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDescriptor.Texture2D.MipSlice = 0;
    
    result = renderer.device()->CreateDepthStencilView(this->_depthStencilBuffer, &depthViewDescriptor, &(this->_depthStencilView));
    if (FAILED(result) || this->_depthStencilView == nullptr)
      throwError(result, "DepthStencil: view failure");
    
    this->_settings.width = width;
    this->_settings.height = height;
    this->_settings.format = format;
  }

  // Destroy depth/stencil buffer
  void DepthStencilBuffer::release() noexcept {
    if (this->_depthStencilBuffer) {
      if (this->_depthStencilView) {
        try { this->_depthStencilView->Release(); } catch (...) {}
        this->_depthStencilView = nullptr;
      }
      try { this->_depthStencilBuffer->Release(); } catch (...) {}
      this->_depthStencilBuffer = nullptr;
    }
  }
  
  // ---
  
  DepthStencilBuffer::DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept 
    : _depthStencilView(rhs._depthStencilView),
      _depthStencilBuffer(rhs._depthStencilBuffer) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_DepthStencilBufferConfig));
    rhs._depthStencilBuffer = nullptr;
    rhs._depthStencilView = nullptr;
  }
  DepthStencilBuffer& DepthStencilBuffer::operator=(DepthStencilBuffer&& rhs) noexcept {
    release();
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_DepthStencilBufferConfig));
    this->_depthStencilBuffer = rhs._depthStencilBuffer;
    this->_depthStencilView = rhs._depthStencilView;
    rhs._depthStencilBuffer = nullptr;
    rhs._depthStencilView = nullptr;
    return *this;
  }


// -----------------------------------------------------------------------------
// shader.h
// -----------------------------------------------------------------------------

// -- create/compile shaders -- ------------------------------------------------

  // Get D3D11 shader model ID
  static const char* __getShaderModel(pandora::video::ShaderType type) {
    switch (type) {
      case pandora::video::ShaderType::vertex:        return "vs_5_0"; break;
      case pandora::video::ShaderType::tesselControl: return "hs_5_0"; break;
      case pandora::video::ShaderType::tesselEval:    return "ds_5_0"; break;
      case pandora::video::ShaderType::geometry:      return "gs_5_0"; break;
      case pandora::video::ShaderType::fragment:      return "ps_5_0"; break;
      default: return "cs_5_0"; break;
    }
  }

  // Compile shader from text content
  Shader::Builder Shader::Builder::compile(pandora::video::ShaderType type, const char* textContent, size_t length, const char* entryPoint, bool isStrict) {
    ID3DBlob* errorMessage = nullptr;
    ID3DBlob* shaderBuffer = nullptr;
    const char* shaderModel = __getShaderModel(type);
    HRESULT result = D3DCompile((LPCVOID)textContent, (SIZE_T)length, nullptr, nullptr, nullptr, entryPoint, shaderModel, 
                                isStrict ? D3DCOMPILE_ENABLE_STRICTNESS : 0, 0, &shaderBuffer, &errorMessage);
    if (FAILED(result))
      throwShaderError(errorMessage, "Shader: compile error", shaderModel);
    return Shader::Builder(type, shaderBuffer);
  }
  // Compile shader from text file
  Shader::Builder Shader::Builder::compileFromFile(pandora::video::ShaderType type, const wchar_t* filePath, const char* entryPoint, bool isStrict) {
    
    ID3DBlob* errorMessage = nullptr;
    ID3DBlob* shaderBuffer = nullptr;
    const char* shaderModel = __getShaderModel(type);
    HRESULT result = D3DCompileFromFile(filePath, nullptr, nullptr, entryPoint, shaderModel, 
                                        isStrict ? D3DCOMPILE_ENABLE_STRICTNESS : 0, 0, &shaderBuffer, &errorMessage);
    if (FAILED(result))
      throwShaderError(errorMessage, "Shader: file/compile error", shaderModel);
    return Shader::Builder(type, shaderBuffer);
  }


// -- create shader objects -- -------------------------------------------------

  // Create shader object
  Shader Shader::Builder::createShader(Shader::DeviceHandle device) const {
    HRESULT result;
    Shader::Handle handle = nullptr;
    switch (this->_type) {
      case pandora::video::ShaderType::vertex:
        result = device->CreateVertexShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11VertexShader**)&handle);
        break;
      case pandora::video::ShaderType::tesselControl:
        result = device->CreateHullShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11HullShader**)&handle);
        break;
      case pandora::video::ShaderType::tesselEval:
        result = device->CreateDomainShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11DomainShader**)&handle);
        break;
      case pandora::video::ShaderType::geometry:
        result = device->CreateGeometryShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11GeometryShader**)&handle);
        break;
      case pandora::video::ShaderType::fragment:
        result = device->CreatePixelShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11PixelShader**)&handle);
        break;
      default:
        result = device->CreateComputeShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11ComputeShader**)&handle);
        break;
    }

    if (FAILED(result) || handle == nullptr)
      throwError(result, "Builder: shader create failed");
    return Shader(handle, this->_type);
  }
  
  // Destroy shader object
  void Shader::release() noexcept {
    if (this->_handle != nullptr) {
      try {
        switch (this->_type) {
          case pandora::video::ShaderType::vertex:        ((ID3D11VertexShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::tesselControl: ((ID3D11HullShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::tesselEval:    ((ID3D11DomainShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::geometry:      ((ID3D11GeometryShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::fragment:      ((ID3D11PixelShader*)this->_handle)->Release(); break;
          default: ((ID3D11ComputeShader*)this->_handle)->Release(); break;
        }
        this->_handle = nullptr;
      }
      catch (...) {}
    }
  }
  
  // ---

  // Create input layout for shader object
  ShaderInputLayout Shader::Builder::createInputLayout(Shader::DeviceHandle device, D3D11_INPUT_ELEMENT_DESC* layoutElements, size_t length) const {
    ID3D11InputLayout* inputLayout = nullptr;
    HRESULT result = device->CreateInputLayout((D3D11_INPUT_ELEMENT_DESC*)layoutElements, (UINT)length, 
                                                                (const void*)this->_data, (SIZE_T)this->_length, &inputLayout);
    if (FAILED(result) || inputLayout == nullptr)
      throwError(result, "Builder: layout create failed");
    return ShaderInputLayout((ShaderInputLayout::Handle)inputLayout);
  }
  
  
// -----------------------------------------------------------------------------
// camera.h
// -----------------------------------------------------------------------------

// -- camera projection -- -----------------------------------------------------

  // Compute shader projection matrix
  void CameraProjection::_computeProjection() noexcept {
    _constrainFieldOfView();
    float fovRad = DirectX::XMConvertToRadians(this->_fieldOfView);
    this->_projection = DirectX::XMMatrixPerspectiveFovLH(fovRad, this->_displayRatio, this->_nearPlane, this->_farPlane);
  }


// -- camera view helpers -- ---------------------------------------------------

//https://docs.microsoft.com/en-us/archive/msdn-magazine/2014/june/directx-factor-the-canvas-and-the-camera
//https://www.gamedev.net/tutorials/programming/graphics/directx-11-c-game-camera-r2978/
//https://github.com/Pindrought/DirectX-11-Engine-VS2017/tree/Tutorial_31/DirectX%2011%20Engine%20VS2017/DirectX%2011%20Engine%20VS2017/Graphics
//https://learnopengl.com/Getting-started/Camera

#endif
