/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <cstring>
# include <string>
# include <stdexcept>
# include "video/d3d11/renderer.h"

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
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
# include "video/d3d11/_private/_d3d_resource.h"

  using namespace pandora::video::d3d11;
  using namespace pandora::video;


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
        throwError(result, "Renderer: failed to create DirectX graphics infrastructure");
    }
    outDxgiFactory = dxgiFactory;
  }
  
  // Get current DXGI factory of device
  static inline void __getCurrentDxgiFactory(ID3D11Device* device, D3dResource<IDXGIFactory1>& outDxgiFactory) { // throws
    auto dxgiDevice = D3dResource<IDXGIDevice>::fromInterface(device, "Renderer: failed to access DirectX graphics device");
    D3dResource<IDXGIAdapter> adapter;
    auto result = dxgiDevice->GetAdapter(adapter.address());
    if (FAILED(result) || !adapter.hasValue())
      throwError(result, "Renderer: failed to obtain Direct3D device adapter");
    
    outDxgiFactory = D3dResource<IDXGIFactory1>::fromChild(adapter.get(), "Renderer: failed to obtain DirectX graphics infra");
  }
  
  // If output information on DXGI factory is stale, try to create a new one
  void Renderer::_refreshDxgiFactory() { // throws
    if (!((IDXGIFactory1*)this->_dxgiFactory)->IsCurrent()) {
      D3dResource<IDXGIFactory1> newFactory;
      __createDxgiFactory(newFactory);
      this->_dxgiFactory = (void*)newFactory.extract();
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
  Renderer::Renderer(const pandora::hardware::DisplayMonitor& monitor, DeviceLevel minLevel) { // throws
    UINT runtimeLayers = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#   ifdef _DEBUG
      if (__isDebugSdkAvailable())
        runtimeLayers |= D3D11_CREATE_DEVICE_DEBUG;
#   endif

    // list supported feature levels
    const D3D_FEATURE_LEVEL featureLevels[] = {
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        D3D_FEATURE_LEVEL_11_1,
#     endif
      D3D_FEATURE_LEVEL_11_0
    };
    UINT featureLevelCount = (UINT)_countof(featureLevels);
    while (featureLevelCount && (uint32_t)minLevel) {
      --featureLevelCount;
      minLevel = static_cast<DeviceLevel>((uint32_t)minLevel - 1u);
    }
    if (featureLevelCount == 0)
      throw std::out_of_range("Renderer: minimum feature level requested is higher than max");

    // create DXGI factory + get primary adapter (if not found, default adapter will be used instead)
    D3dResource<IDXGIFactory1> dxgiFactory;
    __createDxgiFactory(dxgiFactory); // throws
    D3dResource<IDXGIAdapter1> adapter = __getHardwareAdapter(dxgiFactory.get(), monitor); // may be NULL (not found)
    bool isDefaultAdapter = !adapter.hasValue();

    // create rendering device + context
    D3D_FEATURE_LEVEL deviceLevel;
    auto result = D3D11CreateDevice(adapter.get(), (adapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, 
                                    nullptr, runtimeLayers, featureLevels, featureLevelCount, D3D11_SDK_VERSION, 
                                    (ID3D11Device**)&(this->_device), &deviceLevel, 
                                    (ID3D11DeviceContext**)&(this->_context));
    if (FAILED(result) || this->_device == nullptr || this->_context == nullptr)
      throwError(result, "Renderer: failed to create device and context"); // throws
    if (isDefaultAdapter && !dxgiFactory->IsCurrent()) // if adapter not provided, system may generate another factory
      __getCurrentDxgiFactory((ID3D11Device*)this->_device, dxgiFactory); // throws
    
    // feature level detection
    this->_dxgiLevel = __getDxgiFactoryLevel(dxgiFactory.get());
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (deviceLevel != D3D_FEATURE_LEVEL_11_0 && this->_dxgiLevel >= 2u)
        this->_deviceLevel = Renderer::DeviceLevel::direct3D_11_1;
      else {
        this->_deviceLevel = Renderer::DeviceLevel::direct3D_11_0;
        this->_dxgiLevel = 1u;
      }
#   else
      this->_deviceLevel = Renderer::DeviceLevel::direct3D_11_0;
      this->_dxgiLevel = 1u;
#   endif
    this->_dxgiFactory = (void*)dxgiFactory.extract();

#   ifdef _DEBUG
      __configureDeviceDebug((ID3D11Device*)this->_device);
#   endif
  }


// -- device/context destruction/move -- ---------------------------------------

  // Destroy device and context resources
  void Renderer::_destroy() noexcept {
    try {
      // release device context
      if (this->_context) {
        ((ID3D11DeviceContext*)this->_context)->Flush();
        ((ID3D11DeviceContext*)this->_context)->Release();
      }
      // release device
      if (this->_device)
        ((ID3D11Device*)this->_device)->Release();
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
    rhs._dxgiFactory = rhs._device = rhs._context = nullptr;
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    _destroy();
    this->_dxgiFactory = rhs._dxgiFactory;
    this->_device = rhs._device;
    this->_context = rhs._context;
    this->_deviceLevel = rhs._deviceLevel;
    this->_dxgiLevel = rhs._dxgiLevel;
    rhs._dxgiFactory = rhs._device = rhs._context = nullptr;
    return *this;
  }


// -- accessors -- -------------------------------------------------------------

  // Max number of simultaneous render views (swap-chains, texture targets...)
  size_t Renderer::maxSimultaneousRenderViews() noexcept {
    return (size_t)D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
  }
  
  // Read device adapter VRAM size
  bool Renderer::getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept {
    bool isSuccess = false;
    try {
      auto dxgiDevice = D3dResource<IDXGIDevice>::tryFromInterface((ID3D11Device*)this->_device);
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
        auto dxgiDevice = D3dResource<IDXGIDevice>::tryFromInterface((ID3D11Device*)this->_device);
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
  
  // Verify if a multisample mode is supported (MSAA) + get max quality level
  bool Renderer::_isMultisampleSupported(uint32_t sampleCount, int32_t componentFormat, uint32_t& outMaxQualityLevel) const noexcept {
    try {
      UINT qualityLevel = 0;
      if (SUCCEEDED(((ID3D11Device*)this->_device)->CheckMultisampleQualityLevels((DXGI_FORMAT)componentFormat, (UINT)sampleCount, &qualityLevel))) {
        outMaxQualityLevel = (uint32_t)qualityLevel;
        return (qualityLevel > 0);
      }
    }
    catch (...) {}
    return false;
  }


// -- resource builder -- ------------------------------------------------------

  // Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
  RasterizerState Renderer::createRasterizerState(CullMode culling, bool isFrontClockwise, 
                                                  const pandora::video::DepthBias& depth, bool useMsaa,
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

    rasterizerState.MultisampleEnable = useMsaa ? TRUE : FALSE;
    rasterizerState.AntialiasedLineEnable = useMsaa ? TRUE : FALSE;
    rasterizerState.ScissorEnable = scissorClipping ? TRUE : FALSE;

    ID3D11RasterizerState* stateData = nullptr;
    auto result = ((ID3D11Device*)this->_device)->CreateRasterizerState(&rasterizerState, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Renderer: failed to create rasterizer state");
    return RasterizerState(stateData);
  }

  // ---

  // Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  void Renderer::createFilterState(FilterStates& outStateContainer, int32_t index) { // throws
    if (outStateContainer.size() >= outStateContainer.maxSize())
      throw std::out_of_range("Renderer: sampler/filter state container is already full");

    D3D11_SAMPLER_DESC samplerDesc = { }; // TODO: params to customize...
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 8;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = 0;

    ID3D11SamplerState* stateData = nullptr;
    auto result = ((ID3D11Device*)this->_device)->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Renderer: failed to create sampler/filter state");

    bool isValidSlot = (index < 0)
                     ? outStateContainer.append((FilterStates::State)stateData)
                     : outStateContainer.insert(index, (FilterStates::State)stateData);
    if (!isValidSlot) {
      stateData->Release();
      throw std::out_of_range("Renderer: sampler/filter state index out of range");
    }
  }

  // Max array size for sample filters
  size_t Renderer::maxFilterStates() const noexcept { return (size_t)D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; }


// -- status operations -- -----------------------------------------------------

  // Change device rasterizer mode (culling, clipping, depth-bias, multisample, wireframe...)
  void Renderer::setRasterizerState(const RasterizerState& state) noexcept {
    ((ID3D11DeviceContext*)this->_context)->RSSetState((ID3D11RasterizerState*)state.get());
  }
  
  // Set array of sampler filters to the fragment/pixel shader stage
  void Renderer::setFilterStates(uint32_t firstIndex, const FilterStates::State* states, size_t length) noexcept {
    if (firstIndex >= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT)
      return;
    UINT arraySize = (firstIndex + (uint32_t)length <= (uint32_t)D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) 
                   ? (UINT)length 
                   : D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - firstIndex;

    if (states != nullptr)
      ((ID3D11DeviceContext*)this->_context)->PSSetSamplers((UINT)firstIndex, arraySize, (ID3D11SamplerState**)states);
    else {
      ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
      ((ID3D11DeviceContext*)this->_context)->PSSetSamplers((UINT)firstIndex, arraySize, &empty[0]);
    }
  }
  // Reset all sampler filters
  void Renderer::clearFilterStates() noexcept {
    ID3D11SamplerState* empty[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] { nullptr };
    ((ID3D11DeviceContext*)this->_context)->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, &empty[0]);
  }
  

// -- render target operations -------------------------------------------------
  
  /// @brief Clear render-targets + depth buffer: reset to 'clearColorRgba' and to depth 1
  void Renderer::clear(RenderTargetViewHandle* views, size_t numberViews, DepthStencilViewHandle depthBuffer, 
                       const ComponentVector128& clearColorRgba) noexcept {
    DirectX::XMVECTORF32 color;
    color.v = DirectX::XMColorSRGBToRGB((DirectX::XMVECTORF32&)clearColorRgba); // gamma-correct color
      
    auto it = views;
    for (size_t i = 0; i < numberViews; ++i, ++it) {
      if (*it != nullptr)
        ((ID3D11DeviceContext*)this->_context)->ClearRenderTargetView((ID3D11RenderTargetView*)*it, color);
    }
    if (depthBuffer != nullptr)
      ((ID3D11DeviceContext*)this->_context)->ClearDepthStencilView((ID3D11DepthStencilView*)depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ((ID3D11DeviceContext*)this->_context)->Flush();
  }
  /// @brief Clear render-target + depth buffer: reset to 'clearColorRgba' and to depth 1
  void Renderer::clear(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer, const ComponentVector128& clearColorRgba) noexcept {
    DirectX::XMVECTORF32 color;
    color.v = DirectX::XMColorSRGBToRGB((DirectX::XMVECTORF32&)clearColorRgba); // gamma-correct color
    
    if (view != nullptr)
      ((ID3D11DeviceContext*)this->_context)->ClearRenderTargetView((ID3D11RenderTargetView*)view, color);
    if (depthBuffer != nullptr)
      ((ID3D11DeviceContext*)this->_context)->ClearDepthStencilView((ID3D11DepthStencilView*)depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ((ID3D11DeviceContext*)this->_context)->Flush();
  }
  
  // ---
  
  // Bind/replace active render-target(s) in Renderer (multi-target)
  void Renderer::setActiveRenderTargets(RenderTargetViewHandle* views, size_t numberViews, 
                                        DepthStencilViewHandle depthBuffer) noexcept {
    if (views != nullptr && numberViews > 0) { // set active view(s)
      ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets((UINT)numberViews, (ID3D11RenderTargetView**)views, 
                                                                 (ID3D11DepthStencilView*)depthBuffer);
      ((ID3D11DeviceContext*)this->_context)->Flush();
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
      ((ID3D11DeviceContext*)this->_context)->Flush();
    }
  }
  
  // Bind/replace active render-target(s) in Renderer (multi-target) + clear render-target/buffer
  void Renderer::setActiveRenderTargets(RenderTargetViewHandle* views, size_t numberViews, 
                                        DepthStencilViewHandle depthBuffer, const ComponentVector128& clearColorRgba) noexcept {
    if (views != nullptr && numberViews > 0) { // set active view(s)
      DirectX::XMVECTORF32 color;
      color.v = DirectX::XMColorSRGBToRGB((DirectX::XMVECTORF32&)clearColorRgba); // gamma-correct color

      auto it = views;
      for (size_t i = 0; i < numberViews; ++i, ++it) {
        if (*it != nullptr)
          ((ID3D11DeviceContext*)this->_context)->ClearRenderTargetView((ID3D11RenderTargetView*)*it, color);
      }
      if (depthBuffer != nullptr)
        ((ID3D11DeviceContext*)this->_context)->ClearDepthStencilView((ID3D11DepthStencilView*)depthBuffer, 
                                                                      D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
      ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets((UINT)numberViews, (ID3D11RenderTargetView**)views,
                                                                 (ID3D11DepthStencilView*)depthBuffer);
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
      ((ID3D11DeviceContext*)this->_context)->Flush();
    }
  }
  // Bind/replace active render-target in Renderer (multi-target) + clear render-target/buffer
  void Renderer::setActiveRenderTarget(RenderTargetViewHandle view, DepthStencilViewHandle depthBuffer, 
                                       const ComponentVector128& clearColorRgba) noexcept {
    if (view != nullptr) { // set active view
      DirectX::XMVECTORF32 color;
      color.v = DirectX::XMColorSRGBToRGB((DirectX::XMVECTORF32&)clearColorRgba); // gamma-correct color
      
      ((ID3D11DeviceContext*)this->_context)->ClearRenderTargetView((ID3D11RenderTargetView*)view, color);
      if (depthBuffer != nullptr)
        ((ID3D11DeviceContext*)this->_context)->ClearDepthStencilView((ID3D11DepthStencilView*)depthBuffer, 
                                                                      D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
      ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets((UINT)1u, (ID3D11RenderTargetView**)&view,
                                                                 (ID3D11DepthStencilView*)depthBuffer);
    }
    else { // clear active views
      ID3D11RenderTargetView* emptyViews[] { nullptr };
      ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets(_countof(emptyViews), emptyViews, nullptr);
      ((ID3D11DeviceContext*)this->_context)->Flush();
    }
  }


// -- swap-chain creation -- ---------------------------------------------------

  // Create DXGI swap-chain resource
  void* Renderer::_createSwapChain(const _SwapChainConfig& config, pandora::video::WindowHandle window,
                                   uint32_t rateNumerator, uint32_t rateDenominator, 
                                   DeviceLevel& outSwapChainLevel) { // throws
    void* swapChain = nullptr;
    _refreshDxgiFactory();
    
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (this->_dxgiLevel >= 2u) {
        // Direct3D 11.1+
        auto dxgiFactoryV2 = D3dResource<IDXGIFactory2>::fromInterface((IDXGIFactory1*)this->_dxgiFactory, "Renderer: failed to access DirectX 11.1 graphics infra");
        auto deviceV1 = D3dResource<ID3D11Device1>::fromInterface((ID3D11Device*)this->_device, "Renderer: failed to access DirectX 11.1 device");
        outSwapChainLevel = Renderer::DeviceLevel::direct3D_11_1;
        
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscnDescriptor = {};
        ZeroMemory(&fullscnDescriptor, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
        DXGI_SWAP_CHAIN_DESC1 descriptor = {};
        ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC1));
        fullscnDescriptor.Windowed = ((config.flags & SwapChainOutputFlag::stereo) == true) ? FALSE : TRUE;
        descriptor.Width = config.width;
        descriptor.Height = config.height;
        descriptor.Format = (DXGI_FORMAT)config.swapChainFormat;
        descriptor.BufferCount = (UINT)config.frameBufferCount;
        descriptor.BufferUsage = (DXGI_USAGE)config.bufferUsageMode;
        descriptor.Scaling = DXGI_SCALING_STRETCH;
        fullscnDescriptor.RefreshRate.Numerator = (UINT)rateNumerator;
        fullscnDescriptor.RefreshRate.Denominator = (UINT)rateDenominator;
        descriptor.SwapEffect = (config.useFlipSwap) ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
        descriptor.SampleDesc.Count = config.msaaSampleNumber;
        descriptor.SampleDesc.Quality = config.msaaQualityLevel - 1;
        descriptor.Stereo = ((config.flags & SwapChainOutputFlag::stereo) == true) ? TRUE : FALSE;
        descriptor.Flags = (config.useFlipSwap && (config.flags & SwapChainOutputFlag::variableRefresh) == true) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        if ((config.flags & SwapChainOutputFlag::localOutput) == true)
          descriptor.Flags |= DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;

        auto result = dxgiFactoryV2->CreateSwapChainForHwnd(deviceV1.get(), (HWND)window, &descriptor, &fullscnDescriptor, nullptr, (IDXGISwapChain1**)&swapChain);
        if (FAILED(result) || swapChain == nullptr)
          throwError(result, "Renderer: failed to create Direct3D 11.1 swap-chain");
      }
      else
#   endif
    {
      // Direct3D 11.0
      outSwapChainLevel = Renderer::DeviceLevel::direct3D_11_0;
      DXGI_SWAP_CHAIN_DESC descriptor = {};
      ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC));
      descriptor.BufferDesc.Width = config.width;
      descriptor.BufferDesc.Height = config.height;
      descriptor.BufferDesc.Format = (DXGI_FORMAT)config.swapChainFormat;
      descriptor.BufferCount = (UINT)config.frameBufferCount;
      descriptor.BufferUsage = (DXGI_USAGE)config.bufferUsageMode;
      descriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
      descriptor.BufferDesc.RefreshRate.Numerator = (UINT)rateNumerator;
      descriptor.BufferDesc.RefreshRate.Denominator = (UINT)rateDenominator;
      descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      descriptor.OutputWindow = (HWND)window;
      descriptor.Windowed = TRUE;
      descriptor.SampleDesc.Count = config.msaaSampleNumber;
      descriptor.SampleDesc.Quality = config.msaaQualityLevel - 1;

      auto result = ((IDXGIFactory1*)this->_dxgiFactory)->CreateSwapChain((ID3D11Device*)this->_device, &descriptor, (IDXGISwapChain**)&swapChain);
      if (FAILED(result) || swapChain == nullptr)
        throwError(result, "Renderer: failed to create Direct3D swap-chain");
    }
    
    ((IDXGIFactory1*)this->_dxgiFactory)->MakeWindowAssociation((HWND)window, DXGI_MWA_NO_ALT_ENTER); // prevent DXGI from responding to the ALT+ENTER shortcut
    if (!config.useFlipSwap && (config.flags & SwapChainOutputFlag::variableRefresh) == true)
      ((IDXGISwapChain*)swapChain)->SetFullscreenState(TRUE, nullptr); // without flip-swap, screen tearing requires fullscreen state
    
    return swapChain;
  }


// -- rasterizer state container -- --------------------------------------------

  RasterizerState::~RasterizerState() noexcept {
    if (this->_state != nullptr)
      ((ID3D11RasterizerState*)this->_state)->Release();
  }


// -- sampler filter state container -- ----------------------------------------
  
  FilterStates::FilterStates() {
    this->_states = (FilterStates::State*)calloc(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, sizeof(ID3D11SamplerState*));
    if (this->_states == nullptr)
      throw std::bad_alloc();
  }
  FilterStates::~FilterStates() noexcept {
    if (this->_states) {
      clear();
      free(this->_states);
    }
  }
  
  // ---
  
  size_t FilterStates::maxSize() noexcept {
    return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
  }
  
  // ---
  
  bool FilterStates::insert(uint32_t index, FilterStates::State state) noexcept {
    if (this->_length >= maxSize())
      return false;
    
    if (index < (uint32_t)this->_length) {
      memmove((void*)&_states[index + 1], (void*)&_states[index], (this->_length - (size_t)index)*sizeof(ID3D11SamplerState*));
      this->_states[index] = state;
    }
    else if (index == (uint32_t)this->_length) {
      this->_states[this->_length] = state;
    }
    else
      return false;

    ++(this->_length);
    return true;
  }
  void FilterStates::erase(uint32_t index) noexcept {
    if (index < this->_length) {
      if (this->_states[index] != nullptr)
        ((ID3D11SamplerState*)this->_states[index])->Release();
      
      if (index + 1 < (uint32_t)this->_length)
        memmove((void*)&_states[index], (void*)&_states[index + 1], (this->_length - (size_t)index - 1u) * sizeof(ID3D11SamplerState*));
      --(this->_length);
      this->_states[this->_length] = nullptr;
    }
  }
  void FilterStates::clear() noexcept {
    auto it = this->_states;
    for (size_t i = 0; i < this->_length; ++i, ++it) {
      if (*it != nullptr) {
        ((ID3D11SamplerState*)(*it))->Release();
        *it = nullptr;
      }
    }
    this->_length = 0;
  }


// -- buffer format bindings -- ------------------------------------------------

  // Convert portable component format to DXGI_FORMAT
  int32_t Renderer::_toDxgiFormat(ComponentFormat format) noexcept {
    switch (format) {
      case ComponentFormat::rgba8_sRGB: return (int32_t)DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      case ComponentFormat::rgba8_unorm: return (int32_t)DXGI_FORMAT_R8G8B8A8_UNORM;
      case ComponentFormat::rgba8_snorm: return (int32_t)DXGI_FORMAT_R8G8B8A8_SNORM;
      case ComponentFormat::rgba8_ui: return (int32_t)DXGI_FORMAT_R8G8B8A8_UINT;
      case ComponentFormat::rgba8_i: return (int32_t)DXGI_FORMAT_R8G8B8A8_SINT;
      
      case ComponentFormat::d32_f: return (int32_t)DXGI_FORMAT_D32_FLOAT;
      case ComponentFormat::d24_unorm: return (int32_t)DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      case ComponentFormat::d16_unorm: return (int32_t)DXGI_FORMAT_D16_UNORM;
      case ComponentFormat::d32_f_s8_ui: return (int32_t)DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
      case ComponentFormat::d24_unorm_s8_ui: return (int32_t)DXGI_FORMAT_D24_UNORM_S8_UINT;
      case ComponentFormat::s8_ui: return (int32_t)DXGI_FORMAT_X24_TYPELESS_G8_UINT;
      
      case ComponentFormat::rgba16_f_hdr_scRGB: return (int32_t)DXGI_FORMAT_R16G16B16A16_FLOAT;
      case ComponentFormat::rgba16_unorm: return (int32_t)DXGI_FORMAT_R16G16B16A16_UNORM;
      case ComponentFormat::rgba16_snorm: return (int32_t)DXGI_FORMAT_R16G16B16A16_SNORM;
      case ComponentFormat::rgba16_ui: return (int32_t)DXGI_FORMAT_R16G16B16A16_UINT;
      case ComponentFormat::rgba16_i: return (int32_t)DXGI_FORMAT_R16G16B16A16_SINT;
      case ComponentFormat::rgb10a2_unorm_hdr10: return (int32_t)DXGI_FORMAT_R10G10B10A2_UNORM;
      case ComponentFormat::rgb10a2_ui: return (int32_t)DXGI_FORMAT_R10G10B10A2_UINT;
      case ComponentFormat::rgba32_f: return (int32_t)DXGI_FORMAT_R32G32B32A32_FLOAT;
      case ComponentFormat::rgba32_ui: return (int32_t)DXGI_FORMAT_R32G32B32A32_UINT;
      case ComponentFormat::rgba32_i: return (int32_t)DXGI_FORMAT_R32G32B32A32_SINT;
      
      case ComponentFormat::bc6h_uf: return (int32_t)DXGI_FORMAT_BC6H_UF16;
      case ComponentFormat::bc6h_f: return (int32_t)DXGI_FORMAT_BC6H_SF16;
      case ComponentFormat::bc7_sRGB: return (int32_t)DXGI_FORMAT_BC7_UNORM_SRGB;
      case ComponentFormat::bc7_unorm: return (int32_t)DXGI_FORMAT_BC7_UNORM;
      
      case ComponentFormat::bgra8_sRGB: return (int32_t)DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      case ComponentFormat::bgra8_unorm: return (int32_t)DXGI_FORMAT_B8G8R8A8_UNORM;
      case ComponentFormat::r8_ui: return (int32_t)DXGI_FORMAT_R8_UINT;
      case ComponentFormat::r8_i: return (int32_t)DXGI_FORMAT_R8_SINT;
      case ComponentFormat::r8_unorm: return (int32_t)DXGI_FORMAT_R8_UNORM;
      case ComponentFormat::r8_snorm: return (int32_t)DXGI_FORMAT_R8_SNORM;
      case ComponentFormat::a8_unorm: return (int32_t)DXGI_FORMAT_A8_UNORM;
      case ComponentFormat::rg8_unorm: return (int32_t)DXGI_FORMAT_R8G8_UNORM;
      case ComponentFormat::rg8_snorm: return (int32_t)DXGI_FORMAT_R8G8_SNORM;
      case ComponentFormat::rg8_ui: return (int32_t)DXGI_FORMAT_R8G8_UINT;
      case ComponentFormat::rg8_i: return (int32_t)DXGI_FORMAT_R8G8_SINT;
      case ComponentFormat::rgb5a1_unorm: return (int32_t)DXGI_FORMAT_B5G5R5A1_UNORM;
      case ComponentFormat::r5g6b5_unorm: return (int32_t)DXGI_FORMAT_B5G6R5_UNORM;
      
      case ComponentFormat::rg16_f: return (int32_t)DXGI_FORMAT_R16G16_FLOAT;
      case ComponentFormat::r16_f: return (int32_t)DXGI_FORMAT_R16_FLOAT;
      case ComponentFormat::rg16_unorm: return (int32_t)DXGI_FORMAT_R16G16_UNORM;
      case ComponentFormat::r16_unorm: return (int32_t)DXGI_FORMAT_R16_UNORM;
      case ComponentFormat::rg16_snorm: return (int32_t)DXGI_FORMAT_R16G16_SNORM;
      case ComponentFormat::r16_snorm: return (int32_t)DXGI_FORMAT_R16_SNORM;
      case ComponentFormat::rg16_ui: return (int32_t)DXGI_FORMAT_R16G16_UINT;
      case ComponentFormat::r16_ui: return (int32_t)DXGI_FORMAT_R16_UINT;
      case ComponentFormat::rg16_i: return (int32_t)DXGI_FORMAT_R16G16_SINT;
      case ComponentFormat::r16_i: return (int32_t)DXGI_FORMAT_R16_SINT;
      case ComponentFormat::rgb32_f: return (int32_t)DXGI_FORMAT_R32G32B32_FLOAT;
      case ComponentFormat::rg32_f: return (int32_t)DXGI_FORMAT_R32G32_FLOAT;
      case ComponentFormat::r32_f: return (int32_t)DXGI_FORMAT_R32_FLOAT;
      case ComponentFormat::rgb32_ui: return (int32_t)DXGI_FORMAT_R32G32B32_UINT;
      case ComponentFormat::rg32_ui: return (int32_t)DXGI_FORMAT_R32G32_UINT;
      case ComponentFormat::r32_ui: return (int32_t)DXGI_FORMAT_R32_UINT;
      case ComponentFormat::rgb32_i: return (int32_t)DXGI_FORMAT_R32G32B32_SINT;
      case ComponentFormat::rg32_i: return (int32_t)DXGI_FORMAT_R32G32_SINT;
      case ComponentFormat::r32_i: return (int32_t)DXGI_FORMAT_R32_SINT;
      case ComponentFormat::rg11b10_f: return (int32_t)DXGI_FORMAT_R11G11B10_FLOAT;
      case ComponentFormat::rgb9e5_uf: return (int32_t)DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
      
#     if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
        case ComponentFormat::rgba4_unorm: return (int32_t)DXGI_FORMAT_B4G4R4A4_UNORM;
#     endif
      case ComponentFormat::unknown: 
      default: return (int32_t)DXGI_FORMAT_UNKNOWN;
    }
  }


// -- error messages -- --------------------------------------------------------

  void pandora::video::d3d11::throwError(HRESULT result, const char* messageContent) {
    auto message = std::string(messageContent) + ": Direct3D error ";
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

#endif
