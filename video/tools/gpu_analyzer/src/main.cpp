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
Local hardware specifications analyzer (CPU, monitors, display adapters)
*******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1
#include <cstdio>
#include <hardware/cpu_specs.h>
#include <hardware/display_monitor.h>
#if defined(_WINDOWS)
# include <system/api/windows_app.h>
# include <system/api/windows_api.h>
# if defined(_VIDEO_D3D11_SUPPORT)
#   include <video/window.h>
#   include <video/d3d11/renderer.h>
#   include <video/d3d11/texture.h>
# endif
# define _SYSTEM_STR(str) L"" str
#else
# define _SYSTEM_STR(str) str
#endif
#if defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
#endif
#if defined(_VIDEO_OPENGL4_SUPPORT)
//# include <video/openGL4/renderer.h>
#endif

#if defined(__ANDROID__)
# include <stdexcept>
# include <android/log.h>
# include <system/api/android_app.h>
# define printf(...) __android_log_print(ANDROID_LOG_INFO, "-", __VA_ARGS__)
# ifndef LOGE
#   define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , ">", __VA_ARGS__)
# endif
#endif

using namespace pandora::hardware;
using namespace pandora::video;

// -- console helpers -- -------------------------------------------------------

// Boolean serialization
static const char* __labelTrue = "true";
static const char* __labelFalse = "false";
inline const char* toString(bool value) noexcept { return value ? __labelTrue : __labelFalse; }

// Clear all traces in console
void clearScreen() {
# ifdef _WINDOWS
    system("cls");
# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
    if (system("clear") == -1)
      printf("\n____________________________________________________________\n");
# endif
}

// Display menu (first entry should be return/exit command)
template <size_t _Size>
inline void printMenu(const std::array<std::string, _Size>& items) {
  for (int i = 1; i < static_cast<int>(_Size); ++i)
    printf("> %d - %s\n", i, items[i].c_str());
  printf("> 0 - %s\n\n", items[0].c_str());
}

// print main title
inline void printTitle(const char* suffix) {
  printf("\n GPU SPECIFICATIONS ANALYZER %s\n"
         "____________________________________________________________\n\n", suffix);
}

// print "back to menu" message
inline void printReturn() {
  printf("\n____________________________________________________________\n\n"
         "> Press ENTER to return to menu...\n"); 
  while (getchar() != '\n'); // wait for input + clear buffer
}

// Get numeric user input
inline int readNumericInput(int minValue, int maxValue) noexcept {
  int val = -1;
  bool isValid = false;
  printf("Enter a value (%d-%d, or 0) :\n> ", minValue, maxValue);

  do {
    isValid = (scanf("%d", &val) > 0 && (val == 0 || (val >= minValue && val <= maxValue)) );
    if (!isValid) {
      while(getchar() != '\n');
      printf("Invalid value. Please try again (%d-%d or 0) :\n> ", minValue, maxValue);
    }
  } while (!isValid);

  while (getchar() != '\n'); // clear buffer
  return val;
}

