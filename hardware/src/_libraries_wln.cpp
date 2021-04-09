/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <climits>
# include <ctime>
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


  // -- init --

  bool LibrariesWayland::init() noexcept {
    if (this->_isInit)
      return true;
 
    //...

    this->_isInit = true;
    return true;
  }

  // -- shutdown --

  void LibrariesWayland::shutdown() noexcept {
    //...
    
    this->_isInit = false;
  }
#endif
