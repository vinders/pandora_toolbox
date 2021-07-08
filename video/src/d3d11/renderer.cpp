/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Includes hpp implementations at the end of the file 
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <cstring>
# include <cmath>
# include <stdexcept>
# include <memory/light_string.h>

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
# include "video/d3d11/texture.h"
# include "video/d3d11/texture_reader.h"
# include "video/d3d11/texture_writer.h"
# include "video/d3d11/camera.h"

  using namespace pandora::video::d3d11;
  using namespace pandora::video;


// -----------------------------------------------------------------------------
// renderer.h
// -----------------------------------------------------------------------------

// -- display adapter detection -- ---------------------------------------------

  // Verify if a hardware adapter outputs on a monitor
  static bool __isHardwareAdapterForMonitor(IDXGIAdapter1* adapter, const pandora::memory::LightWString& targetMonitorId) {
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
    pandora::memory::LightWString targetDeviceString;
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
            if (target.attributes().id.empty()               // no target provided -> use first result
            || targetDeviceString == description.Description // adapter name available -> verify it
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
        if (target.attributes().id.empty()               // no target provided -> use first result
        || targetDeviceString == description.Description // adapter name available -> verify it
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
        throwError(result, "Renderer: DXGI creation error");
    }
    outDxgiFactory = dxgiFactory;
  }
  
  // Get current DXGI factory of device
  static inline void __getCurrentDxgiFactory(ID3D11Device* device, D3dResource<IDXGIFactory1>& outDxgiFactory) { // throws
    auto dxgiDevice = D3dResource<IDXGIDevice>::fromInterface(device, "Renderer: DXGI access error");
    D3dResource<IDXGIAdapter> adapter;
    auto result = dxgiDevice->GetAdapter(adapter.address());
    if (FAILED(result) || !adapter.hasValue())
      throwError(result, "Renderer: adapter access error");
    
    outDxgiFactory = D3dResource<IDXGIFactory1>::fromChild(adapter.get(), "Renderer: DXGI factory error");
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
        throw std::out_of_range("Renderer: no feat-levels");
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
          throw std::out_of_range("Renderer: no feat-levels/above max supported");
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
      throwError(result, "Renderer: device/context creation error"); // throws
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
    : _device(rhs._device),
      _context(rhs._context),
      _deviceLevel(rhs._deviceLevel),
      _dxgiFactory(rhs._dxgiFactory),
      _dxgiLevel(rhs._dxgiLevel) {
    rhs._dxgiFactory = nullptr;
    rhs._device = nullptr;
    rhs._context = nullptr;
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    _destroy();
    this->_device = rhs._device;
    this->_context = rhs._context;
    this->_deviceLevel = rhs._deviceLevel;
    this->_dxgiFactory = rhs._dxgiFactory;
    this->_dxgiLevel = rhs._dxgiLevel;
    rhs._device = nullptr;
    rhs._context = nullptr;
    rhs._dxgiFactory = nullptr;
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
  
  // Convert standard sRGB(A) color to device RGB(A)
  void Renderer::toGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept {
    DirectX::XMFLOAT3 colorArray(colorRgba);
    DirectX::XMStoreFloat3(&colorArray, DirectX::XMColorSRGBToRGB(DirectX::XMLoadFloat3(&colorArray))); // gamma-correct color
    outRgba[0] = colorArray.x;
    outRgba[1] = colorArray.y;
    outRgba[2] = colorArray.z;
    outRgba[3] = colorRgba[3];
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

  /// @brief Screen tearing supported (variable refresh rate display)
  bool Renderer::isTearingAvailable() const noexcept { 
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      if (this->_dxgiLevel >= 5u) {
        BOOL isSupported = FALSE;
        auto factoryV5 = D3dResource<IDXGIFactory5>::tryFromInterface((IDXGIFactory1*)this->_dxgiFactory);
        return (factoryV5 && SUCCEEDED(factoryV5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &isSupported, sizeof(isSupported))) 
             && isSupported != FALSE);
      }
#   endif
    return false;
  }
 

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
  void Renderer::clearViews(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer, 
                            const ColorChannel clearColorRgba[4]) noexcept {
    auto it = views;
    for (size_t i = 0; i < numberViews; ++i, ++it) {
      if (*it != nullptr)
        this->_context->ClearRenderTargetView(*it, clearColorRgba ? &clearColorRgba[0] : __defaultBlackColor);
    }
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    this->_context->Flush();
  }
  // Clear render-target content + depth buffer: reset to 'clearColorRgba' and to depth 1
  void Renderer::clearView(RenderTargetView view, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4]) noexcept {
    if (view != nullptr)
      this->_context->ClearRenderTargetView(view, clearColorRgba ? &clearColorRgba[0] : __defaultBlackColor);
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    this->_context->Flush();
  }

  // Bind/replace active render-target(s) in Renderer (multi-target)
  void Renderer::setActiveRenderTargets(RenderTargetView* views, size_t numberViews, DepthStencilView depthBuffer) noexcept {
    if (views != nullptr && numberViews > 0) { // set active view(s)
      this->_context->OMSetRenderTargets((UINT)numberViews, views, depthBuffer);
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      this->_context->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
    }
    this->_context->Flush();
  }
  
  // Bind/replace active render-target(s) in Renderer (multi-target) + clear render-target/buffer
  void Renderer::setCleanActiveRenderTargets(RenderTargetView* views, size_t numberViews, 
                                             DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4]) noexcept {
    if (views != nullptr && numberViews > 0) { // set active view(s)
      auto it = views;
      for (size_t i = 0; i < numberViews; ++i, ++it) {
        if (*it != nullptr)
          this->_context->ClearRenderTargetView(*it, clearColorRgba ? &clearColorRgba[0] : __defaultBlackColor);
      }
      if (depthBuffer != nullptr)
        this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
      this->_context->OMSetRenderTargets((UINT)numberViews, views, depthBuffer);
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      this->_context->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
    }
    this->_context->Flush();
  }

  // Bind/replace active render-target in Renderer (single target) + clear render-target/buffer
  void Renderer::setCleanActiveRenderTarget(RenderTargetView view, DepthStencilView depthBuffer, 
                                            const ColorChannel clearColorRgba[4]) noexcept {
    if (view != nullptr)
      this->_context->ClearRenderTargetView(view, clearColorRgba ? &clearColorRgba[0] : __defaultBlackColor);
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            
    this->_context->OMSetRenderTargets((UINT)1u, &view, depthBuffer);
    this->_context->Flush();
  }


// -- primitive binding -- -----------------------------------------------------

  // Set vertex patch topology for input stage (for vertex/tessellation shaders)
  void Renderer::setVertexPatchTopology(uint32_t controlPoints) noexcept {
    if (controlPoints != 0u) {
      --controlPoints;
      if (controlPoints >= 32u)
        controlPoints = 31u;
    }
    int topologyValue = (int)D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (int)controlPoints;
    this->_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topologyValue);
  }


