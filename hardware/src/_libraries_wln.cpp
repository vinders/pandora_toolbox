/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <stdexcept>
# include <dlfcn.h>
# include "hardware/_private/_libraries_wln.h"

  pandora::hardware::LibrariesWayland pandora::hardware::LibrariesWayland::_libs{};

  using namespace pandora::hardware;


  // -- library binding utilities --
  
  static inline LibHandle _loadLibrary(const char fileName[]) noexcept {
    return dlopen(fileName, RTLD_LAZY | RTLD_LOCAL);
  }
  static inline void _freeLibrary(LibHandle lib) noexcept {
    dlclose(lib);
  }
  template <typename _Signature>
  static inline _Signature _getSymbolAddress(LibHandle lib, const char name[]) noexcept {
    return (_Signature)dlsym(lib, name);
  }
  
  
  // -- utilities --
  
  void LibrariesWayland::getScreenPixelSize(WaylandOutput& data, uint32_t& outWidth, uint32_t& outHeight) noexcept {
    if (data.modes != nullptr && data.modesLength > 0) {
      outWidth = data.modes[data.currentMode].width;
      outHeight = data.modes[data.currentMode].height;
    }
    else {
      uint32_t dpi = getMonitorDpi(data);
      double width = (double)(data.physWidth * dpi)/25.4 + 0.500001; // mm to pixels + round (0.5) + fix floating point error
      double height = (double)(data.physHeight * dpi)/25.4 + 0.500001;
      outWidth = static_cast<uint32_t>(width);
      outHeight = static_cast<uint32_t>(height);
    }
  }
  
  uint32_t LibrariesWayland::getMonitorDpi(WaylandOutput& data) noexcept {
    return __P_HARDWARE_WAYLAND_BASE_DPI * data.scaleFactor;
  }


  // -- window manager event handlers --
  
  void LibrariesWayland::_windowManagerPingHandler(void*, struct xdg_wm_base* windowManagerBase, uint32_t serial) {
    xdg_wm_base_pong(windowManagerBase, serial);
  }
  static const struct xdg_wm_base_listener _windowManagerBaseListener = {
    LibrariesWayland::_windowManagerPingHandler
  };
  
  
  // -- output event handlers --
  
  // read position/size/description of monitor
  void LibrariesWayland::_outputGeometryHandler(void* data, struct wl_output*,
                                                int32_t x, int32_t y, int32_t physWidth, int32_t physHeight,
                                                int32_t, const char* brand, const char* model, int32_t) {
    std::lock_guard<std::recursive_mutex> guard(_libs.wl.outputsLock);
    if (!_libs._isOutputAvailable((WaylandOutput*)data))
      return;
    WaylandOutput* outputData = (WaylandOutput*)data;
    
    outputData->x = x;
    outputData->y = y;
    outputData->physWidth = physWidth;
    outputData->physHeight = physHeight;
    outputData->preferredMode = -1;
    
    size_t brandLength = strnlen(brand, 256);
    size_t modelLength = strnlen(model, 512);
    outputData->description = (char*)calloc(brandLength + modelLength + 2, sizeof(char));
    if (outputData->description) {
      char* it = outputData->description;
      if (brandLength > 0) {
        memcpy((void*)it, (void*)brand, brandLength);
        it += brandLength;
        if (modelLength > 0) {
          *it = ' ';
          ++it;
        }
      }
      if (modelLength > 0)
        memcpy((void*)it, (void*)model, modelLength);
    }
    else
      fprintf(stderr, "LibrariesWayland: monitor description alloc failure...");
  }

  // read output display mode
  void LibrariesWayland::_outputModeHandler(void* data, struct wl_output*, uint32_t flags,
                                            int32_t width, int32_t height, int32_t refreshRate) {
    std::lock_guard<std::recursive_mutex> guard(_libs.wl.outputsLock);
    if (!_libs._isOutputAvailable((WaylandOutput*)data))
      return;
    WaylandOutput* outputData = (WaylandOutput*)data;
    
    if (outputData->modes == nullptr) {
      outputData->modes = (WaylandDisplayMode*)malloc(sizeof(WaylandDisplayMode));
      if (outputData->modes == nullptr) {
        fprintf(stderr, "LibrariesWayland: display modes alloc failure...");
        return;
      }
      outputData->modesLength = size_t{ 1u };
    }
    else {
      for (size_t i = 0; i < outputData->modesLength; ++i) { // if mode already in list, do not add an entry
        auto& it = outputData->modes[i];
        if (it.width == (uint32_t)width && it.height == (uint32_t)height && it.refreshRate == (uint32_t)refreshRate) {
          if (flags & WL_OUTPUT_MODE_CURRENT)
            outputData->currentMode = static_cast<uint32_t>(i);
          if (flags & WL_OUTPUT_MODE_PREFERRED)
            outputData->preferredMode = static_cast<int32_t>(i);
          return; // exit here
        }
      }
      
      WaylandDisplayMode* resized = (WaylandDisplayMode*)realloc(outputData->modes, sizeof(WaylandDisplayMode));
      if (resized == nullptr) {
        fprintf(stderr, "LibrariesWayland: display modes realloc failure...");
        return; // on failure, keep previous modes
      }
      outputData->modes = resized;
      ++(outputData->modesLength);
    }
    
    WaylandDisplayMode& newMode = outputData->modes[outputData->modesLength - 1u];
    newMode.width = (width >= 0) ? static_cast<uint32_t>(width) : static_cast<uint32_t>(-width);
    newMode.height = (height >= 0) ? static_cast<uint32_t>(height) : static_cast<uint32_t>(-height);
    newMode.refreshRate = (refreshRate > 0) ? static_cast<uint32_t>(refreshRate) : 0;
    
    if (flags & WL_OUTPUT_MODE_CURRENT)
      outputData->currentMode = static_cast<uint32_t>(outputData->modesLength) - 1u;
    if (flags & WL_OUTPUT_MODE_PREFERRED)
      outputData->preferredMode = static_cast<int32_t>(outputData->modesLength) - 1;
  }

  // read output scaling factor
  void LibrariesWayland::_outputScaleHandler(void* data, struct wl_output* output, int32_t factor) {
    std::lock_guard<std::recursive_mutex> guard(_libs.wl.outputsLock);
    if (!_libs._isOutputAvailable((WaylandOutput*)data))
      return;
    ((WaylandOutput*)data)->scaleFactor = factor;
  }
  
  // end of operations -> verify consistency
  void LibrariesWayland::_outputDoneHandler(void* data, struct wl_output* output) {
    std::lock_guard<std::recursive_mutex> guard(_libs.wl.outputsLock);
    if (!_libs._isOutputAvailable((WaylandOutput*)data))
      return;
    WaylandOutput* outputData = (WaylandOutput*)data;
    
    if (outputData->preferredMode < 0)
      outputData->preferredMode = outputData->currentMode;
    
    if (outputData->scaleFactor <= 0)
      outputData->scaleFactor = 1;
    
    if (outputData->physWidth <= 0 || outputData->physHeight <= 0) {
      if (outputData->modes != nullptr) {
        auto& currentMode = outputData->modes[outputData->currentMode];
        double dpi = (double)(__P_HARDWARE_WAYLAND_BASE_DPI * outputData->scaleFactor);
        outputData->physWidth = static_cast<uint32_t>((double)currentMode.width * (25.4/dpi)); // pixels to mm
        outputData->physHeight = static_cast<uint32_t>((double)currentMode.height * (25.4/dpi));
      }
      else
        outputData->physWidth = outputData->physHeight = 0;
    }
  }

  // global output listener
  static const struct wl_output_listener _outputListener = {
    LibrariesWayland::_outputGeometryHandler,
    LibrariesWayland::_outputModeHandler,
    LibrariesWayland::_outputDoneHandler,
    LibrariesWayland::_outputScaleHandler
  };
  
  
  // -- output collection management --
  
  // verify output existence
  bool LibrariesWayland::_isOutputAvailable(WaylandOutput* data) noexcept {
    // no lock (callers already own lock)
    WaylandOutput* cur = _libs.wl.outputs;
    while (cur != nullptr && cur != (WaylandOutput*)data)
      cur = cur->next;
    return (cur != nullptr);
  }
  
  // insert output entry
  void LibrariesWayland::_insertOutput(uint32_t id, struct wl_registry* registry) noexcept {
    WaylandOutput* newEntry = (WaylandOutput*)calloc(1, sizeof(WaylandOutput));
    if (newEntry == nullptr) {
      fprintf(stderr, "LibrariesWayland: _addOutput alloc failure...");
      return;
    }
    wl_output* output = (wl_output*)wl_registry_bind(registry, id, &wl_output_interface, 2);
    if (output == nullptr) {
      fprintf(stderr, "LibrariesWayland: wayland output binding failure...");
      free(newEntry);
      return;
    }

    newEntry->id = id;
    newEntry->output = output;
    newEntry->scaleFactor = 1;
    wl_output_add_listener(output, &_outputListener, newEntry);
    
    // find position in linked list
    std::lock_guard<std::recursive_mutex> guard(wl.outputsLock);
    WaylandOutput* parent = nullptr;
    WaylandOutput* child = wl.outputs;
    while (child != nullptr && child->id < id) {
      parent = child;
      child = child->next;
    }
    // insert after previous item
    if (parent == nullptr)
      wl.outputs = newEntry;
    else
      parent->next = newEntry;
    // next item
    if (child != nullptr) {
      if (child->id == id) { // same ID -> remove
        newEntry->next = child->next;
        free(child);
      }
      else
        newEntry->next = child;
    }
  }
  
  // remove output entry
  void LibrariesWayland::_eraseOutput(uint32_t id) noexcept {
    wl_output* output = nullptr;
    { // lock scope
      std::lock_guard<std::recursive_mutex> guard(wl.outputsLock);
      WaylandOutput* parent = nullptr;
      for (WaylandOutput* cur = wl.outputs; cur != nullptr; cur = cur->next) {
        if (cur->id == id) {
          output = cur->output;
          if (cur->description)
            free(cur->description);
          if (cur->modes)
            free(cur->modes);
        
          if (parent != nullptr)
            parent->next = cur->next;
          else
            wl.outputs = cur->next;
          free(cur);
          break;
        }
        parent = cur;
      }
    }
    if (output)
      wl_output_destroy(output);
  }
  
  // -- registry handlers --
  
  // resource aquisition
  void LibrariesWayland::_globalRegistryHandler(void*, struct wl_registry* registry, uint32_t id,
                                                const char* interface, uint32_t version) {
    if (strcmp(interface, "wl_output") == 0) {
      if (version >= 2) 
        _libs._insertOutput(id, registry);
      else
        fprintf(stderr, "LibrariesWayland: outdated wayland output interface: not supported");
    }
    else if (strcmp(interface, "wl_compositor") == 0) {
      _libs.wl.compositorVersion = (version != 0xFFFFFFFFu) ? version : 3;
      _libs.wl.compositor = (wl_compositor*)wl_registry_bind(registry, id, &wl_compositor_interface, _libs.wl.compositorVersion);
    }
    else if (strcmp(interface, "wl_subcompositor") == 0) {
      _libs.wl.subCompositor = (wl_subcompositor*)wl_registry_bind(registry, id, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0) {
      _libs.wl.sharedMemory = (wl_shm*)wl_registry_bind(registry, id, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, "wp_viewporter") == 0) {
      _libs.wp.viewporter = (wp_viewporter*)wl_registry_bind(registry, id, &wp_viewporter_interface, 1);
    }
    else if (strcmp(interface, "xdg_wm_base") == 0) {
      _libs.xdg.windowManagerBase = (xdg_wm_base*)wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
      xdg_wm_base_add_listener(_libs.xdg.windowManagerBase, &_windowManagerBaseListener, nullptr);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0) {
      _libs.xdg.decorationManager = (zxdg_decoration_manager_v1*)wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0) {
      _libs.wp.relativePointerManager = (zwp_relative_pointer_manager_v1*)wl_registry_bind(registry, id, &zwp_relative_pointer_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0) {
      _libs.wp.pointerConstraints = (zwp_pointer_constraints_v1*)wl_registry_bind(registry, id, &zwp_pointer_constraints_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0) {
      _libs.wp.idleInhibitManager = (zwp_idle_inhibit_manager_v1*)wl_registry_bind(registry, id, &zwp_idle_inhibit_manager_v1_interface, 1);
    }
  }
  
  // resource removal
  void LibrariesWayland::_globalRegistryRemoveHandler(void*, struct wl_registry* registry, uint32_t id) {
    _libs._eraseOutput(id);
  }
  
  // global listener
  static const struct wl_registry_listener _registryListener = {
    LibrariesWayland::_globalRegistryHandler,
    LibrariesWayland::_globalRegistryRemoveHandler
  };
  
  
  // -- init --

  void LibrariesWayland::init() {
    if (this->_isInit)
      return;
    
    wl.display = wl_display_connect(nullptr);
    if (!wl.display)
      throw std::runtime_error("LibrariesWayland: connection failure to display");

    wl.registry = wl_display_get_registry(wl.display);
    wl_registry_add_listener(wl.registry, &_registryListener, nullptr);
    
    // synchronize registry objects + initial outputs
    wl_display_roundtrip(wl.display);
    wl_display_roundtrip(wl.display);

    if (!xdg.windowManagerBase)
      throw std::runtime_error("LibrariesWayland: xdg-shell not found in compositor");

    this->_isInit = true;
  }

  // -- shutdown --

  void LibrariesWayland::shutdown() noexcept {
    { // lock scope
      std::lock_guard<std::recursive_mutex> guard(wl.outputsLock);
      WaylandOutput* output = wl.outputs;
      while (output) {
        WaylandOutput* cur = output;
        if (cur->output)
          wl_output_destroy(cur->output);
        if (cur->description)
          free(cur->description);
        if (cur->modes)
          free(cur->modes);
        output = output->next;
        free(cur);
      }
      wl.outputs = nullptr;
    }
    
    if (wl.subCompositor)
      wl_subcompositor_destroy(wl.subCompositor);
    if (wl.compositor)
      wl_compositor_destroy(wl.compositor);
    if (wl.sharedMemory)
      wl_shm_destroy(wl.sharedMemory);
    if (wp.viewporter)
      wp_viewporter_destroy(wp.viewporter);
      
    if (xdg.decorationManager)
      zxdg_decoration_manager_v1_destroy(xdg.decorationManager);
    if (xdg.windowManagerBase)
      xdg_wm_base_destroy(xdg.windowManagerBase);
    if (wp.relativePointerManager)
      zwp_relative_pointer_manager_v1_destroy(wp.relativePointerManager);
    if (wp.pointerConstraints)
      zwp_pointer_constraints_v1_destroy(wp.pointerConstraints);
    if (wp.idleInhibitManager)
      zwp_idle_inhibit_manager_v1_destroy(wp.idleInhibitManager);
    
    if (wl.registry)
      wl_registry_destroy(wl.registry);
    if (wl.display) {
      wl_display_flush(wl.display);
      wl_display_disconnect(wl.display);
    }
    
    this->_isInit = false;
  }
#endif
