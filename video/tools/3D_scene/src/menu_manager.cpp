/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include "menu_manager.h"
#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# define _SYSTEM_STR(str) L"" str
# define _P_MENU_LABEL(wstr,str) wstr
#else
# define _SYSTEM_STR(str) str
# define _P_MENU_LABEL(wstr,str) str
#endif

#define __MENU_API_VSYNC         190
#define __MENU_API_SPLIT         191

#define __MENU_API_NONE       200
#define __MENU_API_OPENGL4    201
#define __MENU_API_OPENGLES3  202
#define __MENU_API_D3D11      203

#define __MENU_MOUSE_SENSIV_LOW   300
#define __MENU_MOUSE_SENSIV_AVG   301
#define __MENU_MOUSE_SENSIV_HIGH  302
#define __MENU_MOUSE_SENSIV_MIN __MENU_MOUSE_SENSIV_LOW
#define __MENU_MOUSE_SENSIV_MAX __MENU_MOUSE_SENSIV_HIGH

#define __MENU_SCN_AA_OFF   400
#define __MENU_SCN_AA_MIN __MENU_SCN_AA_OFF
#define __MENU_SCN_AA_MAX __MENU_SCN_AA_OFF

#define __MENU_SCN_FX_OFF   500
#define __MENU_SCN_FX_MIN __MENU_SCN_FX_OFF
#define __MENU_SCN_FX_MAX __MENU_SCN_FX_OFF

#define __MENU_SCN_FLT_NEAREST   600
#define __MENU_SCN_FLT_BILINEAR  601
#define __MENU_SCN_FLT_GAUSS     602
#define __MENU_SCN_FLT_BESSEL    603
#define __MENU_SCN_FLT_LANCZOS   604
#define __MENU_SCN_FLT_SPLINE16  605
#define __MENU_SCN_FLT_MIN __MENU_SCN_FLT_NEAREST
#define __MENU_SCN_FLT_MAX __MENU_SCN_FLT_SPLINE16

#define __MENU_TEX_FLT_NEAREST  700
#define __MENU_TEX_FLT_BILINEAR 701
#define __MENU_TEX_FLT_GAUSS    702
#define __MENU_TEX_FLT_BESSEL   703
#define __MENU_TEX_FLT_LANCZOS  704
#define __MENU_TEX_FLT_SPLINE16 705
#define __MENU_TEX_FLT_MIN __MENU_TEX_FLT_NEAREST
#define __MENU_TEX_FLT_MAX __MENU_TEX_FLT_SPLINE16

#define __MENU_SPR_FLT_NEAREST  800
#define __MENU_SPR_FLT_BILINEAR 801
#define __MENU_SPR_FLT_GAUSS    802
#define __MENU_SPR_FLT_BESSEL   803
#define __MENU_SPR_FLT_LANCZOS  804
#define __MENU_SPR_FLT_SPLINE16 805
#define __MENU_SPR_FLT_MIN __MENU_SPR_FLT_NEAREST
#define __MENU_SPR_FLT_MAX __MENU_SPR_FLT_SPLINE16

#define __MENU_RENDER_NONE       900
#define __MENU_RENDER_WIREFRAME  901
#define __MENU_RENDER_AO         902
#define __MENU_RENDER_MIN __MENU_RENDER_NONE
#define __MENU_RENDER_MAX __MENU_RENDER_AO

#define __MENU_TEX_UP_NONE      1000
#define __MENU_TEX_UP_MIN __MENU_TEX_UP_NONE
#define __MENU_TEX_UP_MAX __MENU_TEX_UP_NONE

#define __MENU_TEX_MAP_NONE     1100
#define __MENU_TEX_MAP_MIN __MENU_TEX_MAP_NONE
#define __MENU_TEX_MAP_MAX __MENU_TEX_MAP_NONE

#define __MENU_SPR_UP_NONE      1200
#define __MENU_SPR_UP_MIN __MENU_SPR_UP_NONE
#define __MENU_SPR_UP_MAX __MENU_SPR_UP_NONE


