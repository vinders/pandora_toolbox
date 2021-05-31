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

# if defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION >= 114
#   include <d3d11_4.h>
#   define _D3D_FEATURE_LEVEL_11_4  (D3D_FEATURE_LEVEL)0xb400
#   define _D3D_FEATURE_LEVEL_11_3  (D3D_FEATURE_LEVEL)0xb300
#   define _D3D_FEATURE_LEVEL_11_2  (D3D_FEATURE_LEVEL)0xb200
# elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 113
#   include <d3d11_3.h>
#   define _D3D_FEATURE_LEVEL_11_3  (D3D_FEATURE_LEVEL)0xb300
#   define _D3D_FEATURE_LEVEL_11_2  (D3D_FEATURE_LEVEL)0xb200
# elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 110
#   include <d3d11.h>
# else
#   include <d3d11_1.h>
# endif

# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
#   include <dxgi1_6.h>
# elif defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#   include <dxgi1_5.h>
# else
#   include <dxgi1_2.h>
# endif
# ifdef _DEBUG
#   include <dxgidebug.h>
# endif
# include <DirectXMath.h>
# include <DirectXColors.h>

  using namespace pandora::video::d3d11;
  
  // Format and throw error messages
  static void __throwErrorMessage(HRESULT result, const char* defaultMessage) {
    if (FAILED(result)) {
      if (FACILITY_WINDOWS == HRESULT_FACILITY(result)) 
        result = HRESULT_CODE(result);
      
      char* buffer; 
      if(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, nullptr, result, 
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, nullptr) != 0) { 
        std::string error = buffer;
        LocalFree(buffer); 
        throw std::runtime_error(std::move(error));
      } 
    }
    throw std::runtime_error(defaultMessage);
  }


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
  Renderer::Renderer(Renderer&& rhs) noexcept : _device(rhs._device), _context(rhs._context), _dxgiFactory(rhs._dxgiFactory) { 
    rhs._device = rhs._context = rhs._dxgiFactory = nullptr; 
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    _device = rhs._device; _context = rhs._context; _dxgiFactory = rhs._dxgiFactory;
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

  // Verify one DXGI factory level support
  template <typename _IDxgiFactoryLevel>
  static inline bool __isDxgiFactoryLevelSupported(IDXGIFactory1* dxgiFactory) noexcept {
    _IDxgiFactoryLevel* factory;
    if (SUCCEEDED(dxgiFactory->QueryInterface(__uuidof(_IDxgiFactoryLevel), 
                                              reinterpret_cast<void**>(&factory))) && factory != nullptr) {
      factory->Release();
      return true;
    }
    return false;
  }
  // Get max DXGI factory level
  static uint32_t __getDxgiFactoryLevel(IDXGIFactory1* dxgiFactory) {
#   if (defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2) || (defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#     if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
        if (__isDxgiFactoryLevelSupported<IDXGIFactory6>(dxgiFactory)) return 6u;
#     endif
      if (__isDxgiFactoryLevelSupported<IDXGIFactory5>(dxgiFactory)) return 5u;
      if (__isDxgiFactoryLevelSupported<IDXGIFactory4>(dxgiFactory)) return 4u;
      if (__isDxgiFactoryLevelSupported<IDXGIFactory3>(dxgiFactory)) return 3u;
#   endif
    if (__isDxgiFactoryLevelSupported<IDXGIFactory2>(dxgiFactory)) return 2u;
    return 1u;
  }

  // Create DXGI factory + configure DXGI debugger
  static void __createDxgiFactory(IDXGIFactory1** outDxgiFactory) {
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
        __throwErrorMessage(result, "d3d11::Renderer: failed to create DirectX graphics infrastructure");
    }
    if (*outDxgiFactory != nullptr)
      (*outDxgiFactory)->Release();
    *outDxgiFactory = dxgiFactory;
  }
  
  // Get current DXGI factory of device
  static inline void __getCurrentDxgiFactory(ID3D11Device* device, IDXGIFactory1** outDxgiFactory) {
    IDXGIFactory1* dxgiFactory = nullptr;
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT result = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (SUCCEEDED(result) && dxgiDevice != nullptr) {
      IDXGIAdapter* adapter = nullptr;
      result = dxgiDevice->GetAdapter(&adapter);
      if (SUCCEEDED(result) && adapter != nullptr) {
        result = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
        adapter->Release();
      }
      dxgiDevice->Release();
    }
    if (FAILED(result) || dxgiFactory == nullptr)
      __throwErrorMessage(result, "d3d11::Renderer: failed to obtain DirectX graphics infrastructure");
    
    if (*outDxgiFactory != nullptr)
      (*outDxgiFactory)->Release();
    *outDxgiFactory = dxgiFactory;
  }
  
