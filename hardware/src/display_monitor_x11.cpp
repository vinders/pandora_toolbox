/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - X11 implementation (Linux/BSD)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
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


// -- monitor attributes - id/area/description/primary -- ----------------------

  namespace attributes {
    // verify if a monitor is the primary device
    static inline bool _isPrimaryDevice(LibrariesX11& libs, RROutput handle) {
      return (handle == libs.randr.GetOutputPrimary_(libs.displayServer, libs.rootWindow));
    }
    
    // read brand/description string of adapter
    static inline std::string _readAdapterName(RROutput handle) {
      std::string adapterName;
      
      LibrariesX11& libs = LibrariesX11::instance();
      if (handle && libs.randr.GetProviderInfo_) {
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          if (XRRProviderResources* providers = libs.randr.GetProviderResources_(libs.displayServer, libs.rootWindow)) {
            
            for (int p = 0; adapterName.empty() && p < providers->nproviders; ++p) {
              if (XRRProviderInfo* provider = libs.randr.GetProviderInfo_(libs.displayServer, resources, providers->providers[p])) {
                try {
                  for (int o = 0; o < provider->noutputs; ++o) {
                    if (provider->outputs[o] == handle) {
                      adapterName = std::string(provider->name, provider->nameLen); 
                      break; // found -> exit loop
                    }
                  }
                } 
                catch (...) { 
                  libs.randr.FreeProviderInfo_(provider); 
                  libs.randr.FreeProviderResources_(providers); 
                  libs.randr.FreeScreenResources_(resources); 
                  throw; 
                }
                libs.randr.FreeProviderInfo_(provider);
              }
            }
            libs.randr.FreeProviderResources_(providers);
          }
          libs.randr.FreeScreenResources_(resources);
        }
      }
      return adapterName;
    }
    
    // read work area of a monitor (usable position/size)
    static inline void _readScreenArea(XRRCrtcInfo* crtc, XRRScreenResources* resources, 
                                       DisplayArea& outScreenArea, DisplayArea& outWorkArea) {
      LibrariesX11& libs = LibrariesX11::instance(); // XRR params -> libs already instanced
      
      outScreenArea.x = outWorkArea.x = crtc->x;
      outScreenArea.y = outWorkArea.y = crtc->y;

      XRRModeInfo* modeInfo = nullptr;
      for (int m = 0; m < resources->nmode; ++m) {
        if (resources->modes[m].id == crtc->mode) {
          modeInfo = resources->modes + m;
          break;
        }
      }
      if (modeInfo != nullptr) { // store screen area at first
        outScreenArea.width = outWorkArea.width = modeInfo->width;
        outScreenArea.height = outWorkArea.height = modeInfo->height;
      }
      else {
        outScreenArea.width = outWorkArea.width = crtc->width;
        outScreenArea.height = outWorkArea.height = crtc->height;
      }

      if (libs.atoms.NET_WORKAREA && libs.atoms.NET_CURRENT_DESKTOP) { // if extensions available, adjust work area
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
    
    // ---
    
    // read all attributes of a monitor handle
    static bool read(RROutput handle, bool checkPrimary, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      bool isSuccess = false;
      
      LibrariesX11& libs = LibrariesX11::instance();
      if (handle && libs.randr.isAvailable) {
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          
          if (XRROutputInfo* outInfo = libs.randr.GetOutputInfo_(libs.displayServer, resources, handle)) {
            if (outInfo->connection == RR_Connected && outInfo->crtc != None) {
              if (XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, outInfo->crtc)) {
                try {
                  isSuccess = true;
                  outController = outInfo->crtc;
                  outAttr.description = outAttr.id = outInfo->name;
                  ::attributes::_readScreenArea(crtc, resources, outAttr.screenArea, outAttr.workArea);
                  if (checkPrimary)
                    outAttr.isPrimary = ::attributes::_isPrimaryDevice(libs, handle);
                } 
                catch (...) {
                  libs.randr.FreeCrtcInfo_(crtc);
                  libs.randr.FreeOutputInfo_(outInfo);
                  libs.randr.FreeScreenResources_(resources);
                  throw;
                }
                libs.randr.FreeCrtcInfo_(crtc);
              }
            }
            libs.randr.FreeOutputInfo_(outInfo);
          }
          libs.randr.FreeScreenResources_(resources);
        }
      }
      return isSuccess;
    }
    // find monitor handle for ID + read all attributes (if outAttr not null)
    static DisplayMonitor::Handle readForId(const DisplayMonitor::DeviceId& id, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      RROutput handle = (RROutput)0;
      
      LibrariesX11& libs = LibrariesX11::instance();
      if (!id.empty() && libs.randr.isAvailable) {
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          
          for (int o = 0; handle == (RROutput)0 && o < resources->noutput; ++o) {
            if (XRROutputInfo* outInfo = libs.randr.GetOutputInfo_(libs.displayServer, resources, resources->outputs[o])) {
              if (id == outInfo->name && outInfo->connection == RR_Connected && outInfo->crtc != None) {
                if (XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, outInfo->crtc)) {
                  try {
                    handle = resources->outputs[o];
                    outController = outInfo->crtc;
                    outAttr.description = outAttr.id = outInfo->name;
                    ::attributes::_readScreenArea(crtc, resources, outAttr.screenArea, outAttr.workArea);
                    outAttr.isPrimary = ::attributes::_isPrimaryDevice(libs, handle);
                  }
                  catch (...) {
                    libs.randr.FreeCrtcInfo_(crtc);
                    libs.randr.FreeOutputInfo_(outInfo);
                    libs.randr.FreeScreenResources_(resources);
                    throw;
                  }
                  libs.randr.FreeCrtcInfo_(crtc);
                }
              }
              libs.randr.FreeOutputInfo_(outInfo);
            }
          }
          libs.randr.FreeScreenResources_(resources);
        }
      }
      return (DisplayMonitor::Handle)handle;
    }
  }