#if defined(_WINDOWS)
  static inline void* __createMenu(bool isSubMenu) noexcept {
    return isSubMenu ? (void*)CreatePopupMenu() : (void*)CreateMenu();
  }
  static inline void __addMenuItem(void* menuHandle, uint32_t id, const wchar_t* label, bool isActive) noexcept {
    AppendMenuW((HMENU)menuHandle, isActive ? (MF_STRING|MF_CHECKED) : (MF_STRING|MF_UNCHECKED), (UINT)id, label);
  }
  static inline void __addSubMenu(void* menuHandle, void* subMenuHandle, const wchar_t* label) noexcept {
    AppendMenuW((HMENU)menuHandle, MF_POPUP, (UINT_PTR)subMenuHandle, label);
  }
  static inline void __addSeparator(void* menuHandle, bool isSubMenu) noexcept {
    AppendMenuW((HMENU)menuHandle, isSubMenu ? MF_SEPARATOR : MF_MENUBREAK, 0, nullptr);
  }
  static inline void __selectMenuItem(void* menu, uint32_t id, bool isActive) noexcept {
    CheckMenuItem((HMENU)menu, (UINT)id, (isActive) ? MF_CHECKED : MF_UNCHECKED);
  }
  static void __selectMenuItem(void* menu, uint32_t id, uint32_t minVal, uint32_t maxVal) noexcept {
    for (UINT i = (UINT)minVal; i <= (UINT)maxVal; ++i) {
      CheckMenuItem((HMENU)menu, i, (i == (UINT)id) ? MF_CHECKED : MF_UNCHECKED);
    }
  }
#else
  static inline void* __createMenu(bool) noexcept { return nullptr; }
  static inline void __addMenuItem(void*, uint32_t, const char*, bool) noexcept {}
  static inline void __addSubMenu(void*, void*, const char*) noexcept {}
  static inline void __addSeparator(void*, bool) noexcept {}
  static void __selectMenuItem(void*, uint32_t, bool) noexcept {}
  static void __selectMenuItem(void*, uint32_t, uint32_t, uint32_t) noexcept {}
#endif

// ---

