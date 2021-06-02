/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
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
# include "video/d3d11/_private/_d3d_resource.h"

  using namespace pandora::video::d3d11;
  using namespace pandora::video;

  static int32_t _toDxgiFormat(ComponentFormat format) noexcept;


// -- device resource creation -- ----------------------------------------------

  // Destroy resources
  Renderer::~Renderer() noexcept {
    try {
      if (_dxgiFactory)
        ((IDXGIFactory1*)_dxgiFactory)->Release();
      if (_context)
        ((ID3D11DeviceContext*)_context)->Release();
      if (_device)
        ((ID3D11Device*)_device)->Release();
    }
    catch (...) {}
  }
  
  // Move instance
  Renderer::Renderer(Renderer&& rhs) noexcept 
    : _device(rhs._device), _context(rhs._context), _dxgiFactory(rhs._dxgiFactory), 
      _deviceLevel(rhs._deviceLevel), _dxgiLevel(rhs._dxgiLevel) { 
    rhs._device = rhs._context = rhs._dxgiFactory = nullptr; 
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    _device = rhs._device; _context = rhs._context; _dxgiFactory = rhs._dxgiFactory;
    _deviceLevel = rhs._deviceLevel; _dxgiLevel = rhs._dxgiLevel;
    rhs._device = rhs._context = rhs._dxgiFactory = nullptr;
    return *this;
  }

  // ---

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
  
  // Find primary hardware adapter
  static D3dResource<IDXGIAdapter1> __getHardwareAdapter(IDXGIFactory1* factory) {
    D3dResource<IDXGIAdapter1> adapter;
#   if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
      auto factoryV6 = D3dResource<IDXGIFactory6>::tryFromInterface(factory);
      if (factoryV6) {
        for (UINT i = 0; SUCCEEDED(factoryV6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, 
                                                                        IID_PPV_ARGS(adapter.address()))); ++i) {
          DXGI_ADAPTER_DESC1 description;
          if (SUCCEEDED(adapter->GetDesc1(&description)) && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
            break; // hardware adapter found -> exit loop
          adapter.destroy(); // don't select Basic Render Driver -> release
        }
        if (adapter)
          return adapter;
      }
#   endif

    for (UINT i = 0; SUCCEEDED(factory->EnumAdapters1(i, adapter.address())); ++i) {
      DXGI_ADAPTER_DESC1 description;
      if (SUCCEEDED(adapter->GetDesc1(&description)) && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
        break; // hardware adapter found -> exit loop
      adapter.destroy(); // don't select Basic Render Driver -> release
    }
    return adapter;
  }

  // ---

  // Create resources
  void Renderer::_createDeviceResources(uint32_t minLevel) {
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
    while (featureLevelCount && minLevel) {
      --featureLevelCount;
      --minLevel;
    }
    if (featureLevelCount == 0)
      throw std::out_of_range("Renderer: minimum feature level requested is higher than max");

    // create DXGI factory + get primary adapter (if not found, default adapter will be used instead)
    D3dResource<IDXGIFactory1> dxgiFactory;
    __createDxgiFactory(dxgiFactory); // throws
    D3dResource<IDXGIAdapter1> adapter = __getHardwareAdapter(dxgiFactory.get()); // may be NULL (not found)
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
        this->_deviceLevel = RendererDeviceLevel::direct3D_11_1;
      else {
        this->_deviceLevel = RendererDeviceLevel::direct3D_11_0;
        this->_dxgiLevel = 1u;
      }
#   else
      this->_deviceLevel = RendererDeviceLevel::direct3D_11_0;
      this->_dxgiLevel = 1u;
#   endif
    this->_dxgiFactory = dxgiFactory.extract();

#   ifdef _DEBUG
      __configureDeviceDebug((ID3D11Device*)this->_device);
#   endif
  }


// -- feature support -- -------------------------------------------------------

  // Verify if a multisample mode is supported
  bool Renderer::isMultisampleSupported(uint32_t sampleCount, uint32_t& outMaxQualityLevel) const noexcept {
    try {
      UINT qualityLevel = 0;
      if (SUCCEEDED(((ID3D11Device*)this->_device)->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &qualityLevel))) {
        outMaxQualityLevel = qualityLevel;
        return (qualityLevel > 0);
      }
    }
    catch (...) {}
    return false;
  }
  
  // Verify if a monitor can display HDR colors
# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
    bool Renderer::isMonitorHdrCapable(const pandora::hardware::DisplayMonitor& target) const noexcept {
      bool isHdrCompatible = false;
      try {
        auto dxgiDevice = D3dResource<IDXGIDevice>::tryFromInterface((ID3D11Device*)this->_device);
        if (dxgiDevice) {
          D3dResource<IDXGIAdapter> adapter;
          if (SUCCEEDED(dxgiDevice->GetAdapter(adapter.address())) && adapter) {

            D3dResource<IDXGIOutput> output;
            for (UINT index = 0; adapter->EnumOutputs(index, output.address()) == S_OK; ++index) {
              auto outputV6 = D3dResource<IDXGIOutput6>::tryFromInterface(output.get());
              if (outputV6) {
                DXGI_OUTPUT_DESC1 monitorDescription;
                if (outputV6->GetDesc1(&monitorDescription) == S_OK && target.attributes().id == monitorDescription.DeviceName) {
                  isHdrCompatible = (monitorDescription.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
                  break;
                }
              }
            }
          }
        }
      }
      catch (...) {}
      return isHdrCompatible;
    }
# else
    bool Renderer::isMonitorHdrCapable(const pandora::hardware::DisplayMonitor&) const noexcept { return false; }
# endif
  
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
  
  
// -- swap-chain creation -- ---------------------------------------------------

  // Verify if buffer format is supported by flip-swap (-> returned in outIsFlipSwapAllowed) + return swap-chain format to use
  static inline DXGI_FORMAT __verifyFlipSwap(DXGI_FORMAT backBufferFormat, bool& outIsFlipSwapAllowed) noexcept {
    switch (backBufferFormat) {
      case DXGI_FORMAT_R16G16B16A16_FLOAT: 
      case DXGI_FORMAT_R8G8B8A8_UNORM:
      case DXGI_FORMAT_B8G8R8A8_UNORM:
      case DXGI_FORMAT_R10G10B10A2_UNORM: outIsFlipSwapAllowed = true; return backBufferFormat;
      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: outIsFlipSwapAllowed = true; return DXGI_FORMAT_R8G8B8A8_UNORM;
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: outIsFlipSwapAllowed = true; return DXGI_FORMAT_B8G8R8A8_UNORM;
      default: break;
    }
    outIsFlipSwapAllowed = false;
    return backBufferFormat;
  }
  
  // Convert portable params to Direct3D swap-chain params
  static inline void __convertSwapChainParams(const Renderer& renderer, const SwapChainParams& params, 
                                              DXGI_FORMAT& outSwapChainFormat, SwapChain::Config& outConfig) noexcept {
    // convert buffer formats
    outConfig.backBufferFormat = (params.backBufferFormat() != ComponentFormat::custom)
                                 ? _toDxgiFormat(params.backBufferFormat())
                                 : params.customBackBufferFormat();
    outConfig.depthBufferFormat = (params.depthBufferFormat() != ComponentFormat::custom)
                                  ? _toDxgiFormat(params.depthBufferFormat())
                                  : params.customDepthBufferFormat();
    outConfig.drawingBufferNumber = params.drawingBufferCount();
    
    // verify flip-swap support/constraints
    outConfig.useFlipSwap = (renderer.isFlipSwapAvailable() 
                        && (renderer.isTearingAvailable() || (params.outputFlags() & SwapChainOutputFlag::variableRefresh) == SwapChainOutputFlag::none)
                        && (params.renderTargetMode() == RenderTargetMode::uniqueOutput 
                          || params.renderTargetMode() == RenderTargetMode::uniqueRatioOutput));
    outSwapChainFormat = (outConfig.useFlipSwap) 
                       ? __verifyFlipSwap((DXGI_FORMAT)outConfig.backBufferFormat, outConfig.useFlipSwap) 
                       : (DXGI_FORMAT)outConfig.backBufferFormat;
    
    // buffer constraints
    outConfig.bufferUsageMode = (params.renderTargetMode() != RenderTargetMode::none) ? (int32_t)DXGI_USAGE_RENDER_TARGET_OUTPUT : 0;
    if ((params.outputFlags() & SwapChainOutputFlag::shaderInput) == true)
      outConfig.bufferUsageMode |= (int32_t)DXGI_USAGE_SHADER_INPUT;
    outConfig.flags = params.outputFlags();
    if ((outConfig.flags & SwapChainOutputFlag::localOutput) == true && !renderer.isLocalOnlyOutputAvailable())
      outConfig.flags &= ~(SwapChainOutputFlag::localOutput);
    
    // buffer display
    switch (params.renderTargetMode()) {
      case RenderTargetMode::partialOutput:     outConfig.scalingMode = (int32_t)DXGI_SCALING_NONE; break;
      case RenderTargetMode::uniqueRatioOutput: outConfig.scalingMode = (int32_t)DXGI_SCALING_ASPECT_RATIO_STRETCH; break;
      default: outConfig.scalingMode = (int32_t)DXGI_SCALING_STRETCH; break;
    }
    switch (params.alphaBlending()) {
      case AlphaBlending::ignored: outConfig.alphaBlending = (int32_t)DXGI_ALPHA_MODE_IGNORE; break;
      case AlphaBlending::preMultiplied: outConfig.alphaBlending = (int32_t)DXGI_ALPHA_MODE_PREMULTIPLIED; break;
      default: outConfig.alphaBlending = (int32_t)DXGI_ALPHA_MODE_STRAIGHT; break;
    }
    
    // multisample mode
    if (!outConfig.useFlipSwap && params.multisampleCount() > 1u && renderer.isMultisampleSupported(params.multisampleCount(), outConfig.msaaQualityLevel))
      outConfig.msaaSampleNumber = params.multisampleCount();
    else {
      outConfig.msaaSampleNumber = 1u;
      outConfig.msaaQualityLevel = 1u;
    }
  }
  
  // ---

  // Create new rendering swap-chain for current device
  SwapChain Renderer::createSwapChain(const SwapChainParams& params, 
                                      WindowHandle window, uint32_t width, uint32_t height) {
    if (width == 0 || height == 0)
      throw std::invalid_argument("Renderer.createSwapChain: invalid width/height: values must not be 0");
    if (window == nullptr)
      throw std::invalid_argument("Renderer.createSwapChain: invalid window handle: must not be NULL");
    
    SwapChain::Config config{};
    DXGI_FORMAT swapChainFormat;
    __convertSwapChainParams(*this, params, swapChainFormat, config);
    
    // clear previous device context
    ID3D11RenderTargetView* nullViews[] = {nullptr};
    ((ID3D11DeviceContext*)this->_context)->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    
    // create swap-chain
    bool isSwapChain11_1 = false;
    SwapChain::Handle swapChain = nullptr;

#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (this->_dxgiLevel >= 2u) {
        // Direct3D 11.1+
        auto dxgiFactoryV2 = D3dResource<IDXGIFactory2>::fromInterface((IDXGIFactory1*)this->_dxgiFactory, "Renderer: failed to access DirectX 11.1 graphics infra");
        auto deviceV1 = D3dResource<ID3D11Device1>::fromInterface((ID3D11Device*)this->_device, "Renderer: failed to access DirectX 11.1 device");
        isSwapChain11_1 = true;
        
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscnDescriptor = {};
        fullscnDescriptor.Windowed = ((config.flags & SwapChainOutputFlag::stereo) == true) ? FALSE : TRUE;
        DXGI_SWAP_CHAIN_DESC1 descriptor = {};
        descriptor.Width = width;
        descriptor.Height = height;
        descriptor.Format = swapChainFormat;
        descriptor.BufferCount = (UINT)config.drawingBufferNumber;
        descriptor.BufferUsage = (DXGI_USAGE)config.bufferUsageMode;
        descriptor.Scaling = (DXGI_SCALING)config.scalingMode;
        descriptor.AlphaMode = (DXGI_ALPHA_MODE)config.alphaBlending;
        fullscnDescriptor.RefreshRate.Numerator = (UINT)params.rateNumerator();
        fullscnDescriptor.RefreshRate.Denominator = (UINT)params.rateDenominator();
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
      DXGI_SWAP_CHAIN_DESC descriptor = {};
      descriptor.BufferDesc.Width = width;
      descriptor.BufferDesc.Height = height;
      descriptor.BufferDesc.Format = swapChainFormat;
      descriptor.BufferCount = (UINT)config.drawingBufferNumber;
      descriptor.BufferUsage = (DXGI_USAGE)config.bufferUsageMode;
      descriptor.BufferDesc.Scaling = ((DXGI_SCALING)config.scalingMode != DXGI_SCALING_NONE) ? DXGI_MODE_SCALING_STRETCHED : DXGI_MODE_SCALING_CENTERED;
      descriptor.BufferDesc.RefreshRate.Numerator = (UINT)params.rateNumerator();
      descriptor.BufferDesc.RefreshRate.Denominator = (UINT)params.rateDenominator();
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
    return SwapChain(this, config, width, height, swapChain, isSwapChain11_1); // throws
  }


// -- swap-chain params -- -----------------------------------------------------

  static int32_t _toDxgiFormat(ComponentFormat format) noexcept {
    switch (format) {
      case ComponentFormat::rgba8_sRGB: return (int32_t)DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      case ComponentFormat::rgba8_unorm: return (int32_t)DXGI_FORMAT_R8G8B8A8_UNORM;
      case ComponentFormat::rgba8_snorm: return (int32_t)DXGI_FORMAT_R8G8B8A8_SNORM;
      case ComponentFormat::rgba8_ui: return (int32_t)DXGI_FORMAT_R8G8B8A8_UINT;
      case ComponentFormat::rgba8_i: return (int32_t)DXGI_FORMAT_R8G8B8A8_SINT;
      case ComponentFormat::rgba8_any: return (int32_t)DXGI_FORMAT_R8G8B8A8_TYPELESS;
      
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
      case ComponentFormat::rgba16_any: return (int32_t)DXGI_FORMAT_R16G16B16A16_TYPELESS;
      case ComponentFormat::rgb10a2_unorm_hdr10: return (int32_t)DXGI_FORMAT_R10G10B10A2_UNORM;
      case ComponentFormat::rgb10a2_any: return (int32_t)DXGI_FORMAT_R10G10B10A2_TYPELESS;
      case ComponentFormat::rgb10a2_ui: return (int32_t)DXGI_FORMAT_R10G10B10A2_UINT;
      case ComponentFormat::rgba32_f: return (int32_t)DXGI_FORMAT_R32G32B32A32_FLOAT;
      case ComponentFormat::rgba32_ui: return (int32_t)DXGI_FORMAT_R32G32B32A32_UINT;
      case ComponentFormat::rgba32_i: return (int32_t)DXGI_FORMAT_R32G32B32A32_SINT;
      case ComponentFormat::rgba32_any: return (int32_t)DXGI_FORMAT_R32G32B32A32_TYPELESS;
      
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
      case ComponentFormat::r8_any: return (int32_t)DXGI_FORMAT_R8_TYPELESS;
      case ComponentFormat::bgra8_any: return (int32_t)DXGI_FORMAT_B8G8R8A8_TYPELESS;
      case ComponentFormat::rg8_unorm: return (int32_t)DXGI_FORMAT_R8G8_UNORM;
      case ComponentFormat::rg8_snorm: return (int32_t)DXGI_FORMAT_R8G8_SNORM;
      case ComponentFormat::rg8_ui: return (int32_t)DXGI_FORMAT_R8G8_UINT;
      case ComponentFormat::rg8_i: return (int32_t)DXGI_FORMAT_R8G8_SINT;
      case ComponentFormat::rg8_any: return (int32_t)DXGI_FORMAT_R8G8_TYPELESS;
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
      case ComponentFormat::rg16_any: return (int32_t)DXGI_FORMAT_R16G16_TYPELESS;
      case ComponentFormat::r16_any: return (int32_t)DXGI_FORMAT_R16_TYPELESS;
      case ComponentFormat::rgb32_f: return (int32_t)DXGI_FORMAT_R32G32B32_FLOAT;
      case ComponentFormat::rg32_f: return (int32_t)DXGI_FORMAT_R32G32_FLOAT;
      case ComponentFormat::r32_f: return (int32_t)DXGI_FORMAT_R32_FLOAT;
      case ComponentFormat::rgb32_ui: return (int32_t)DXGI_FORMAT_R32G32B32_UINT;
      case ComponentFormat::rg32_ui: return (int32_t)DXGI_FORMAT_R32G32_UINT;
      case ComponentFormat::r32_ui: return (int32_t)DXGI_FORMAT_R32_UINT;
      case ComponentFormat::rgb32_i: return (int32_t)DXGI_FORMAT_R32G32B32_SINT;
      case ComponentFormat::rg32_i: return (int32_t)DXGI_FORMAT_R32G32_SINT;
      case ComponentFormat::r32_i: return (int32_t)DXGI_FORMAT_R32_SINT;
      case ComponentFormat::rgb32_any: return (int32_t)DXGI_FORMAT_R32G32B32_TYPELESS;
      case ComponentFormat::rg32_any: return (int32_t)DXGI_FORMAT_R32G32_TYPELESS;
      case ComponentFormat::r32_any: return (int32_t)DXGI_FORMAT_R32_TYPELESS;
      case ComponentFormat::rg11b10_f: return (int32_t)DXGI_FORMAT_R11G11B10_FLOAT;
      case ComponentFormat::rgb9e5_uf: return (int32_t)DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
      
#     if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
        case ComponentFormat::rgba4_unorm: return (int32_t)DXGI_FORMAT_B4G4R4A4_UNORM;
#     endif
      default: return (int32_t)format;
    }
  }

#endif
