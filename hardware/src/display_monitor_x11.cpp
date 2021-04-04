/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - X11 implementation (Linux/BSD)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdint>
# include <cstdlib>
# include <string>
# include <stdexcept>
# include <set>
# include <vector>
# include <climits>
# include <cmath>
# include <unistd.h>
# include <X11/Xlib.h>
# include "hardware/_private/_libraries_x11.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;


// -- monitor handle & description/attributes -- -------------------------------

  static inline bool __fillAdapterName(RROutput handle, XRRScreenResources* resources, DisplayMonitor::String& outName) {
    LibrariesX11& libs = *(LibrariesX11::instance()); // XRR params -> libs already instanced
    if (!libs.randr.GetProviderInfo_)
      return false;
    
    bool isAdapterFound = false;
    if (XRRProviderResources* providers = libs.randr.GetProviderResources_(libs.displayServer, libs.rootWindow)) {
      for (int p = 0; !isAdapterFound && p < providers->nproviders; ++p) {
        
        if (XRRProviderInfo* provider = libs.randr.GetProviderInfo_(libs.displayServer, resources, providers->providers[p])) {
          for (int out = 0; out < provider->noutputs; ++out) {
            if (provider->outputs[out] == handle) {
              outName = provider->name;
              isAdapterFound = true;
              break;
            }
          }
          libs.randr.FreeProviderInfo_(provider);
        }
      }
      libs.randr.FreeProviderResources_(providers);
    }
    return isAdapterFound;
  }
  
  void __fillMonitorWorkArea(XRRCrtcInfo* crtc, XRRScreenResources* resources, DisplayArea& outWorkArea) {
    LibrariesX11& libs = *(LibrariesX11::instance()); // XRR params -> libs already instanced
    outWorkArea.x = crtc->x;
    outWorkArea.y = crtc->y;

    XRRModeInfo* modeInfo = nullptr;
    for (int m = 0; m < resources->nmode; ++m) {
      if (resources->modes[m].id == crtc->mode) {
        modeInfo = resources->modes + m;
        break;
      }
    }
    if (modeInfo != nullptr) {
      outWorkArea.width = modeInfo->width;
      outWorkArea.height = modeInfo->height;
    }
    else {
      outWorkArea.width = crtc->width;
      outWorkArea.height = crtc->height;
    }

    if (libs.atoms.NET_WORKAREA && libs.atoms.NET_CURRENT_DESKTOP) {
      Atom* workArea = nullptr;
      uint32_t workAreaLength = libs.getWindowProperty(libs.rootWindow, libs.atoms.NET_WORKAREA, 
                                                       XA_CARDINAL, (unsigned char**)&workArea);
      if (workArea == nullptr)
        return;
      
      Atom* currentDesktop = nullptr;
      if (libs.getWindowProperty(libs.rootWindow, libs.atoms.NET_CURRENT_DESKTOP,
                                 XA_CARDINAL, (unsigned char**)&currentDesktop) > 0) {
        if (workAreaLength >= 4u && *currentDesktop < (workAreaLength >> 2)) {
          DisplayArea globalArea { static_cast<int32_t>(workArea[(*currentDesktop << 2) + 0]), 
                                   static_cast<int32_t>(workArea[(*currentDesktop << 2) + 1]), 
                                   static_cast<uint32_t>(workArea[(*currentDesktop << 2) + 2]), 
                                   static_cast<uint32_t>(workArea[(*currentDesktop << 2) + 3]) };
          if (outWorkArea.x < globalArea.x) {
            outWorkArea.width -= static_cast<uint32_t>(globalArea.x - outWorkArea.x);
            outWorkArea.x = globalArea.x;
          }
          if (outWorkArea.y < globalArea.y) {
            outWorkArea.height -= static_cast<uint32_t>(globalArea.y - outWorkArea.y);
            outWorkArea.y = globalArea.y;
          }
          if (outWorkArea.x + outWorkArea.width > globalArea.x + globalArea.width)
            outWorkArea.width = static_cast<uint32_t>((int32_t)globalArea.width + globalArea.x - outWorkArea.x);
          if (outWorkArea.y + outWorkArea.height > globalArea.y + globalArea.height)
            outWorkArea.height = static_cast<uint32_t>((int32_t)globalArea.height + globalArea.y - outWorkArea.y);
        }
        libs.xlib.Free_(currentDesktop);
      }
      libs.xlib.Free_(workArea);
    }
  }

  static bool __fillOutputInfoAttributes(RROutput handle, XRROutputInfo& outputInfo, XRRScreenResources* resources, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
    LibrariesX11& libs = *(LibrariesX11::instance()); // XRR params -> libs already instanced
    if (outputInfo.connection != RR_Connected || outputInfo.crtc == None)
      return false;
    
    XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, outputInfo.crtc);
    if (crtc == nullptr)
      return false;
    
    outController = outputInfo.crtc;
    outAttr.id = outAttr.description = outputInfo.name;
    __fillAdapterName(handle, resources, outAttr.adapter);
    
    outAttr.screenArea.x = crtc->x;
    outAttr.screenArea.y = crtc->y;
    outAttr.screenArea.width = crtc->width;
    outAttr.screenArea.height = crtc->height;
    __fillMonitorWorkArea(crtc, resources, outAttr.workArea);

    libs.randr.FreeCrtcInfo_(crtc);
    return true;   
  }
  
  // ---------------------------------------------------------------------------

  static bool __readDisplayMonitorAttributes(RROutput handle, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
    LibrariesX11& libs = *(LibrariesX11::instance()); // XRR params -> libs already instanced
    if (handle && libs.randr.isAvailable) {
      if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
        bool isSuccess = false;
        
        if (XRROutputInfo* outInfo = libs.randr.GetOutputInfo_(libs.displayServer, resources, handle)) {
          isSuccess = __fillOutputInfoAttributes(handle, *outInfo, resources, outController, outAttr);
          libs.randr.FreeOutputInfo_(outInfo);
        }
        libs.randr.FreeScreenResources_(resources);
        return isSuccess;
      }
    }
    return false; 
  }
  
  static inline bool _readDisplayMonitorAttributes(DisplayMonitor::Handle monitorHandle, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs != nullptr && libs->randr.isAvailable) {
      outAttr.isPrimary = ((RROutput)monitorHandle == libs->randr.GetOutputPrimary_(libs->displayServer, libs->rootWindow));
      return __readDisplayMonitorAttributes((RROutput)monitorHandle, outController, outAttr);
    }
    return false;
  }
  static inline void _readPrimaryDisplayMonitorInfo(DisplayMonitor::Handle& outHandle, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs != nullptr && libs->randr.isAvailable) {
      RROutput primaryOutput = libs->randr.GetOutputPrimary_(libs->displayServer, libs->rootWindow);
      outHandle = (DisplayMonitor::Handle)primaryOutput;

      __readDisplayMonitorAttributes((RROutput)outHandle, outController, outAttr);
    }
    outAttr.isPrimary = true;
  }
  
  // ---------------------------------------------------------------------------
  
  static DisplayMonitor::Handle _getDisplayMonitorById(DisplayMonitor::DeviceId id, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs != nullptr && libs->randr.isAvailable) {
      if (XRRScreenResources* resources = libs->randr.GetScreenResourcesCurrent_(libs->displayServer, libs->rootWindow)) {
        
        DisplayMonitor::Handle handle = (DisplayMonitor::Handle)0;
        for (int out = 0; handle == (DisplayMonitor::Handle)0 && out < resources->noutput; ++out) {
          
          if (XRROutputInfo* outInfo = libs->randr.GetOutputInfo_(libs->displayServer, resources, resources->outputs[out])) {
            if (id == outInfo->name && __fillOutputInfoAttributes(resources->outputs[out], *outInfo, resources, outController, outAttr))
              handle = (DisplayMonitor::Handle)resources->outputs[out];
            libs->randr.FreeOutputInfo_(outInfo);
          }
        }
        libs->randr.FreeScreenResources_(resources);
        return handle;
      }
    }
    return (DisplayMonitor::Handle)0;
  }
  
  static bool _listDisplayMonitors(std::vector<DisplayMonitor::Handle>& resultsOut) {
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs != nullptr && libs->randr.isAvailable) {
      if (XRRScreenResources* resources = libs->randr.GetScreenResourcesCurrent_(libs->displayServer, libs->rootWindow)) {
        
        for (int out = 0; out < resources->noutput; ++out) {
          if (XRROutputInfo* outInfo = libs->randr.GetOutputInfo_(libs->displayServer, resources, resources->outputs[out])) {
            if (outInfo->connection == RR_Connected && outInfo->crtc != None)
              resultsOut.emplace_back((DisplayMonitor::Handle)resources->outputs[out]);
            libs->randr.FreeOutputInfo_(outInfo);
          }
        }
        libs->randr.FreeScreenResources_(resources);
        return true;
      }
    }
    return false; 
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    _readPrimaryDisplayMonitorInfo(this->_handle, this->_controller, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (!this->_handle || !_readDisplayMonitorAttributes(this->_handle, this->_controller, this->_attributes)) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = _getDisplayMonitorById(id, this->_controller, this->_attributes);
    if (!this->_handle) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    std::vector<DisplayMonitor::Handle> handles;
    if (_listDisplayMonitors(handles) && index < handles.size())
      this->_handle = handles[index];
    if (!this->_handle || !_readDisplayMonitorAttributes(this->_handle, this->_controller, this->_attributes)) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitors;

    std::vector<DisplayMonitor::Handle> handles;
    if (_listDisplayMonitors(handles)) {
      for (auto it : handles) {
        try {
          monitors.emplace_back(it, false);
        }
        catch (...) {}
      }
    }

    if (monitors.empty()) { // primary monitor as default
      monitors.emplace_back();
      if (monitors[0]._handle == (DisplayMonitor::Handle)0) // no display monitor
        monitors.clear();
    }
    return monitors;
  }