// create main menu-bar
scene::MenuManager::MenuManager(scene::Options& outSettings) : _settings(outSettings) {
  auto viewerPopup = __createMenu(true);
  auto screenFilterPopup = __createMenu(true);
  auto textureFilterPopup = __createMenu(true);
  auto spriteFilterPopup = __createMenu(true);

# if defined(_VIDEO_D3D11_SUPPORT)
    outSettings.api = scene::RenderingApi::d3d11;
    __addMenuItem(viewerPopup, __MENU_API_D3D11, _SYSTEM_STR("Direct3D 11"), true);
#   if defined(_VIDEO_OPENGL4_SUPPORT)
      __addMenuItem(viewerPopup, __MENU_API_OPENGL4, _SYSTEM_STR("OpenGL 4"), false);
#   endif
#   if defined(_VIDEO_OPENGLES3_SUPPORT)
      __addMenuItem(viewerPopup, __MENU_API_OPENGL4, _SYSTEM_STR("OpenGLES 3"), false);
#   endif

# elif defined(_VIDEO_OPENGL4_SUPPORT)
    outSettings.api = scene::RenderingApi::openGL4;
    __addMenuItem(viewerPopup, __MENU_API_OPENGL4, _SYSTEM_STR("OpenGL 4"), true);
#   if defined(_VIDEO_OPENGLES3_SUPPORT)
      __addMenuItem(viewerPopup, __MENU_API_OPENGL4, _SYSTEM_STR("OpenGLES 3"), false);
#   endif

# elif defined(_VIDEO_OPENGLES3_SUPPORT)
    outSettings.api = scene::RenderingApi::openGLES3;
    __addMenuItem(viewerPopup, __MENU_API_OPENGLES3, _SYSTEM_STR("OpenGLES 3"), true);
# else
    outSettings.api = scene::RenderingApi::none;
    __addMenuItem(viewerPopup, __MENU_API_NONE, _SYSTEM_STR("No renderer"), true);
# endif

  outSettings.mouseSensitivity = __MENU_MOUSE_SENSIV_AVG - __MENU_MOUSE_SENSIV_MIN;
  __addSeparator(viewerPopup, true);
  __addMenuItem(viewerPopup, __MENU_MOUSE_SENSIV_LOW, _SYSTEM_STR("Low sensitivity"), false);
  __addMenuItem(viewerPopup, __MENU_MOUSE_SENSIV_AVG, _SYSTEM_STR("Average sensitivity"), true);
  __addMenuItem(viewerPopup, __MENU_MOUSE_SENSIV_HIGH, _SYSTEM_STR("High sensitivity"), false);

  outSettings.renderMode = RenderingMode::normal;
  __addSeparator(viewerPopup, true);
  __addMenuItem(viewerPopup, __MENU_RENDER_NONE, _SYSTEM_STR("Standard"), true);
  __addMenuItem(viewerPopup, __MENU_RENDER_WIREFRAME, _SYSTEM_STR("Wireframe"), false);
  __addMenuItem(viewerPopup, __MENU_RENDER_AO, _SYSTEM_STR("Ambient occlusion"), false);
  outSettings.splitScreen = false;
  __addSeparator(viewerPopup, true);
  __addMenuItem(viewerPopup, __MENU_API_SPLIT, _SYSTEM_STR("Split screen"), false);
  outSettings.useVsync = false;
  __addSeparator(viewerPopup, true);
  __addMenuItem(viewerPopup, __MENU_API_VSYNC, _SYSTEM_STR("Vsync"), false);

  outSettings.scnFilter = scene::Interpolation::bilinear;
  __addMenuItem(screenFilterPopup, __MENU_SCN_FLT_NEAREST,  _SYSTEM_STR("Nearest"), false);
  __addMenuItem(screenFilterPopup, __MENU_SCN_FLT_BILINEAR, _SYSTEM_STR("Bilinear"), true);
  __addMenuItem(screenFilterPopup, __MENU_SCN_FLT_GAUSS,    _SYSTEM_STR("Gauss"), false);
  __addMenuItem(screenFilterPopup, __MENU_SCN_FLT_BESSEL,   _SYSTEM_STR("Bessel"), false);
  __addMenuItem(screenFilterPopup, __MENU_SCN_FLT_LANCZOS,  _SYSTEM_STR("Lanczos"), false);
  __addMenuItem(screenFilterPopup, __MENU_SCN_FLT_SPLINE16, _SYSTEM_STR("Spline16"), false);

  outSettings.aa = scene::AntiAliasing::none;
  __addSeparator(screenFilterPopup, true);
  __addMenuItem(screenFilterPopup, __MENU_SCN_AA_OFF, _SYSTEM_STR("No AA"), true);

  outSettings.fx = scene::VisualEffect::none;
  __addSeparator(screenFilterPopup, true);
  __addMenuItem(screenFilterPopup, __MENU_SCN_FX_OFF, _SYSTEM_STR("No effect"), true);

  outSettings.texFilter = scene::Interpolation::bilinear;
  __addMenuItem(textureFilterPopup, __MENU_TEX_FLT_NEAREST,  _SYSTEM_STR("Nearest"), false);
  __addMenuItem(textureFilterPopup, __MENU_TEX_FLT_BILINEAR, _SYSTEM_STR("Bilinear"), true);
  __addMenuItem(textureFilterPopup, __MENU_TEX_FLT_GAUSS,    _SYSTEM_STR("Gauss"), false);
  __addMenuItem(textureFilterPopup, __MENU_TEX_FLT_BESSEL,   _SYSTEM_STR("Bessel"), false);
  __addMenuItem(textureFilterPopup, __MENU_TEX_FLT_LANCZOS,  _SYSTEM_STR("Lanczos"), false);
  __addMenuItem(textureFilterPopup, __MENU_TEX_FLT_SPLINE16, _SYSTEM_STR("Spline16"), false);

  outSettings.texUpscale = Upscaling::none;
  __addSeparator(textureFilterPopup, true);
  __addMenuItem(textureFilterPopup, __MENU_TEX_UP_NONE, _SYSTEM_STR("1x"), true);

  outSettings.texMapping = Mapping::none;
  __addSeparator(textureFilterPopup, true);
  __addMenuItem(textureFilterPopup, __MENU_TEX_MAP_NONE, _SYSTEM_STR("No mapping"), true);


  outSettings.sprFilter = scene::Interpolation::bilinear;
  __addMenuItem(spriteFilterPopup, __MENU_SPR_FLT_NEAREST,  _SYSTEM_STR("Nearest"), false);
  __addMenuItem(spriteFilterPopup, __MENU_SPR_FLT_BILINEAR, _SYSTEM_STR("Bilinear"), true);
  __addMenuItem(spriteFilterPopup, __MENU_SPR_FLT_GAUSS,    _SYSTEM_STR("Gauss"), false);
  __addMenuItem(spriteFilterPopup, __MENU_SPR_FLT_BESSEL,   _SYSTEM_STR("Bessel"), false);
  __addMenuItem(spriteFilterPopup, __MENU_SPR_FLT_LANCZOS,  _SYSTEM_STR("Lanczos"), false);
  __addMenuItem(spriteFilterPopup, __MENU_SPR_FLT_SPLINE16, _SYSTEM_STR("Spline16"), false);

  outSettings.sprUpscale = Upscaling::none;
  __addSeparator(spriteFilterPopup, true);
  __addMenuItem(spriteFilterPopup, __MENU_SPR_UP_NONE, _SYSTEM_STR("1x"), true);

  auto menuBar = __createMenu(false);
  __addSubMenu(menuBar, viewerPopup, _P_MENU_LABEL(L"&Viewer", "Viewer"));
  __addSeparator(menuBar, false);
  __addSubMenu(menuBar, screenFilterPopup, _P_MENU_LABEL(L"&Screen filter", "Screen filter"));
  __addSubMenu(menuBar, textureFilterPopup, _P_MENU_LABEL(L"&Texture filter", "Texture filter"));
  __addSubMenu(menuBar, spriteFilterPopup, _P_MENU_LABEL(L"S&prite filter", "Sprite filter"));
  this->_resource = pandora::video::WindowResource::buildMenu((pandora::video::MenuHandle)menuBar);
}

