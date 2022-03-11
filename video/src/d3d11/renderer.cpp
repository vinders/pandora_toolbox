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
Includes hpp implementations at the end of the file 
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# ifndef __MINGW32__
#   pragma warning(push)
#   pragma warning(disable: 4100)  // disable warnings about unused params
#   pragma warning(disable: 6387)  // disable warnings about empty handles
#   pragma warning(disable: 33011) // disable warnings about upper bound check
# endif
# include <cstddef>
# include <cstring>
# include <cmath>
# include <mutex>
# include <stdexcept>
# include <thread/spin_lock.h>
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
# ifdef _P_VIDEO_SHADER_COMPILERS
#   include <D3DCompiler.h>
# endif

# include "video/d3d11/_private/_error.h"
# include "video/d3d11/_private/_shared_resource.h"
# include "video/d3d11/renderer.h"
# include "video/d3d11/swap_chain.h"
# include "video/d3d11/graphics_pipeline.h"
# include "video/d3d11/depth_stencil_buffer.h"

# include "video/d3d11/shader.h"
# include "video/d3d11/buffer.h"
# include "video/d3d11/texture.h"
# include "video/d3d11/texture_reader.h"
# include "video/d3d11/texture_writer.h"
# include "video/d3d11/camera_utils.h"

# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
#   define __if_constexpr if constexpr
# else
#   define __if_constexpr if
# endif

  using namespace pandora::video::d3d11;
  using namespace pandora::video;
  using pandora::memory::LightVector;


// -----------------------------------------------------------------------------
// renderer.h
// -----------------------------------------------------------------------------