// -- get display monitors (handle + attributes) -- ----------------------------

  namespace monitors {
    // read handle/attributes of primary/default monitor
    static DisplayMonitor::Handle getPrimary(DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      DisplayMonitor::Handle handle = (DisplayMonitor::Handle)0;
      
      LibrariesX11& libs = LibrariesX11::instance();
      if (libs.randr.isAvailable) {
        RROutput primaryOutput = libs.randr.GetOutputPrimary_(libs.displayServer, libs.rootWindow);
        handle = (DisplayMonitor::Handle)primaryOutput;
        attributes::read(primaryOutput, false, outController, outAttr);
      }
      outAttr.isPrimary = true;
      return (DisplayMonitor::Handle)handle;
    }
    
    // read handle/attributes of primary/default monitor
    static bool getByHandle(DisplayMonitor::Handle handle, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      LibrariesX11& libs = LibrariesX11::instance();
      if (libs.randr.isAvailable) {
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          for (int o = 0; o < resources->noutput; ++o) {
            if (resources->outputs[o] == (RROutput)handle) {
              libs.randr.FreeScreenResources_(resources);
              return attributes::read(resources->outputs[o], true, outController, outAttr);
            }
          }
          libs.randr.FreeScreenResources_(resources);
        }
      }
      return false; 
    }
    
    // ---
    
    // list handles of all active monitors
    static inline bool listHandles(std::vector<DisplayMonitor::Handle>& resultsOut) {
      LibrariesX11& libs = LibrariesX11::instance();
      if (libs.randr.isAvailable) {
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          
          for (int o = 0; o < resources->noutput; ++o) {
            if (XRROutputInfo* outInfo = libs.randr.GetOutputInfo_(libs.displayServer, resources, resources->outputs[o])) {
              if (outInfo->connection == RR_Connected && outInfo->crtc != None)
                resultsOut.emplace_back((DisplayMonitor::Handle)resources->outputs[o]);
              
              libs.randr.FreeOutputInfo_(outInfo);
            }
          }
          libs.randr.FreeScreenResources_(resources);
          return true;
        }
      }
      return false; 
    }
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (!this->_handle || !monitors::getByHandle(this->_handle, this->_controller, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = attributes::readForId(id, this->_controller, this->_attributes);
    if (!this->_handle) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    std::vector<DisplayMonitor::Handle> handles;
    if (monitors::listHandles(handles) && index < handles.size())
      this->_handle = handles[index];
    if (!this->_handle || !attributes::read(this->_handle, true, this->_controller, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitorList;

    std::vector<DisplayMonitor::Handle> handles;
    if (monitors::listHandles(handles)) {
      for (auto it : handles) {
        try {
          monitorList.emplace_back(it, false);
        }
        catch (const std::invalid_argument&) {} // ignore display if "not found"
        catch (...) { throw; }
      }
    }
    return monitorList;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return attributes::_readAdapterName((RROutput)this->_handle);
  }
  

// -- display modes -- ---------------------------------------------------------

  namespace monitors {
    // display mode attribute - refresh rate
    static inline uint32_t _readRefreshRate(XRRModeInfo& modeInfo) {
      if (modeInfo.hTotal && modeInfo.vTotal) {
        double rate = (double)modeInfo.dotClock / ((double)modeInfo.hTotal * (double)modeInfo.vTotal);
        return static_cast<uint32_t>(rate + 0.0000001); // show "59.94" as "59", to distinguish it from "60"
      }
      return undefinedRefreshRate();
    }
    // display mode attribute - bit depth
    static inline uint32_t _readBitDepth(LibrariesX11& libs) {
      int depth = DefaultDepth(libs.displayServer, libs.screenIndex);
      return (depth > 0) ? depth : 32;
    }
    
    // convert display mode values to mode ID
    static inline bool _findDisplayModeId(DisplayMode desiredMode, LibrariesX11& libs, XRROutputInfo* outInfo, XRRScreenResources* resources, RRMode& outMode) {
      std::set<RRMode> supportedModes;
      for (int m = 0; m < outInfo->nmode; ++m)
        supportedModes.emplace(outInfo->modes[m]);
      
      for (int m = 0; m < resources->nmode; ++m) {
        XRRModeInfo& modeInfo = resources->modes[m];
        
        if (supportedModes.find(modeInfo.id) != supportedModes.end() && (modeInfo.modeFlags & RR_Interlace) == 0 // supported
            && modeInfo.width == desiredMode.width && modeInfo.height == desiredMode.height // appropriate dimensions
            && ::monitors::_readRefreshRate(modeInfo) == desiredMode.refreshRate
            && ::monitors::_readBitDepth(libs) == desiredMode.bitDepth) {
          outMode = modeInfo.id;
          return true;
        }
      }
      return false;
    }
    
    // ---

    // read display resolution/depth/rate of a monitor
    static bool getDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller, DisplayMode& out) noexcept { 
      try {
        LibrariesX11& libs = LibrariesX11::instance();
        if (!libs.randr.isAvailable || controller == (DisplayMonitor::Handle)0)
          return false;

        bool isSuccess = false;
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          if (XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, (RRCrtc)controller)) {
            
            for (int m = 0; m < resources->nmode; ++m) {
              if (resources->modes[m].id == crtc->mode) {
                XRRModeInfo* modeInfo = resources->modes + m;
                out.width = modeInfo->width;
                out.height = modeInfo->height;
                out.refreshRate = ::monitors::_readRefreshRate(*modeInfo);
                out.bitDepth = ::monitors::_readBitDepth(libs);
                
                isSuccess = true;
                break;
              }
            }
            libs.randr.FreeCrtcInfo_(crtc);
          }
          libs.randr.FreeScreenResources_(resources);
        }
        return isSuccess;
      } 
      catch (...) { return false; }
    }

    // set display resolution/depth/rate of a monitor
    static inline bool setDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller, const DisplayMode& desiredMode) noexcept { 
      try {
        LibrariesX11& libs = LibrariesX11::instance();
        if (!libs.randr.isAvailable || controller == (DisplayMonitor::Handle)0)
          return false;

        bool isSuccess = false;
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          if (XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, (RRCrtc)controller)) {
            if (XRROutputInfo* outInfo = libs.randr.GetOutputInfo_(libs.displayServer, resources, (RROutput)handle)) {
              RRMode originalMode = crtc->mode;
              
              try {
                RRMode nativeTargetMode = 0;
                if (::monitors::_findDisplayModeId(desiredMode, libs, outInfo, resources, nativeTargetMode)
                 && libs.randr.SetCrtcConfig_(libs.displayServer, resources, (RRCrtc)controller, CurrentTime,
                                              crtc->x, crtc->y, nativeTargetMode, crtc->rotation, 
                                              crtc->outputs, crtc->noutput)) {
                
                  if (libs.originalModes.find((RROutput)handle) == libs.originalModes.end())
                    libs.originalModes.emplace((RROutput)handle, originalMode); // store original display mode (if it hasn't been changed before)
                  isSuccess = true;
                }
              } 
              catch (...) { // alloc failure -> restore original mode
                isSuccess = false;
                libs.randr.SetCrtcConfig_(libs.displayServer, resources, (RRCrtc)controller, CurrentTime, crtc->x, crtc->y, 
                                           originalMode, crtc->rotation, crtc->outputs, crtc->noutput);
              }
              libs.randr.FreeOutputInfo_(outInfo);
            }
            libs.randr.FreeCrtcInfo_(crtc);
          }
          libs.randr.FreeScreenResources_(resources);
        }
        return isSuccess;
      } 
      catch (...) { return false; }
    }
    
    // reset display resolution/depth/rate of a monitor to default values
    static inline bool setDefaultDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller) noexcept { 
      try {
        LibrariesX11& libs = LibrariesX11::instance();
        if (!libs.randr.isAvailable || controller == (DisplayMonitor::Handle)0)
          return false;
        if (libs.originalModes.find((RROutput)handle) == libs.originalModes.end()) // no previous display change -> already default mode
          return true; 

        bool isSuccess = false;
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          if (XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, (RRCrtc)controller)) {
            
            if (libs.randr.SetCrtcConfig_(libs.displayServer, resources, (RRCrtc)controller, CurrentTime,
                                          crtc->x, crtc->y, libs.originalModes[(RROutput)handle], 
                                          crtc->rotation, crtc->outputs, crtc->noutput)) {
              libs.originalModes.erase((RROutput)handle); // restored to initial mode -> remove backup
              isSuccess = true;
            }
            libs.randr.FreeCrtcInfo_(crtc);
          }
          libs.randr.FreeScreenResources_(resources);
        }
        return isSuccess;
      } 
      catch (...) { return false; }
    }
    
    // ---
  
    // list all display modes of a monitor
    static inline std::vector<DisplayMode> listDisplayModes(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller) {
      std::vector<DisplayMode> modes;
      
      LibrariesX11& libs = LibrariesX11::instance();
      if (controller && libs.randr.isAvailable) {
        if (XRRScreenResources* resources = libs.randr.GetScreenResourcesCurrent_(libs.displayServer, libs.rootWindow)) {
          if (XRRCrtcInfo* crtc = libs.randr.GetCrtcInfo_(libs.displayServer, resources, (RRCrtc)controller)) {
            if (XRROutputInfo* outInfo = libs.randr.GetOutputInfo_(libs.displayServer, resources, (RROutput)handle)) {
              try {
                std::set<RRMode> supportedModes; // list of all existing display modes
                for (int m = 0; m < outInfo->nmode; ++m)
                  supportedModes.emplace(outInfo->modes[m]);
              
                // get display modes available for current monitor
                for (int m = 0; m < resources->nmode; ++m) {
                  XRRModeInfo* modeInfo = resources->modes + m;
                  if (supportedModes.find(modeInfo->id) != supportedModes.end() && (modeInfo->modeFlags & RR_Interlace) == 0) {

                    DisplayMode data;
                    data.width = modeInfo->width;
                    data.height = modeInfo->height;
                    data.refreshRate = ::monitors::_readRefreshRate(*modeInfo);
                    data.bitDepth = ::monitors::_readBitDepth(libs);
                    modes.emplace_back(std::move(data));
                  }
                }
              } 
              catch (...) { 
                libs.randr.FreeOutputInfo_(outInfo); 
                libs.randr.FreeCrtcInfo_(crtc); 
                libs.randr.FreeScreenResources_(resources); 
                throw; 
              }
              libs.randr.FreeOutputInfo_(outInfo);
            }
            libs.randr.FreeCrtcInfo_(crtc);
          }
          libs.randr.FreeScreenResources_(resources);
        }
      }
      return modes;
    }
  }
  
  // ---
  
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!monitors::getDisplayMode(this->_handle, this->_controller, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) {
    if (monitors::setDisplayMode(this->_handle, this->_controller, mode)) {
      if (refreshAttributes && !attributes::read(this->_handle, true, this->_controller, this->_attributes)) {
        this->_attributes.screenArea.width = mode.width;
        this->_attributes.screenArea.height = mode.height;
      }
      return true;
    }
    return false;
  }
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) {
    if (monitors::setDefaultDisplayMode(this->_handle, this->_controller)) {
      if (refreshAttributes)
        attributes::read(this->_handle, true, this->_controller, this->_attributes);
      return true;
    }
    return false;
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    return monitors::listDisplayModes(this->_handle, this->_controller);
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    try {
      LibrariesX11& libs = LibrariesX11::instance();     
      if (isEnabled)
        libs.readSystemDpi();
      else
        libs.dpiX = libs.dpiY = __P_HARDWARE_X11_BASE_DPI;
      return true;
    } 
    catch (...) { return false; }
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle) const noexcept {
    try {
      LibrariesX11& libs = LibrariesX11::instance();
      outDpiX = static_cast<uint32_t>(libs.dpiX + 0.5f); // round value
      outDpiY = static_cast<uint32_t>(libs.dpiY + 0.5f);
    } 
    catch (...) { outDpiX = outDpiY = static_cast<uint32_t>(__P_HARDWARE_X11_BASE_DPI); }
  }
  
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle) const noexcept {
    try {
      LibrariesX11& libs = LibrariesX11::instance();
      outScaleX = libs.dpiX / __P_HARDWARE_X11_BASE_DPI;
      outScaleY = libs.dpiY / __P_HARDWARE_X11_BASE_DPI;
    } 
    catch (...) { outScaleX = outScaleY = 1.0f; }
  }