# ifdef _DEBUG
    // Configure device debugger
    static inline void __configureDeviceDebug(ID3D11Device* device) {
      ID3D11Debug* d3dDebug;
      if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug)))) {
        ID3D11InfoQueue* d3dInfoQueue;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&d3dInfoQueue)))) {
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
          
          D3D11_MESSAGE_ID disabled[] = { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS };
          D3D11_INFO_QUEUE_FILTER filter = {};
          filter.DenyList.NumIDs = _countof(disabled);
          filter.DenyList.pIDList = disabled;
          d3dInfoQueue->AddStorageFilterEntries(&filter);
          
          d3dInfoQueue->Release();
        }
        d3dDebug->Release();
      }
    }
# endif

  // ---
  
  // Find primary hardware adapter
  static IDXGIAdapter1* __getHardwareAdapter(IDXGIFactory1* factory) {
    IDXGIAdapter1* adapter = nullptr;
#   if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
      IDXGIFactory6* factory6 = nullptr;
      if (SUCCEEDED(factory->QueryInterface(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(&factory6)))) {
        for (UINT i = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, 
                                                                        IID_PPV_ARGS(&adapter))); ++i) {
          DXGI_ADAPTER_DESC1 description;
          if (SUCCEEDED(adapter->GetDesc1(&description)) && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
            break; // hardware adapter found -> exit loop
          adapter->Release(); // don't select Basic Render Driver -> release
        }
        
        factory6->Release();
        if (adapter != nullptr)
          return adapter;
      }
#   endif
    
    for (UINT i = 0; SUCCEEDED(factory->EnumAdapters1(i, &adapter)); ++i) {
      DXGI_ADAPTER_DESC1 description;
      if (SUCCEEDED(adapter->GetDesc1(&description)) && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
        break; // hardware adapter found -> exit loop
      adapter->Release(); // don't select Basic Render Driver -> release
    }
    return adapter;
  }

  // ---

  // Create resources
  void Renderer::createDeviceResources(uint32_t minLevel) {
    UINT runtimeLayers = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#   ifdef _DEBUG
      if (__isDebugSdkAvailable())
        runtimeLayers |= D3D11_CREATE_DEVICE_DEBUG;
#   endif

    // list supported feature levels
    const D3D_FEATURE_LEVEL featureLevels[] = {
#     if defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION >= 114
        _D3D_FEATURE_LEVEL_11_4,
        _D3D_FEATURE_LEVEL_11_3,
        _D3D_FEATURE_LEVEL_11_2,
#     elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 113
        _D3D_FEATURE_LEVEL_11_3,
        _D3D_FEATURE_LEVEL_11_2,
#     endif
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
      throw std::out_of_range("d3d11::Renderer: minimum feature level requested is higher than max");

    // create DXGI factory + get primary adapter (if not found, default adapter will be used instead)
    __createDxgiFactory((IDXGIFactory1**)&(this->_dxgiFactory)); // throws
    IDXGIAdapter1* adapter = __getHardwareAdapter((IDXGIFactory1*)this->_dxgiFactory); // may be NULL (not found)
    bool isDefaultAdapter = (adapter == nullptr);

    // create rendering device + context
    D3D_FEATURE_LEVEL deviceLevel;
    auto result = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, runtimeLayers, 
                                    featureLevels, featureLevelCount, D3D11_SDK_VERSION, 
                                    (ID3D11Device**)&(this->_device), &deviceLevel, 
                                    (ID3D11DeviceContext**)&(this->_context));
    if (adapter != nullptr)
      adapter->Release(); // release *before* potential throw
    if (FAILED(result) || this->_device == nullptr || this->_context == nullptr)
      __throwErrorMessage(result, "d3d11::Renderer: failed to create device and context"); // throws
    if (isDefaultAdapter && !((IDXGIFactory1*)this->_dxgiFactory)->IsCurrent()) // if adapter not provided, system may generate another factory
      __getCurrentDxgiFactory((ID3D11Device*)this->_device, (IDXGIFactory1**)&(this->_dxgiFactory)); // throws
    
    // feature level detection
    this->_dxgiLevel = __getDxgiFactoryLevel((IDXGIFactory1*)this->_dxgiFactory);
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

#   ifdef _DEBUG
      __configureDeviceDebug((ID3D11Device*)this->_device);
#   endif
  }


// -- feature support -- -------------------------------------------------------

  // Verify if a multisample mode is supported
  bool Renderer::isMultisampleSupported(uint32_t sampleCount, uint32_t& outMaxQualityLevel) const noexcept {
    UINT qualityLevel = 0;
    if (SUCCEEDED(((ID3D11Device*)this->_device)->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &qualityLevel))) {
      outMaxQualityLevel = qualityLevel;
      return (qualityLevel > 0);
    }
    return false;
  }

#endif