// -- display modes -- ---------------------------------------------------------

  static inline uint32_t __getRefreshRate(XRRModeInfo& modeInfo) {
    if (modeInfo.hTotal && modeInfo.vTotal) {
      double rate = (double)modeInfo.dotClock / ((double)modeInfo.hTotal * (double)modeInfo.vTotal);
      return static_cast<uint32_t>(rate + 0.0000001); // show "59.94" as "59", to distinguish it from "60"
    }
    return undefinedRefreshRate();
  }
  static inline uint32_t __getBitDepth(LibrariesX11& libs) {
    int depth = DefaultDepth(libs.displayServer, libs.screenIndex);
    return (depth > 0) ? depth : 32;
  }
  
  static inline bool __findDisplayModeId(DisplayMode desiredMode, LibrariesX11& libs, XRROutputInfo* outInfo, XRRScreenResources* resources, RRMode& outMode) {
    std::set<RRMode> supportedModes;
    for (int m = 0; m < outInfo->nmode; ++m)
      supportedModes.emplace(outInfo->modes[m]);
    
    for (int m = 0; m < resources->nmode; ++m) {
      XRRModeInfo& modeInfo = resources->modes[m];
      
      if (supportedModes.find(modeInfo.id) != supportedModes.end() && (modeInfo.modeFlags & RR_Interlace) == 0 // supported
          && modeInfo.width == desiredMode.width && modeInfo.height == desiredMode.height // appropriate dimensions
          && __getRefreshRate(modeInfo) == desiredMode.refreshRate
          && __getBitDepth(libs) == desiredMode.bitDepth) {
        outMode = modeInfo.id;
        return true;
      }
    }
    return false;
  }

  static inline bool _getMonitorDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller, DisplayMode& out) noexcept { 
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs == nullptr || !libs->randr.isAvailable || controller == (DisplayMonitor::Handle)0)
      return false;

    bool isSuccess = false;
    if (XRRScreenResources* resources = libs->randr.GetScreenResourcesCurrent_(libs->displayServer, libs->rootWindow)) {
      if (XRRCrtcInfo* crtc = libs->randr.GetCrtcInfo_(libs->displayServer, resources, (RRCrtc)controller)) {
        
        for (int m = 0; m < resources->nmode; ++m) {
          if (resources->modes[m].id == crtc->mode) {
            XRRModeInfo* modeInfo = resources->modes + m;
            out.width = modeInfo->width;
            out.height = modeInfo->height;
            out.refreshRate = __getRefreshRate(*modeInfo);
            out.bitDepth = __getBitDepth(*libs);
            
            isSuccess = true;
            break;
          }
        }
        libs->randr.FreeCrtcInfo_(crtc);
      }
      libs->randr.FreeScreenResources_(resources);
    }
    return isSuccess;
  }
  
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!_getMonitorDisplayMode(this->_handle, this->_controller, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  // ---------------------------------------------------------------------------
  
  static inline bool _setMonitorDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller, const DisplayMode& desiredMode) { 
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs == nullptr || !libs->randr.isAvailable || controller == (DisplayMonitor::Handle)0)
      return false;

    bool isSuccess = false;
    if (XRRScreenResources* resources = libs->randr.GetScreenResourcesCurrent_(libs->displayServer, libs->rootWindow)) {
      if (XRRCrtcInfo* crtc = libs->randr.GetCrtcInfo_(libs->displayServer, resources, (RRCrtc)controller)) {
        if (XRROutputInfo* outInfo = libs->randr.GetOutputInfo_(libs->displayServer, resources, (RROutput)handle)) {
          
          RRMode originalMode = crtc->mode;
          RRMode nativeTargetMode = 0;
          if (__findDisplayModeId(desiredMode, *libs, outInfo, resources, nativeTargetMode)
           && libs->randr.SetCrtcConfig_(libs->displayServer, resources, (RRCrtc)controller, CurrentTime,
                                        crtc->x, crtc->y, nativeTargetMode, crtc->rotation, 
                                        crtc->outputs, crtc->noutput)) {
            if (libs->originalModes.find((RROutput)handle) == libs->originalModes.end())
              libs->originalModes.emplace((RROutput)handle, originalMode); // store original display mode (if it hasn't already been changed)
            isSuccess = true;
          }
          libs->randr.FreeOutputInfo_(outInfo);
        }
        libs->randr.FreeCrtcInfo_(crtc);
      }
      libs->randr.FreeScreenResources_(resources);
    }
    return isSuccess;
  }
  
  static inline bool _setDefaultMonitorDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller) { 
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs == nullptr || !libs->randr.isAvailable || controller == (DisplayMonitor::Handle)0)
      return false;
    if (libs->originalModes.find((RROutput)handle) == libs->originalModes.end()) // no previous display change -> already default mode
      return true; 

    bool isSuccess = false;
    if (XRRScreenResources* resources = libs->randr.GetScreenResourcesCurrent_(libs->displayServer, libs->rootWindow)) {
      if (XRRCrtcInfo* crtc = libs->randr.GetCrtcInfo_(libs->displayServer, resources, (RRCrtc)controller)) {
        
        if (libs->randr.SetCrtcConfig_(libs->displayServer, resources, (RRCrtc)controller, CurrentTime,
                                      crtc->x, crtc->y, libs->originalModes[(RROutput)handle], 
                                      crtc->rotation, crtc->outputs, crtc->noutput)) {
          libs->originalModes.erase((RROutput)handle);
          isSuccess = true;
        }
        libs->randr.FreeCrtcInfo_(crtc);
      }
      libs->randr.FreeScreenResources_(resources);
    }
    return isSuccess;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) {
    if (_setMonitorDisplayMode(this->_handle, this->_controller, mode)) {
      if (refreshAttributes) {
        if (this->_attributes.isPrimary) {
          _readPrimaryDisplayMonitorInfo(this->_handle, this->_controller, this->_attributes);
        }
        else if (!_readDisplayMonitorAttributes(this->_handle, this->_controller, this->_attributes)) {
          this->_attributes.screenArea.width = mode.width;
          this->_attributes.screenArea.height = mode.height;
        }
      }
      return true;
    }
    return false;
  }
  
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) {
    if (_setDefaultMonitorDisplayMode(this->_handle, this->_controller)) {
      if (refreshAttributes) {
        if (this->_attributes.isPrimary) {
          _readPrimaryDisplayMonitorInfo(this->_handle, this->_controller, this->_attributes);
        }
        else
          _readDisplayMonitorAttributes(this->_handle, this->_controller, this->_attributes);
      }
      return true;
    }
    return false;
  }
  
  // ---------------------------------------------------------------------------

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    std::vector<DisplayMode> modes;
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs == nullptr || !libs->randr.isAvailable || this->_controller == (DisplayMonitor::Handle)0)
      return modes;

    if (XRRScreenResources* resources = libs->randr.GetScreenResourcesCurrent_(libs->displayServer, libs->rootWindow)) {
      if (XRRCrtcInfo* crtc = libs->randr.GetCrtcInfo_(libs->displayServer, resources, (RRCrtc)this->_controller)) {
        if (XRROutputInfo* outInfo = libs->randr.GetOutputInfo_(libs->displayServer, resources, (RROutput)this->_handle)) {
          std::set<RRMode> supportedModes;
          for (int m = 0; m < outInfo->nmode; ++m)
            supportedModes.emplace(outInfo->modes[m]);
        
          for (int m = 0; m < resources->nmode; ++m) {
            XRRModeInfo* modeInfo = resources->modes + m;
            if (supportedModes.find(modeInfo->id) != supportedModes.end() && (modeInfo->modeFlags & RR_Interlace) == 0) {

              DisplayMode data;
              data.width = modeInfo->width;
              data.height = modeInfo->height;
              data.refreshRate = __getRefreshRate(*modeInfo);
              data.bitDepth = __getBitDepth(*libs);
              modes.emplace_back(std::move(data));
            }
          }
          libs->randr.FreeOutputInfo_(outInfo);
        }
        libs->randr.FreeCrtcInfo_(crtc);
      }
      libs->randr.FreeScreenResources_(resources);
    }
    return modes;
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs == nullptr)
      return false;
    
    if (isEnabled) {
      if (libs->displayServer == nullptr)
        return false;
      libs->readSystemDpi();
    }
    else
      libs->dpiX = libs->dpiY = __P_HARDWARE_X11_BASE_DPI;
    return true;
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle) const noexcept {
    if (LibrariesX11* libs = LibrariesX11::instance()) {
      outDpiX = static_cast<uint32_t>(libs->dpiX + 0.5f); // round value
      outDpiY = static_cast<uint32_t>(libs->dpiY + 0.5f);
    }
    else
      outDpiX = outDpiY = static_cast<uint32_t>(__P_HARDWARE_X11_BASE_DPI);
  }
  
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle) const noexcept {
    if (LibrariesX11* libs = LibrariesX11::instance()) {
      outScaleX = libs->dpiX / __P_HARDWARE_X11_BASE_DPI;
      outScaleY = libs->dpiY / __P_HARDWARE_X11_BASE_DPI;}
    else
      outScaleX = outScaleY = 1.0f;
  }