// menu event management
void scene::MenuManager::onMenuCommand(int32_t id) {
  switch (id/100) {
    case __MENU_API_NONE/100: {
      if (this->_settings.api != (scene::RenderingApi)(id - __MENU_API_NONE)) {
        __selectMenuItem((void*)this->_resource->handle(), __MENU_API_NONE + (uint32_t)this->_settings.api, false);
        this->_settings.api = (scene::RenderingApi)(id - __MENU_API_NONE);
        __selectMenuItem((void*)this->_resource->handle(), __MENU_API_NONE + (uint32_t)this->_settings.api, true);
        apiChangeHandler(true);
      }
      break;
    }
    case __MENU_API_VSYNC/100: {
      if (id == __MENU_API_SPLIT) {
        this->_settings.splitScreen ^= true;
        __selectMenuItem((void*)this->_resource->handle(), __MENU_API_SPLIT, this->_settings.splitScreen);
        apiChangeHandler(false);
      }
      else {
        this->_settings.useVsync ^= true;
        __selectMenuItem((void*)this->_resource->handle(), __MENU_API_VSYNC, this->_settings.useVsync);
        vsyncChangeHandler(this->_settings.useVsync);
      }
      break;
    }
    case __MENU_MOUSE_SENSIV_MIN/100:
      if (this->_settings.mouseSensitivity != (id - __MENU_MOUSE_SENSIV_MIN)) {
        this->_settings.mouseSensitivity = (id - __MENU_MOUSE_SENSIV_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_MOUSE_SENSIV_MIN, __MENU_MOUSE_SENSIV_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_RENDER_NONE/100:
      if (this->_settings.renderMode != (scene::RenderingMode)(id - __MENU_RENDER_MIN)) {
        this->_settings.renderMode = (scene::RenderingMode)(id - __MENU_RENDER_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_RENDER_MIN, __MENU_RENDER_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_AA_MIN/100:
      if (this->_settings.aa != (scene::AntiAliasing)(id - __MENU_SCN_AA_MIN)) {
        this->_settings.aa = (scene::AntiAliasing)(id - __MENU_SCN_AA_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_SCN_AA_MIN, __MENU_SCN_AA_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_FX_MIN/100:
      if (this->_settings.fx != (scene::VisualEffect)(id - __MENU_SCN_FX_MIN)) {
        this->_settings.fx = (scene::VisualEffect)(id - __MENU_SCN_FX_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_SCN_FX_MIN, __MENU_SCN_FX_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_FLT_MIN/100:
      if (this->_settings.scnFilter != (scene::Interpolation)(id - __MENU_SCN_FLT_MIN)) {
        this->_settings.scnFilter = (scene::Interpolation)(id - __MENU_SCN_FLT_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_SCN_FLT_MIN, __MENU_SCN_FLT_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_TEX_FLT_MIN/100:
      if (this->_settings.texFilter != (scene::Interpolation)(id - __MENU_TEX_FLT_MIN)) {
        this->_settings.texFilter = (scene::Interpolation)(id - __MENU_TEX_FLT_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_TEX_FLT_MIN, __MENU_TEX_FLT_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_TEX_UP_MIN/100:
      if (this->_settings.texUpscale != (scene::Upscaling)(id - __MENU_TEX_UP_MIN)) {
        this->_settings.texUpscale = (scene::Upscaling)(id - __MENU_TEX_UP_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_TEX_UP_MIN, __MENU_TEX_UP_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_TEX_MAP_MIN/100:
      if (this->_settings.texMapping != (scene::Mapping)(id - __MENU_TEX_MAP_MIN)) {
        this->_settings.texMapping = (scene::Mapping)(id - __MENU_TEX_MAP_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_TEX_MAP_MIN, __MENU_TEX_MAP_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_SPR_FLT_MIN/100:
      if (this->_settings.sprFilter != (scene::Interpolation)(id - __MENU_SPR_FLT_MIN)) {
        this->_settings.sprFilter = (scene::Interpolation)(id - __MENU_SPR_FLT_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_SPR_FLT_MIN, __MENU_SPR_FLT_MAX);
        filterChangeHandler();
      }
      break;
    case __MENU_SPR_UP_MIN/100:
      if (this->_settings.sprUpscale != (scene::Upscaling)(id - __MENU_SPR_UP_MIN)) {
        this->_settings.sprUpscale = (scene::Upscaling)(id - __MENU_SPR_UP_MIN);
        __selectMenuItem((void*)this->_resource->handle(), (uint32_t)id, __MENU_SPR_UP_MIN, __MENU_SPR_UP_MAX);
        filterChangeHandler();
      }
      break;
    default: break;
  }
}