// -----------------------------------------------------------------------------
// swap_chain.h
// -----------------------------------------------------------------------------

// -- color management --

# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
    // Find color space for a buffer format
    static __forceinline DXGI_COLOR_SPACE_TYPE __getColorSpace(DXGI_FORMAT backBufferFormat) noexcept {
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
  static __forceinline bool __setColorSpace(IDXGISwapChain* swapChain, DXGI_FORMAT backBufferFormat, 
                                            bool isHdrPreferred, DXGI_COLOR_SPACE_TYPE& outColorSpace) { // throws
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
            throwError(result, "SwapChain: color space error");
          return true;
        }
      }
#   endif
    return false;
  }


// -- swap-chain creation -- ---------------------------------------------------

  // Get swap-chain format (based on buffer format + flip-swap request) + detect flip-swap support for format
  // inOutUseFlipSwap: [in] = request flip-swap or not  /  [out] = requested and supported for format
  static inline DXGI_FORMAT __getSwapChainFormat(DXGI_FORMAT backBufferFormat, bool& inOutUseFlipSwap) noexcept {
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      if (inOutUseFlipSwap) {
        switch (backBufferFormat) {
          case DXGI_FORMAT_R16G16B16A16_FLOAT: 
          case DXGI_FORMAT_R8G8B8A8_UNORM:
          case DXGI_FORMAT_B8G8R8A8_UNORM:
          case DXGI_FORMAT_R10G10B10A2_UNORM:   inOutUseFlipSwap = true; return backBufferFormat;
          case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: inOutUseFlipSwap = true; return DXGI_FORMAT_R8G8B8A8_UNORM;
          case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: inOutUseFlipSwap = true; return DXGI_FORMAT_B8G8R8A8_UNORM;
          default: inOutUseFlipSwap = false; break;
        }
      }
#   endif
    return backBufferFormat;
  }
  
  // ---
  
  // Create DXGI swap-chain resource for existing renderer
  void SwapChain::_createSwapChain(pandora::video::WindowHandle window, const SwapChain::RefreshRate& rate) { // throws
    if (this->_renderer == nullptr || window == nullptr)
      throw std::invalid_argument("SwapChain: NULL renderer/window");
    __refreshDxgiFactory(this->_renderer->_dxgiFactory);

    // build swap-chain
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (this->_renderer->_dxgiLevel >= 2u) { // Direct3D 11.1+
        bool useFlipSwap = ( (this->_flags & (SwapChain::OutputFlag::disableFlipSwap 
                                            | SwapChain::OutputFlag::partialOutput)) == false  // * flip-swap not disabled + unique render-target view
                          && this->_renderer->isFlipSwapAvailable()                            // * supported by device/API/system
                          && ((this->_flags & SwapChain::OutputFlag::variableRefresh) == false // * no tearing or support with flip-swap
                            || this->_renderer->isTearingAvailable()) );

        DXGI_SWAP_CHAIN_DESC1 descriptor = {};
        ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC1));
        descriptor.Width = _width();
        descriptor.Height = _height();
        descriptor.Format = __getSwapChainFormat(this->_backBufferFormat, useFlipSwap); // * verify color format supported with flip-swap (or unset useFlipSwap)
        descriptor.BufferCount = (UINT)this->_framebufferCount;
        descriptor.BufferUsage = ((this->_flags & SwapChain::OutputFlag::shaderInput) == true)
                               ? (DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT) : DXGI_USAGE_RENDER_TARGET_OUTPUT;
        descriptor.Scaling = DXGI_SCALING_STRETCH;
        descriptor.SampleDesc.Count = 1;
        descriptor.Stereo = ((this->_flags & SwapChain::OutputFlag::stereo) == true) ? TRUE : FALSE;
        
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscnDescriptor = {};
        ZeroMemory(&fullscnDescriptor, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
        fullscnDescriptor.RefreshRate.Numerator = (UINT)rate.numerator();
        fullscnDescriptor.RefreshRate.Denominator = (UINT)rate.denominator();
        
        if (useFlipSwap) {
          descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
          if ((this->_flags & SwapChain::OutputFlag::variableRefresh) == true) {
            descriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            this->_tearingSwapFlags = (UINT)DXGI_PRESENT_ALLOW_TEARING;
          }
          if (descriptor.Stereo == FALSE)
            fullscnDescriptor.Windowed = TRUE;
        }
        else {
          this->_flags |= SwapChain::OutputFlag::disableFlipSwap; // mark as disabled
          descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
          if ((this->_flags & (SwapChain::OutputFlag::variableRefresh|SwapChain::OutputFlag::stereo)) == false)
            fullscnDescriptor.Windowed = TRUE;
        }

        auto dxgiFactoryV2 = D3dResource<IDXGIFactory2>::fromInterface((IDXGIFactory1*)this->_renderer->_dxgiFactory, "SwapChain: DXGI access error");
        auto deviceV1 = D3dResource<ID3D11Device1>::fromInterface(this->_renderer->_device, "SwapChain: device access error");
        auto result = dxgiFactoryV2->CreateSwapChainForHwnd(deviceV1.get(), (HWND)window, &descriptor, &fullscnDescriptor, nullptr, (IDXGISwapChain1**)&_swapChain);
        if (FAILED(result) || this->_swapChain == nullptr)
          throwError(result, "Renderer: swap-chain not created");
        this->_deviceContext11_1 = D3dResource<ID3D11DeviceContext1>::tryFromInterface(this->_renderer->context()).extract();
      }
      else