// -- metrics -- ---------------------------------------------------------------

  static inline int32_t __getBorderSize(DisplayMonitor::WindowHandle windowHandle) noexcept {
    LibrariesX11& libs = *(LibrariesX11::instance());
    XWindowAttributes attributes;
    libs.xlib.GetWindowAttributes_(libs.displayServer, (windowHandle != (DisplayMonitor::WindowHandle)0) ? windowHandle : libs.rootWindow, &attributes);
    return attributes.border_width;
  }

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, 
                                                            bool hasMenu, uint32_t hasBorders, uint32_t hasCaption) const noexcept {
    LibrariesX11* libs = LibrariesX11::instance();
    if (libs == nullptr)
      return clientArea;

    // read window size with extents (if supported)
    if (libs->atoms.NET_FRAME_EXTENTS && windowHandle != (DisplayMonitor::WindowHandle)0) {
      // wait until extents are set up by window manager
      int32_t* extents = nullptr;
      if (libs->getWindowProperty(windowHandle, libs->atoms.NET_FRAME_EXTENTS, AnyPropertyType, 
                                 (unsigned char**)&extents, 4, 1024) > 0) {
        DisplayArea windowArea { extents[0], extents[2], static_cast<uint32_t>(extents[0]+extents[1]), static_cast<uint32_t>(extents[2]+extents[3]) };
        libs->xlib.Free_(extents);
        return windowArea;
      }
    }
    
    // fallback if not supported
    int32_t borderSize = (hasBorders) ? __getBorderSize(windowHandle) : 0;
    int32_t captionSize = (hasCaption && windowHandle != (DisplayMonitor::WindowHandle)0) ? 24 : 0;
    
    DisplayArea windowArea = clientArea;
    windowArea.x -= borderSize;
    if (clientArea.x >= 0 && windowArea.x < 0)
      windowArea.x = 0;
    windowArea.width += borderSize << 1;
    
    windowArea.y -= (borderSize + captionSize) >> 1;
    if (clientArea.y >= 0 && windowArea.y < 0)
      windowArea.y = 0;
    windowArea.height += borderSize + captionSize;
    return windowArea;
  }
#endif