// -- metrics -- ---------------------------------------------------------------

  static inline int32_t __getBorderSize(DisplayMonitor::WindowHandle windowHandle) {
    LibrariesX11& libs = LibrariesX11::instance();
    XWindowAttributes attributes;
    libs.xlib.GetWindowAttributes_(libs.displayServer, (windowHandle != (DisplayMonitor::WindowHandle)0) ? windowHandle : libs.rootWindow, &attributes);
    return attributes.border_width;
  }

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, 
                                                            bool hasMenu, uint32_t hasBorders, uint32_t hasCaption) const noexcept {
    try {
      LibrariesX11& libs = LibrariesX11::instance();

      // read window size with extents (if supported)
      if (libs.atoms.NET_FRAME_EXTENTS && windowHandle != (DisplayMonitor::WindowHandle)0) {
        // wait until extents are set up by window manager
        int32_t* extents = nullptr;
        if (libs.getWindowProperty(windowHandle, libs.atoms.NET_FRAME_EXTENTS, AnyPropertyType, 
                                   (unsigned char**)&extents, 4, 1024) > 0) {
          DisplayArea windowArea { extents[0], extents[2], static_cast<uint32_t>(extents[0]+extents[1]), static_cast<uint32_t>(extents[2]+extents[3]) };
          libs.xlib.Free_(extents);
          return windowArea;
        }
      }
      
      // fallback if not supported
      int32_t borderSize = (hasBorders) ? __getBorderSize(windowHandle) : 0;
      int32_t captionSize = (hasCaption && windowHandle != (DisplayMonitor::WindowHandle)0) ? 24 : 0;
      if (hasMenu)
        captionSize += 24;
      
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
    catch (...) { return clientArea; }
  }
#endif