// -- display adapter detection -- ---------------------------------------------

  // Verify if a hardware adapter outputs on a monitor
  static bool __isHardwareAdapterForMonitor(IDXGIAdapter1* adapter, const pandora::memory::LightWString& targetMonitorId) {
    SharedResource<IDXGIOutput> output;
    DXGI_OUTPUT_DESC monitorDescription;
    for (UINT index = 0; adapter->EnumOutputs(index, output.address()) == S_OK && output; ++index) {
      if (output->GetDesc(&monitorDescription) == S_OK && targetMonitorId == monitorDescription.DeviceName)
        return true; // hardware adapter uses the target output -> exit loop
    }
    return false;
  }

  // Find primary hardware adapter
  static SharedResource<IDXGIAdapter1> __getHardwareAdapter(IDXGIFactory1* factory, const pandora::hardware::DisplayMonitor& target) {
    // try to find adapter associated with monitor (if not empty monitor)
    pandora::memory::LightWString targetDeviceString;
    if (!target.attributes().id.empty())
      targetDeviceString = target.adapterName();

    SharedResource<IDXGIAdapter1> adapter;
    SharedResource<IDXGIAdapter1> preferredAdapter;
#   if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
      auto factoryV6 = SharedResource<IDXGIFactory6>::tryFromInterface(factory);
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
          adapter.release(); // do not return current value if not appropriate
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
      adapter.release(); // do not return current value if not appropriate
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
        if (SharedResource<IDXGIFactory6>::tryFromInterface(dxgiFactory)) return 6u;
#     endif
      if (SharedResource<IDXGIFactory5>::tryFromInterface(dxgiFactory)) return 5u;
      if (SharedResource<IDXGIFactory4>::tryFromInterface(dxgiFactory)) return 4u;
      if (SharedResource<IDXGIFactory3>::tryFromInterface(dxgiFactory)) return 3u;
#   endif
    if (SharedResource<IDXGIFactory2>::tryFromInterface(dxgiFactory)) return 2u;
    return 1u;
  }

  // Create DXGI factory + configure DXGI debugger
  static void __createDxgiFactory(SharedResource<IDXGIFactory1>& outDxgiFactory) { // throws
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
  static inline void __getCurrentDxgiFactory(ID3D11Device* device, SharedResource<IDXGIFactory1>& outDxgiFactory) { // throws
    auto dxgiDevice = SharedResource<IDXGIDevice>::fromInterface(device, "Renderer: DXGI access error");
    SharedResource<IDXGIAdapter> adapter;
    auto result = dxgiDevice->GetAdapter(adapter.address());
    if (FAILED(result) || !adapter.hasValue())
      throwError(result, "Renderer: adapter access error");
    
    outDxgiFactory = SharedResource<IDXGIFactory1>::fromChild(adapter.get(), "Renderer: DXGI factory error");
  }
  
  // If output information on DXGI factory is stale, try to create a new one
  static __forceinline void __refreshDxgiFactory(void* dxgiFactory) { // throws
    if (!((IDXGIFactory1*)dxgiFactory)->IsCurrent()) {
      SharedResource<IDXGIFactory1> newFactory;
      __createDxgiFactory(newFactory);
      dxgiFactory = (void*)newFactory.extract();
    }
  }
  
# ifdef _DEBUG
    // Configure device debugger
    static inline void __configureDeviceDebug(ID3D11Device* device) {
      auto d3dDebug = SharedResource<ID3D11Debug>::tryFromInterface(device);
      if (d3dDebug) {
        auto d3dInfoQueue = SharedResource<ID3D11InfoQueue>::tryFromInterface(d3dDebug.get());
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
  Renderer::Renderer(const pandora::hardware::DisplayMonitor& monitor,
                     const D3D_FEATURE_LEVEL* featureLevels, size_t featureLevelCount) { // throws
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
        throw std::out_of_range("Renderer: no feature-levels");
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
          throw std::out_of_range("Renderer: no feature-levels/above max supported");
      }
#   endif

    // create DXGI factory + get primary adapter (if not found, default adapter will be used instead)
    SharedResource<IDXGIFactory1> dxgiFactory;
    __createDxgiFactory(dxgiFactory); // throws
    SharedResource<IDXGIAdapter1> adapter = __getHardwareAdapter(dxgiFactory.get(), monitor); // may be NULL (not found)
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
      else
        this->_context11_1 = SharedResource<ID3D11DeviceContext1>::tryFromInterface(this->_context).extract();
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
  void Renderer::release() noexcept {
    try {
      // release device context
      if (this->_context) {
#       if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
          if (this->_context11_1) {
            ((ID3D11DeviceContext1*)this->_context11_1)->Release();
            this->_context11_1 = nullptr;
          }
#       endif
        this->_context->Flush();
        this->_context->Release();
        this->_context = nullptr;
      }
      // release device
      if (this->_device) {
        this->_device->Release();
        this->_device = nullptr;
      }
      if (this->_dxgiFactory) {
        ((IDXGIFactory1*)this->_dxgiFactory)->Release();
        this->_dxgiFactory = nullptr;
      }
    }
    catch (...) {}
  }

  Renderer::Renderer(Renderer&& rhs) noexcept
    : _device(rhs._device),
      _context(rhs._context),
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        _context11_1(rhs._context11_1),
#     endif
      _dxgiFactory(rhs._dxgiFactory),
      _dxgiLevel(rhs._dxgiLevel),
      _deviceLevel(rhs._deviceLevel),
      _rasterizerStateCache(std::move(rhs._rasterizerStateCache)),
      _depthStencilStateCache(std::move(rhs._depthStencilStateCache)),
      _blendStateCache(std::move(rhs._blendStateCache)),
      _attachedPipeline(std::move(rhs._attachedPipeline)) {
    rhs._dxgiFactory = nullptr;
    rhs._device = nullptr;
    rhs._context = nullptr;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      rhs._context11_1 = nullptr;
#   endif
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    release();
    this->_device = rhs._device;
    this->_context = rhs._context;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      this->_context11_1 = rhs._context11_1;
#   endif
    this->_dxgiFactory = rhs._dxgiFactory;
    this->_dxgiLevel = rhs._dxgiLevel;
    this->_deviceLevel = rhs._deviceLevel;
    this->_rasterizerStateCache = std::move(rhs._rasterizerStateCache);
    this->_depthStencilStateCache = std::move(rhs._depthStencilStateCache);
    this->_blendStateCache = std::move(rhs._blendStateCache);
    this->_attachedPipeline = std::move(rhs._attachedPipeline);
    rhs._device = nullptr;
    rhs._context = nullptr;
    rhs._dxgiFactory = nullptr;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      rhs._context11_1 = nullptr;
#   endif
    return *this;
  }


// -- accessors -- -------------------------------------------------------------
  
  // Read device adapter VRAM size
  bool Renderer::getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept {
    bool isSuccess = false;
    try {
      auto dxgiDevice = SharedResource<IDXGIDevice>::tryFromInterface(this->_device);
      if (dxgiDevice) {
        SharedResource<IDXGIAdapter> adapter;
        if (SUCCEEDED(dxgiDevice->GetAdapter(adapter.address())) && adapter) {
          
          DXGI_ADAPTER_DESC adapterInfo;
          if (adapter->GetDesc(&adapterInfo) == S_OK) {
            outDedicatedRam = (size_t)adapterInfo.DedicatedVideoMemory + (size_t)adapterInfo.DedicatedSystemMemory;
            outSharedRam = (size_t)adapterInfo.SharedSystemMemory;
            isSuccess = true;
          }
        }
      }
    }
    catch (...) {}
    return isSuccess;
  }
  
  // ---
  
  // Find first color format supported (from a list of candidates, ordered from best to worst)
  DataFormat Renderer::findSupportedDataFormat(const DataFormat* candidates, size_t count,
                                               FormatAttachment attachmentType) const noexcept {
    for (const DataFormat* endIt = candidates + (intptr_t)count; candidates < endIt; ++candidates) {
      FormatAttachment flag = _getDataFormatBindFlag(*candidates);
      if (flag == attachmentType || (flag == FormatAttachment::colorBlend && attachmentType == FormatAttachment::color))
        return *candidates;
    }
    return DataFormat::unknown;
  }
  // Find first depth/stencil format supported (from a list of candidates, ordered from best to worst)
  DepthStencilFormat Renderer::findSupportedDepthStencilFormat(const DepthStencilFormat* candidates, size_t count) const noexcept {
    for (const DepthStencilFormat* endIt = candidates + (intptr_t)count; candidates < endIt; ++candidates) {
      if (*candidates == DepthStencilFormat::d32_f || *candidates == DepthStencilFormat::d32_f_s8_ui
       || *candidates == DepthStencilFormat::d24_unorm_s8_ui || *candidates == DepthStencilFormat::d16_unorm)
        return *candidates;
    }
    return (DepthStencilFormat)0;
  }
  
  // Convert standard sRGB(A) color to gamma-correct linear RGB(A)
  void Renderer::sRgbToGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept {
    DirectX::XMFLOAT3 colorArray(colorRgba);
    DirectX::XMStoreFloat3(&colorArray, DirectX::XMColorSRGBToRGB(DirectX::XMLoadFloat3(&colorArray))); // gamma-correct color
    outRgba[0] = colorArray.x;
    outRgba[1] = colorArray.y;
    outRgba[2] = colorArray.z;
    outRgba[3] = colorRgba[3];
  }


// -- feature support -- -------------------------------------------------------

  // Detect current color space used by display monitor
  ColorSpace Renderer::getMonitorColorSpace(const pandora::hardware::DisplayMonitor& target) const noexcept {
#   if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
      if (_areColorSpacesAvailable()) {
        try {
          auto dxgiDevice = SharedResource<IDXGIDevice>::tryFromInterface(this->_device);
          if (dxgiDevice) {
            SharedResource<IDXGIAdapter> adapter;
            if (SUCCEEDED(dxgiDevice->GetAdapter(adapter.address())) && adapter) {

              SharedResource<IDXGIOutput> output;
              DXGI_OUTPUT_DESC1 monitorDescription;
              for (UINT index = 0; adapter->EnumOutputs(index, output.address()) == S_OK; ++index) {
                auto outputV6 = SharedResource<IDXGIOutput6>::tryFromInterface(output.get());
                if (outputV6) {
                  // if monitor found, verify color space
                  if (outputV6->GetDesc1(&monitorDescription) == S_OK && target.attributes().id == monitorDescription.DeviceName) {
                    return (monitorDescription.ColorSpace != DXGI_COLOR_SPACE_CUSTOM && monitorDescription.ColorSpace != DXGI_COLOR_SPACE_RESERVED)
                         ? (ColorSpace)monitorDescription.ColorSpace
                         : ColorSpace::sRgb;
                  }
                }
              }
            }
          }
        } catch (...) {}
      }
#   endif
    return ColorSpace::unknown;
  }

  // Verify if a multisample mode is supported (MSAA) + get max quality level
  bool Renderer::_isSampleCountSupported(DeviceHandle device, DXGI_FORMAT format, UINT sampleCount, UINT& outMaxQualityLevel) noexcept {
    try {
      UINT qualityLevel = 0;
      if (SUCCEEDED(((ID3D11Device*)device)->CheckMultisampleQualityLevels(format, (UINT)sampleCount, &qualityLevel)) && qualityLevel > 0) {
        outMaxQualityLevel = qualityLevel - 1u;
        return true;
      }
    }
    catch (...) {}
    return false;
  }

  // Screen tearing supported (variable refresh rate display)
  bool Renderer::isTearingAvailable() const noexcept { 
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      if (this->_dxgiLevel >= 5u) {
        BOOL isSupported = FALSE;
        auto factoryV5 = SharedResource<IDXGIFactory5>::tryFromInterface((IDXGIFactory1*)this->_dxgiFactory);
        return (factoryV5 && SUCCEEDED(factoryV5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &isSupported, sizeof(isSupported))) 
             && isSupported != FALSE);
      }
#   endif
    return false;
  }
 

// -- render target operations -------------------------------------------------

  // Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
  void Renderer::setViewports(const Viewport* viewports, size_t numberViewports) noexcept {
    if (numberViewports > D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
      numberViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

    __if_constexpr (sizeof(Viewport) == sizeof(D3D11_VIEWPORT)) {
      this->_context->RSSetViewports((UINT)numberViewports, (const D3D11_VIEWPORT*)viewports);
    }
    else { // realign data (only for weird compilers where alignment differs)
      D3D11_VIEWPORT values[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
      if (numberViewports) {
        D3D11_VIEWPORT* out = &values[numberViewports - 1u];
        for (const Viewport* it = &viewports[numberViewports - 1u]; it >= viewports; --it, --out)
          memcpy((void*)out, (void*)it, sizeof(D3D11_VIEWPORT));
      }
      this->_context->RSSetViewports((UINT)numberViewports, &values[0]);
    }
    this->_attachedPipeline.viewportScissorId = 0;
  }

  // Set rasterizer scissor-test rectangle(s)
  void Renderer::setScissorRectangles(const ScissorRectangle* rectangles, size_t numberRectangles) noexcept {
    if (numberRectangles > D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
      numberRectangles = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

    __if_constexpr(sizeof(Viewport) == sizeof(D3D11_RECT)) {
      this->_context->RSSetScissorRects((UINT)numberRectangles, (const D3D11_RECT*)rectangles);
    }
    else { // realign data (only for weird compilers where alignment differs)
      D3D11_RECT values[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
      if (numberRectangles) {
        D3D11_RECT* out = &values[numberRectangles - 1u];
        for (const ScissorRectangle* it = &rectangles[numberRectangles - 1u]; it >= rectangles; --it, --out)
          memcpy((void*)out, (void*)it, sizeof(D3D11_RECT));
      }
      this->_context->RSSetScissorRects((UINT)numberRectangles, &values[0]);
    }
    this->_attachedPipeline.viewportScissorId = 0;
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
  }
  // Clear render-target content + depth buffer: reset to 'clearColorRgba' and to depth 1
  void Renderer::clearView(RenderTargetView view, DepthStencilView depthBuffer, const ColorChannel clearColorRgba[4]) noexcept {
    if (view != nullptr)
      this->_context->ClearRenderTargetView(view, clearColorRgba ? &clearColorRgba[0] : __defaultBlackColor);
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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
  }

  // Bind/replace active render-target in Renderer (single target) + clear render-target/buffer
  void Renderer::setCleanActiveRenderTarget(RenderTargetView view, DepthStencilView depthBuffer, 
                                            const ColorChannel clearColorRgba[4]) noexcept {
    if (view != nullptr)
      this->_context->ClearRenderTargetView(view, clearColorRgba ? &clearColorRgba[0] : __defaultBlackColor);
    if (depthBuffer != nullptr)
      this->_context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            
    this->_context->OMSetRenderTargets((UINT)1u, &view, depthBuffer);
  }


// -- primitive binding -- -----------------------------------------------------

# ifndef __P_DISABLE_TESSELLATION_STAGE
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
# endif

  // Change output merger blend state with constant factors (color/alpha blending with render-target(s))
  void Renderer::setBlendState(const BlendState& state, const ColorChannel constantColorRgba[4]) noexcept {
    this->_context->OMSetBlendState(state.get(), constantColorRgba, 0xFFFFFFFFu);
    this->_attachedPipeline.blendState = state.get();
    this->_attachedPipeline.blendFactorId = constantColorRgba ? BlendParams::computeFactorId(constantColorRgba)
                                                              : BlendParams::defaultFactorId();
  }

  // ---

  // Bind graphics pipeline to the rendering device
  // -> topology, input-assembler stage, shader stages, pipeline states, viewport/scissor descriptors
  void Renderer::bindGraphicsPipeline(GraphicsPipelineHandle pipeline) noexcept {
    static_assert((unsigned int)ShaderType::vertex == 0 && (unsigned int)ShaderType::fragment == 1
#            ifndef __P_DISABLE_GEOMETRY_STAGE
               && (unsigned int)ShaderType::geometry == 2
#              ifndef __P_DISABLE_TESSELLATION_STAGE
                 && (unsigned int)ShaderType::tessCtrl == 3 && (unsigned int)ShaderType::tessEval == 4
#              endif
#            else
#              ifndef __P_DISABLE_TESSELLATION_STAGE
                 && (unsigned int)ShaderType::tessCtrl == 2 && (unsigned int)ShaderType::tessEval == 3
#              endif
#            endif
             ,"ShaderType enum values inconsistent with Renderer::bindGraphicsPipeline");

    if (pipeline != nullptr) {
      ID3D11DeviceChild** oldShaderStage = this->_attachedPipeline.shaderStages;
      SharedResource<ID3D11DeviceChild>* newShaderStage = pipeline->shaderStages;

      // -> shader stages + input format
      if (pipeline->topology != this->_attachedPipeline.topology) {
        this->_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)pipeline->topology);
        this->_attachedPipeline.topology = pipeline->topology;
      }
      if (newShaderStage->get() != *oldShaderStage) {
        this->_context->IASetInputLayout(pipeline->inputLayout.get());
        this->_context->VSSetShader((ID3D11VertexShader*)newShaderStage->get(), nullptr, 0);
        *oldShaderStage = newShaderStage->get();
      }
      if ((++newShaderStage)->get() != *(++oldShaderStage)) {
        this->_context->PSSetShader((ID3D11PixelShader*)newShaderStage->get(), nullptr, 0);
        *oldShaderStage = newShaderStage->get();
      }
#     if !defined(__P_DISABLE_GEOMETRY_STAGE)
        if ((++newShaderStage)->get() != *(++oldShaderStage)) {
          this->_context->GSSetShader((ID3D11GeometryShader*)newShaderStage->get(), nullptr, 0);
          *oldShaderStage = newShaderStage->get();
        }
#     endif
#     if !defined(__P_DISABLE_TESSELLATION_STAGE)
        if ((++newShaderStage)->get() != *(++oldShaderStage)) {
          this->_context->HSSetShader((ID3D11HullShader*)newShaderStage->get(), nullptr, 0);
          *oldShaderStage = newShaderStage->get();
        }
        if ((++newShaderStage)->get() != *(++oldShaderStage)) {
          this->_context->DSSetShader((ID3D11DomainShader*)newShaderStage->get(), nullptr, 0);
          *oldShaderStage = newShaderStage->get();
        }
#     endif

      // -> pipeline states
      if (pipeline->rasterizerState.get() != this->_attachedPipeline.rasterizerState) {
        this->_context->RSSetState(pipeline->rasterizerState.get());
        this->_attachedPipeline.rasterizerState = pipeline->rasterizerState.get();
      }
      if (pipeline->depthStencilState.get() != this->_attachedPipeline.depthStencilState
      || pipeline->stencilRef != this->_attachedPipeline.stencilRef) {
        this->_context->OMSetDepthStencilState(pipeline->depthStencilState.get(), (UINT)pipeline->stencilRef);
        this->_attachedPipeline.depthStencilState = pipeline->depthStencilState.get();
        this->_attachedPipeline.stencilRef = pipeline->stencilRef;
      }
      if (pipeline->blendState.get() != this->_attachedPipeline.blendState
      || pipeline->blendFactorId != this->_attachedPipeline.blendFactorId) {
        this->_context->OMSetBlendState(pipeline->blendState.get(), pipeline->blendConstant, 0xFFFFFFFFu);
        this->_attachedPipeline.blendState = pipeline->blendState.get();
        this->_attachedPipeline.blendFactorId = pipeline->blendFactorId;
      }

      // -> viewport(s) / scissor(s)
      if (pipeline->viewportScissorId != this->_attachedPipeline.viewportScissorId) {
        if (pipeline->viewports.data())
          this->_context->RSSetViewports((UINT)pipeline->viewports.size(), pipeline->viewports.data());
        if (pipeline->scissorTests.data())
          this->_context->RSSetScissorRects((UINT)pipeline->scissorTests.size(), pipeline->scissorTests.data());
        this->_attachedPipeline.viewportScissorId = pipeline->viewportScissorId;
      }
    }
    // Empty pipeline -> unbind current
    else {
      this->_context->VSSetShader(nullptr, nullptr, 0);
      this->_context->PSSetShader(nullptr, nullptr, 0);
#     if !defined(__P_DISABLE_GEOMETRY_STAGE)
        this->_context->GSSetShader(nullptr, nullptr, 0);
#     endif
#     if !defined(__P_DISABLE_TESSELLATION_STAGE)
        this->_context->HSSetShader(nullptr, nullptr, 0);
        this->_context->DSSetShader(nullptr, nullptr, 0);
#     endif
      this->_context->IASetInputLayout(nullptr);
      this->_context->RSSetState(nullptr);
      this->_context->OMSetDepthStencilState(nullptr, (UINT)1);
      this->_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFFu);

      memset(&(this->_attachedPipeline), 0, sizeof(_DxPipelineCache));
      this->_attachedPipeline.topology = (VertexTopology)-1;
    }
    this->_attachedPipeline.lastPipeline = pipeline;
  }


// -- pipeline state cache -- --------------------------------------------------

  static pandora::thread::SpinLock g_resourceCacheLock;
  static constexpr inline uint32_t __indexNotFound() noexcept { return (uint32_t)-1; }

  template <typename _Resource, size_t _IdSize>
  static uint32_t __binarySearch(const SharedResourceCache<_Resource,_IdSize>* collec, size_t length,
                                 const SharedResourceId<_IdSize>& target) noexcept {
    if (length == 0)
      return __indexNotFound();

    uint32_t first = 0, last = static_cast<uint32_t>(length)-1u, mid;
    while (first < last) {
      mid = (first + last) >> 1;
      if (collec[mid].id < target)
        first = mid + 1u;
      else
        last = mid;
    }
    return (collec[first].id == target) ? first : __indexNotFound();
  }

  template <typename _Resource, size_t _IdSize>
  static inline void __insertInPlace(LightVector<SharedResourceCache<_Resource,_IdSize> >& collec,
                                     const SharedResourceId<_IdSize>& target, const _Resource& handle) {
    uint32_t insertPos = 0;
    if (!collec.empty()) {
      uint32_t first = 0, last = static_cast<uint32_t>(collec.size()) - 1, mid;
      while (first < last) {
        mid = (first + last) >> 1;
        if (collec[mid].id < target)
          first = mid + 1;
        else
          last = mid;
      }

      if (collec[first].id == target) { // already exists: increase instance counter
        ++(collec[first].instances);
        return;
      }
      insertPos = (collec[first].id > target) ? first : first + 1u; // new entry: store ordered position
    }
    try { collec.insert(insertPos, SharedResourceCache<_Resource, _IdSize>{ target, handle, 1 }); } catch (...) {}
  }

  // ---

  void Renderer::_addRasterizerState(const RasterizerStateId& id, const RasterizerState& handle) {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    __insertInPlace(this->_rasterizerStateCache, id, handle);
  }
  void Renderer::_addDepthStencilState(const DepthStencilStateId& id, const DepthStencilState& handle) {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    __insertInPlace(this->_depthStencilStateCache, id, handle);
  }
  void Renderer::_addBlendState(const BlendStateId& id, const BlendState& handle) {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    __insertInPlace(this->_blendStateCache, id, handle);
  }

  void Renderer::_removeRasterizerState(const RasterizerStateId& id) noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    uint32_t index = __binarySearch(this->_rasterizerStateCache.data(), this->_rasterizerStateCache.size(), id);
    if (index != __indexNotFound() && --(this->_rasterizerStateCache[index].instances) < 1)
      this->_rasterizerStateCache.erase(index);
  }
  void Renderer::_removeDepthStencilState(const DepthStencilStateId& id) noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    uint32_t index = __binarySearch(this->_depthStencilStateCache.data(), this->_depthStencilStateCache.size(), id);
    if (index != __indexNotFound() && --(this->_depthStencilStateCache[index].instances) < 1)
      this->_depthStencilStateCache.erase(index);
  }
  void Renderer::_removeBlendState(const BlendStateId& id) noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    uint32_t index = __binarySearch(this->_blendStateCache.data(), this->_blendStateCache.size(), id);
    if (index != __indexNotFound() && --(this->_blendStateCache[index].instances) < 1)
      this->_blendStateCache.erase(index);
  }

  // ---

  bool Renderer::_findRasterizerState(const RasterizerStateId& id, RasterizerState& out) const noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    uint32_t index = __binarySearch(this->_rasterizerStateCache.data(), this->_rasterizerStateCache.size(), id);
    if (index != __indexNotFound()) {
      out = this->_rasterizerStateCache[index].handle;
      return true;
    }
    return false;
  }
  bool Renderer::_findDepthStencilState(const DepthStencilStateId& id, DepthStencilState& out) const noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    uint32_t index = __binarySearch(this->_depthStencilStateCache.data(), this->_depthStencilStateCache.size(), id);
    if (index != __indexNotFound()) {
      out = this->_depthStencilStateCache[index].handle;
      return true;
    }
    return false;
  }
  bool Renderer::_findBlendState(const BlendStateId& id, BlendState& out) const noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_resourceCacheLock);
    uint32_t index = __binarySearch(this->_blendStateCache.data(), this->_blendStateCache.size(), id);
    if (index != __indexNotFound()) {
      out = this->_blendStateCache[index].handle;
      return true;
    }
    return false;
  }
  

// -----------------------------------------------------------------------------
// Include hpp implementations
// -----------------------------------------------------------------------------
# include "./swap_chain.hpp"
# include "./_error.hpp"
# include "./graphics_pipeline.hpp"
# include "./depth_stencil_buffer.hpp"
# include "./buffer.hpp"
# include "./texture.hpp"
# include "./texture_reader_writer.hpp"
# include "./shader.hpp"
# include "./camera_utils.hpp"
# undef __if_constexpr

# ifndef __MINGW32__
#   pragma warning(pop)
# endif
#endif