// -- renderer info readers -- -------------------------------------------------

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  void displaySpecs_d3d11() {
    clearScreen();
    printTitle("- Direct3D 11.x");
    pandora::hardware::DisplayMonitor monitor;
    d3d11::Renderer renderer(monitor);

    // API level
    uint32_t featureLevel = (renderer.featureLevel() == D3D_FEATURE_LEVEL_11_0) ? 0 : 1u;
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      if (renderer.dxgiLevel() > 2u) {
        switch (renderer.dxgiLevel()) {
#       if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
          case 3u: featureLevel = 2u; break;
          case 4u: featureLevel = 3u; break;
          case 5u:
          case 6u:
          default: featureLevel = 4u; break;
#       else
          default: featureLevel = 2u; break;
#       endif
        }
      }
#   endif

    // memory
    size_t dedicatedRam = 0, sharedRam = 0;
    renderer.getAdapterVramSize(dedicatedRam, sharedRam);

    // color support
    UINT buffer = 0;
    auto monitorColorSpace = renderer.getMonitorColorSpace(monitor);
    bool isHdrMonitor = (monitorColorSpace == d3d11::ColorSpace::hdr10_bt2084 || monitorColorSpace == d3d11::ColorSpace::scRgb);
    bool iHdrScRGB = (SUCCEEDED(renderer.device()->CheckFormatSupport(_getDataFormatComponents(d3d11::DataFormat::rgba16_f_scRGB), &buffer))
                     && (buffer & D3D11_FORMAT_SUPPORT_RENDER_TARGET) );
    bool iHdr10 = (SUCCEEDED(renderer.device()->CheckFormatSupport(_getDataFormatComponents(d3d11::DataFormat::rgb10a2_unorm_hdr10), &buffer))
                     && (buffer & D3D11_FORMAT_SUPPORT_RENDER_TARGET) );

    bool isSrgbBlendingSupported = false;
    d3d11::DataFormat srgbFormats[]{ d3d11::DataFormat::rgba8_sRGB, d3d11::DataFormat::bgra8_sRGB };
    auto srgbFormat = renderer.findSupportedDataFormat(srgbFormats, 2u, d3d11::FormatAttachment::colorBlend);
    isSrgbBlendingSupported = (srgbFormat != d3d11::DataFormat::unknown);
    if (!isSrgbBlendingSupported) {
      srgbFormat = renderer.findSupportedDataFormat(srgbFormats, 2u, d3d11::FormatAttachment::color);
    }
    const char* sRgbSupport = isSrgbBlendingSupported ? "color/blending" : ((srgbFormat != d3d11::DataFormat::unknown) ? "color only" : "none");
    bool isTearingSupported = renderer.isTearingAvailable();

    // anti-aliasing support
    uint32_t maxMSAA = 64u;
    while ((!renderer.isColorSampleCountAvailable(d3d11::DataFormat::rgba8_unorm, maxMSAA)
      || !renderer.isDepthSampleCountAvailable(d3d11::DepthStencilFormat::d32_f, maxMSAA)) && maxMSAA > 1u)
      maxMSAA >>= 1;

    printf(" Hardware capabilities:\n\n"
           " > Direct3D 11.%u\n > DXGI level: 1.%u\n > GPU VRAM:   %.3f MB\n > Shared RAM: %.3f MB\n\n"
           " Available features:\n\n"
           " > sRGB format support: %s\n > HDR-scRGB support:   %s\n > HDR-10 support:      %s\n > HDR-capable monitor: %s\n\n"
           " > Screen tearing support:  %s\n > Multi-viewport support:  %s (max %u)\n"
           " > MSAA support:            %s (max %u\x78)\n > Max anisotropy level:    %u\n > Max render-target count: %u"
           ""
           "",
           featureLevel, renderer.dxgiLevel(), (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
           sRgbSupport, toString(iHdrScRGB), toString(iHdr10), toString(isHdrMonitor),
           toString(isTearingSupported), toString(renderer.maxViewports() > 1u), (uint32_t)renderer.maxViewports(),
           toString(maxMSAA > 1u), maxMSAA, d3d11::SamplerParams::maxAnisotropy(), (uint32_t)renderer.maxRenderTargets());
  }
#endif

#if defined(_VIDEO_VULKAN_SUPPORT)
  void displaySpecs_vulkan() {
    clearScreen();
    printTitle("- Vulkan 1.x");
    pandora::hardware::DisplayMonitor monitor;

    uint32_t vulkanVersions[] = {
#     if (!defined(NTDDI_WIN10_RS2) || NTDDI_VERSION >= NTDDI_WIN10_RS2) && defined(VK_HEADER_VERSION) && VK_HEADER_VERSION >= 204
        VK_API_VERSION_1_3,
#     endif
      VK_API_VERSION_1_2, VK_API_VERSION_1_1, VK_API_VERSION_1_0
    };
    std::shared_ptr<vulkan::VulkanInstance> vulkanInstance = nullptr;
    for (uint32_t i = 0; i < sizeof(vulkanVersions) / sizeof(*vulkanVersions) && vulkanInstance == nullptr; ++i) {
      vulkanInstance = vulkan::VulkanInstance::create("gpu_analyzer", VK_MAKE_VERSION(1, 0, 0), vulkanVersions[i]);
    }
    auto features = vulkan::Renderer::defaultFeatures();
    features.variableMultisampleRate = true;
    auto renderer = std::make_shared<vulkan::Renderer>(monitor, vulkanInstance);

    // API level
    uint32_t featureLevel = VK_VERSION_MINOR(renderer->featureLevel());

    // memory
    size_t dedicatedRam = 0, sharedRam = 0;
    renderer->getAdapterVramSize(dedicatedRam, sharedRam);

    // color support
    bool iHdrScRGB = false, isHdr10 = false;
    Window::Builder builder;
    auto window = builder.setDisplayMode(WindowType::window, WindowBehavior::none, ResizeMode::fixed).setSize(400, 300)
                         .create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Test Window"));
    {
      vulkan::DisplaySurface surface(renderer, window->handle());
      iHdrScRGB = surface.isFormatSupported(vulkan::DataFormat::rgba16_f_scRGB);
      isHdr10 = surface.isFormatSupported(vulkan::DataFormat::rgb10a2_unorm_hdr10);
    }
    window = nullptr;

    bool isSrgbBlendingSupported = false;
    vulkan::DataFormat srgbFormats[]{ vulkan::DataFormat::rgba8_sRGB, vulkan::DataFormat::bgra8_sRGB };
    auto srgbFormat = renderer->findSupportedDataFormat(srgbFormats, 2u, vulkan::FormatAttachment::colorBlend);
    isSrgbBlendingSupported = (srgbFormat != vulkan::DataFormat::unknown);
    if (!isSrgbBlendingSupported) {
      srgbFormat = renderer->findSupportedDataFormat(srgbFormats, 2u, vulkan::FormatAttachment::color);
    }
    const char* sRgbSupport = isSrgbBlendingSupported ? "color/blending" : ((srgbFormat != vulkan::DataFormat::unknown) ? "color only" : "none");
    bool isTearingSupported = renderer->isTearingAvailable();

    // anti-aliasing support
    uint32_t maxMSAA = 64u;
    while ((!renderer->isColorSampleCountAvailable(vulkan::DataFormat::rgba8_unorm, maxMSAA)
         || !renderer->isDepthSampleCountAvailable(vulkan::DepthStencilFormat::d32_f, maxMSAA)) && maxMSAA > 1u)
      maxMSAA >>= 1;

    printf(" Hardware capabilities:\n\n"
           " > Vulkan 1.%u\n > GPU VRAM:   %.3f MB\n > Shared RAM: %.3f MB\n"
           " > Dynamic rendering support: %s\n > Extended dynamic states:   %s\n\n"
           " Available features:\n\n"
           " > sRGB format support: %s\n > HDR-scRGB support:   %s\n > HDR-10 support:      %s\n > HDR-capable monitor: %s\n\n"
           " > Screen tearing support:  %s\n > Multi-viewport support:  %s (max %u)\n"
           " > MSAA support:            %s (max %u\x78)\n > Max anisotropy level:    %u\n > Max render-target count: %u"
           ""
           "",
           featureLevel, (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
           toString(renderer->isDynamicRenderingSupported()), toString(renderer->isExtendedDynamicStateSupported()),
           sRgbSupport, toString(iHdrScRGB), toString(isHdr10), "unknown",
           toString(isTearingSupported), toString(renderer->maxViewports() > 1u), (uint32_t)renderer->maxViewports(),
           toString(maxMSAA > 1u), maxMSAA, (uint32_t)renderer->deviceLimits().maxSamplerAnisotropy, (uint32_t)renderer->maxRenderTargets());
  }
#endif

#if defined(_VIDEO_OPENGL4_SUPPORT)
  void displaySpecs_openGL4() {
    clearScreen();
    printTitle("- OpenGL 4.x");

    printf(" Not yet supported...\n");
  }
#endif


// ---

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# define __ENTRY_D3D11 1
#else
# define __ENTRY_D3D11 0
#endif
#if defined(_VIDEO_VULKAN_SUPPORT)
# define __ENTRY_VULKAN 1
#else
# define __ENTRY_VULKAN 0
#endif
#if defined(_VIDEO_OPENGL4_SUPPORT)
# define __ENTRY_OPENGL4 1
#else
# define __ENTRY_OPENGL4 0
#endif
#define __MENU_ENTRY_COUNT (__ENTRY_D3D11+__ENTRY_VULKAN+__ENTRY_OPENGL4)

#if defined(__ANDROID__)
  // GPU analysis - entry point for Android
  void android_main(struct android_app* state) {
    try {
      pandora::system::AndroidApp::instance().init(state);
      printf(" Not yet supported on Android...\n"); // not implemented yet
    }
    catch (const std::exception& exc) { LOGE("%s", exc.what()); }
  }
  
#else
  // GPU analysis - entry point
  int main() {
#   ifdef _WINDOWS
      pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)GetModuleHandle(nullptr));
#   endif

    bool isRunning = true;
    while (isRunning) {
      clearScreen();
      printTitle("");

      printf("\nGraphics API :\n");
      printMenu<__MENU_ENTRY_COUNT + 1>({
        "Exit..."
#       if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
          ,"Direct3D 11.x"
#       endif
#       if defined(_VIDEO_VULKAN_SUPPORT)
          ,"Vulkan 1.x"
#       endif
#       if defined(_VIDEO_OPENGL4_SUPPORT)
          ,"OpenGL 4.x"
#       endif
      });
      
      int option = readNumericInput(1, __MENU_ENTRY_COUNT);
      try {
        switch (option) {
#         if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
            case 1: displaySpecs_d3d11(); break;
#         endif
#         if defined(_VIDEO_VULKAN_SUPPORT)
            case (__ENTRY_D3D11+1): displaySpecs_vulkan(); break;
#         endif
#         if defined(_VIDEO_OPENGL4_SUPPORT)
            case (__ENTRY_D3D11+__ENTRY_VULKAN+1): displaySpecs_openGL4(); break;
#         endif
          case 0:
          default: isRunning = false; break;
        }
      }
      catch (const std::exception& exc) { printf(" EXCEPTION: %s", exc.what()); }
      if (option != 0)
        printReturn();
    }
    return 0;
  }
#endif