#   endif
    { // Direct3D 11.0
      this->_flags |= SwapChain::OutputFlag::disableFlipSwap; // no flip-swap in 11.0
    
      DXGI_SWAP_CHAIN_DESC descriptor = {};
      ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC));
      descriptor.BufferDesc.Width = _width();
      descriptor.BufferDesc.Height = _height();
      descriptor.BufferDesc.Format = this->_backBufferFormat;
      descriptor.BufferCount = (UINT)this->_framebufferCount;
      descriptor.BufferUsage = ((this->_flags & SwapChain::OutputFlag::shaderInput) == true)
                             ? (DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT) : DXGI_USAGE_RENDER_TARGET_OUTPUT;
      descriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
      descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      descriptor.SampleDesc.Count = 1;
      descriptor.OutputWindow = (HWND)window;
      descriptor.BufferDesc.RefreshRate.Numerator = (UINT)rate.numerator();
      descriptor.BufferDesc.RefreshRate.Denominator = (UINT)rate.denominator();
      if ((this->_flags & SwapChain::OutputFlag::variableRefresh) == false)
        descriptor.Windowed = TRUE;

      auto result = ((IDXGIFactory1*)this->_renderer->_dxgiFactory)->CreateSwapChain(this->_renderer->_device, &descriptor, (IDXGISwapChain**)&_swapChain);
      if (FAILED(result) || this->_swapChain == nullptr)
        throwError(result, "Renderer: swap-chain not created");
    }
    
    ((IDXGIFactory1*)this->_renderer->_dxgiFactory)->MakeWindowAssociation((HWND)window, DXGI_MWA_NO_ALT_ENTER);// no DXGI response to ALT+ENTER
  }
  
  // ---
  
  // Create/refresh swap-chain render-target view + color space
  void SwapChain::_createOrRefreshTargetView() { // throws
    // find + set color space value (if supported)
    __setColorSpace((IDXGISwapChain*)this->_swapChain, this->_backBufferFormat,
                    (this->_flags & SwapChain::OutputFlag::hdrPreferred) == true, this->_colorSpace);

    // create render-target view
    if (this->_renderTargetView == nullptr) {
      D3dResource<ID3D11Texture2D> renderTarget;
      auto targetResult = ((IDXGISwapChain*)this->_swapChain)->GetBuffer(0, IID_PPV_ARGS(renderTarget.address()));
      if (FAILED(targetResult) || !renderTarget.hasValue()) {
        throwError(targetResult, "SwapChain: render-target error"); return;
      }
      
      CD3D11_RENDER_TARGET_VIEW_DESC viewDescriptor(D3D11_RTV_DIMENSION_TEXTURE2D, this->_backBufferFormat);
      targetResult = this->_renderer->device()->CreateRenderTargetView(renderTarget.get(), &viewDescriptor, &(this->_renderTargetView));
      if (FAILED(targetResult) || this->_renderTargetView == nullptr)
        throwError(targetResult, "SwapChain: view not created");
    }
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
            ((ID3D11DeviceContext1*)this->_deviceContext11_1)->Release();
#       endif
      } 
      catch (...) {}
        
      try {
        if (this->_renderer->_dxgiLevel >= 2u) {
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
    : _swapChain(rhs._swapChain),
      _tearingSwapFlags(rhs._tearingSwapFlags),
      _pixelSize(rhs._pixelSize),
      _framebufferCount(rhs._framebufferCount),
      _flags(rhs._flags),
      _backBufferFormat(rhs._backBufferFormat),
      _colorSpace(rhs._colorSpace),
      _renderer(std::move(rhs._renderer)),
      _renderTargetView(rhs._renderTargetView)
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        , _deviceContext11_1(rhs._deviceContext11_1)
#     endif
  {
    rhs._swapChain = nullptr;
    rhs._renderer = nullptr;
    rhs._renderTargetView = nullptr;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      rhs._deviceContext11_1 = nullptr;
#   endif
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    release();
    this->_swapChain = rhs._swapChain;
    this->_tearingSwapFlags = rhs._tearingSwapFlags;
    this->_pixelSize = rhs._pixelSize;
    this->_framebufferCount = rhs._framebufferCount;
    this->_flags = rhs._flags;
    this->_backBufferFormat = rhs._backBufferFormat;
    this->_colorSpace = rhs._colorSpace;
    this->_renderer = std::move(rhs._renderer);
    this->_renderTargetView = rhs._renderTargetView;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      this->_deviceContext11_1 = rhs._deviceContext11_1;
#   endif
    rhs._swapChain = nullptr;
    rhs._renderer = nullptr;
    rhs._renderTargetView = nullptr;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      rhs._deviceContext11_1 = nullptr;
#   endif
    return *this;
  }


// -- operations -- ------------------------------------------------------------

  // Change back-buffer(s) size + refresh color space
  bool SwapChain::resize(uint32_t width, uint32_t height) { // throws
    uint32_t pixelSize = _toPixelSize(width, height);
    bool isResized = (this->_pixelSize != pixelSize);
  
    if (isResized) {
      // clear previous size-specific context
      this->_renderer->setActiveRenderTarget(nullptr);
      this->_renderTargetView->Release();
      this->_renderTargetView = nullptr; // set to NULL -> tells _createOrRefreshTargetView to re-create it
      ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(FALSE, nullptr);
      this->_renderer->context()->Flush();
      
      // resize swap-chain
      DXGI_SWAP_CHAIN_FLAG flags = (DXGI_SWAP_CHAIN_FLAG)0;
      if ((this->_flags & (SwapChain::OutputFlag::variableRefresh|SwapChain::OutputFlag::disableFlipSwap)) == SwapChain::OutputFlag::variableRefresh)
        flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
      bool useFlipSwap = ((this->_flags & SwapChain::OutputFlag::disableFlipSwap) == false);
      DXGI_FORMAT swapChainFormat = __getSwapChainFormat(this->_backBufferFormat, useFlipSwap);
      
      auto result = ((IDXGISwapChain*)this->_swapChain)->ResizeBuffers(this->_framebufferCount, width, height, swapChainFormat, flags);
      if (FAILED(result)) {
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_HUNG)
          throw std::domain_error("Adapter changed: Renderer/SwapChain must be recreated");
        throwError(result, "SwapChain: resize error");
      }
      this->_pixelSize = pixelSize;
      
      // fullscreen required: tearing without flip-swap / stereo rendering
      if ((this->_flags & SwapChain::OutputFlag::stereo) == true || flags == DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
        ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(TRUE, nullptr);
    }

    // create/refresh render-target-view (if not resized, will only verify color space (in case monitor changed))
    _createOrRefreshTargetView();
    return isResized;
  }
  
  // ---
  
  // Throw appropriate exception for 'swap buffers' error
  static void __processSwapError(HRESULT result) {
    switch (result) {
      // minor issues -> ignore
      case DXGI_ERROR_WAS_STILL_DRAWING:
      case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
      case DXGI_ERROR_NONEXCLUSIVE:
      case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: break;
      // device lost
      case DXGI_ERROR_DEVICE_REMOVED:
      case DXGI_ERROR_DEVICE_RESET: throw std::domain_error("SwapChain: device lost");
      // invalid option / internal error
      default: throwError(result, "SwapChain: internal error"); break;
    }
  }

  // Swap back-buffer(s) and front-buffer, to display drawn content on screen
  void SwapChain::swapBuffers(bool useVsync) {
    auto result = useVsync
                ? ((IDXGISwapChain*)this->_swapChain)->Present(1, 0) // no screen tearing with vsync
                : ((IDXGISwapChain*)this->_swapChain)->Present(0, this->_tearingSwapFlags);
    if (FAILED(result))
      __processSwapError(result);
    __refreshDxgiFactory(this->_renderer->_dxgiFactory);
  }
  
  // Swap back-buffer(s) and front-buffer, to display drawn content on screen + discard render-target content after it
  void SwapChain::swapBuffersDiscard(bool useVsync, DepthStencilView depthBuffer) {
    auto result = useVsync
                ? ((IDXGISwapChain*)this->_swapChain)->Present(1, 0) // no screen tearing with vsync
                : ((IDXGISwapChain*)this->_swapChain)->Present(0, this->_tearingSwapFlags);
    if (FAILED(result))
      __processSwapError(result);
    __refreshDxgiFactory(this->_renderer->_dxgiFactory);
    
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      // discard content of render target + depth/stencil buffer
      if (this->_deviceContext11_1) {
        ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView(this->_renderTargetView);
        if (depthBuffer != nullptr)
          ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView(depthBuffer);
      }
#   endif
  }


// -----------------------------------------------------------------------------
// _d3d_resource.h -- error messages
// -----------------------------------------------------------------------------

  // Exception class with LightString
  class RuntimeException final : public std::runtime_error {
  public:
    RuntimeException(std::shared_ptr<pandora::memory::LightString>&& msg) noexcept : std::runtime_error(msg->c_str()), _message(std::move(msg)) {}
    RuntimeException(const RuntimeException& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException(RuntimeException&& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException& operator=(const RuntimeException& rhs) noexcept { std::runtime_error::operator=(rhs); _message = rhs._message; return *this; }
    virtual ~RuntimeException() noexcept { _message.reset(); }
  private:
    std::shared_ptr<pandora::memory::LightString> _message;
  };
  
  // ---
  
  static inline const char* __getDirect3DError(HRESULT result) {
    switch (result) {
      case E_UNEXPECTED: return "UNEXPECTED";
      case E_NOTIMPL: return "NOTIMPL";
      case E_OUTOFMEMORY: return "OUTOFMEM";
      case DXGI_ERROR_INVALID_CALL:
      case E_INVALIDARG: return "INVALIDARG";
      case E_NOINTERFACE: return "NOINTERFACE";
      case E_POINTER: return "POINTER";
      case E_HANDLE: return "HANDLE";
      case E_ABORT: return "ABORT";
      case E_FAIL: return "FAIL";
      case DXGI_ERROR_ACCESS_DENIED:
      case E_ACCESSDENIED: return "ACCESSDENIED";
      case DXGI_ERROR_ACCESS_LOST: return "ACCESS_LOST";
      case DXGI_ERROR_NAME_ALREADY_EXISTS:
      case DXGI_ERROR_ALREADY_EXISTS: return "ALREADY_EXISTS";
      case E_PENDING: return "PENDING";
      case E_BOUNDS: return "BOUNDS";
      case E_CHANGED_STATE: return "CHANGED_STATE";
      case E_ILLEGAL_STATE_CHANGE: return "ILLEGAL_STATE_CHANGE";
      case E_ILLEGAL_METHOD_CALL: return "ILLEGAL_METHOD_CALL";
      case RO_E_METADATA_NAME_NOT_FOUND: return "META_NAME_NOT_FOUND";
      case RO_E_METADATA_NAME_IS_NAMESPACE: return "META_NAME_IS_NAMESPACE";
      case RO_E_METADATA_INVALID_TYPE_FORMAT: return "META_INVALID_TYPE";
      case RO_E_INVALID_METADATA_FILE: return "INVALID_META_FILE";
      case RO_E_CLOSED: return "CLOSED";
      case RO_E_EXCLUSIVE_WRITE: return "EXCLUSIVE_WRITE";
      case RO_E_ERROR_STRING_NOT_FOUND: return "ERROR_STR_NOT_FOUND";
      case E_STRING_NOT_NULL_TERMINATED: return "STR_NOT_NULL_TERM";
      case E_ILLEGAL_DELEGATE_ASSIGNMENT: return "ILLEGAL_DELEGATE_ASSIGN";
      case E_ASYNC_OPERATION_NOT_STARTED: return "ASYNC_OP_NOT_STARTED";
      case CO_E_INIT_ONLY_SINGLE_THREADED: return "INIT_ONLY_SINGLE_THREADED";
      case CO_E_CANT_REMOTE: return "CANT_REMOTE";
      case CO_E_LAUNCH_PERMSSION_DENIED: return "LAUNCH_PERMSSION_DENIED";
      case CO_E_REMOTE_COMMUNICATION_FAILURE: return "REMOTE_COMM_FAILURE";
      case CO_E_IIDREG_INCONSISTENT: return "IIDREG_INCONSIST";
      case DXGI_ERROR_UNSUPPORTED:
      case CO_E_NOT_SUPPORTED: return "NOT_SUPPORTED";
      case CO_E_RELOAD_DLL: return "RELOAD_DLL";
      default: return "INTERNAL_ERROR";
    }
  }

  void pandora::video::d3d11::throwError(HRESULT result, const char* messageContent) {
    const char* d3dError = __getDirect3DError(result);

    // pre-compute total size to avoid having multiple dynamic allocs
    size_t prefixSize = strlen(messageContent);
    size_t errorSize = strlen(d3dError);
    auto message = std::make_shared<pandora::memory::LightString>(prefixSize + 2u + errorSize);
    
    // copy message in preallocated string
    if (!message->empty()) { // if no alloc failure
      memcpy((void*)message->data(),                 messageContent, prefixSize*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize]),    ": ",         size_t{2u}*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize+2u]), d3dError,     errorSize *sizeof(char));
    }
    throw RuntimeException(std::move(message));
  }
  
  void pandora::video::d3d11::throwShaderError(ID3DBlob* errorMessage, const char* messagePrefix, const char* shaderInfo) {
    const char* errorData = (errorMessage) ? (const char*)errorMessage->GetBufferPointer() : "missing/empty shader file/content";
    
    // pre-compute total size to avoid having multiple dynamic allocs
    size_t prefixSize = strlen(messagePrefix);
    size_t infoSize = strlen(shaderInfo);
    size_t errorSize = strlen(errorData);
    auto message = std::make_shared<pandora::memory::LightString>(prefixSize + 2u + infoSize + 3u + errorSize);
    
    // copy message in preallocated string
    if (!message->empty()) { // if no alloc failure
      memcpy((void*)message->data(),                           messagePrefix, prefixSize*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize]),             " (",         size_t{2u}*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize + 2u]),        shaderInfo,   infoSize  *sizeof(char));
      memcpy((void*)&(message->data()[prefixSize+infoSize+2u]), "): ",        size_t{3u}*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize+infoSize+2u+3u]), errorData, errorSize *sizeof(char));
    }
    if (errorMessage)
      errorMessage->Release();
    throw RuntimeException(std::move(message));
  }
  

// -----------------------------------------------------------------------------
// Include hpp implementations
// -----------------------------------------------------------------------------
# include "./renderer_state_factory.hpp"
# include "./buffers.hpp"
# include "./texture.hpp"
# include "./texture_reader_writer.hpp"
# include "./shader.hpp"
# include "./camera.hpp"

#endif
