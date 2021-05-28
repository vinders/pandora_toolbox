/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include "menu_manager.h"
#if defined(_WINDOWS)
# include <system/api/windows_api.h>
#endif

#define __MENU_API_OPENGL4    200
#define __MENU_API_OPENGLES3  201
#define __MENU_API_D3D11      202
#define __MENU_API_MIN __MENU_API_OPENGL4
#if defined(_WINDOWS)
# define __MENU_API_MAX __MENU_API_D3D11
#elif defined(__APPLE_)
# define __MENU_API_MAX __MENU_API_OPENGL4
#else
# define __MENU_API_MAX __MENU_API_OPENGLES3
#endif

#define __MENU_MOUSE_SENSIV_LOW   300
#define __MENU_MOUSE_SENSIV_AVG   301
#define __MENU_MOUSE_SENSIV_HIGH  302
#define __MENU_MOUSE_SENSIV_MIN __MENU_MOUSE_SENSIV_LOW
#define __MENU_MOUSE_SENSIV_MAX __MENU_MOUSE_SENSIV_HIGH

#define __MENU_SCN_FLT_NEAREST   400
#define __MENU_SCN_FLT_BILINEAR  401
#define __MENU_SCN_FLT_BICUBIC   402
#define __MENU_SCN_FLT_GAUSS     403
#define __MENU_SCN_FLT_LANCZOS   404
#define __MENU_SCN_FLT_MIN __MENU_SCN_FLT_NEAREST
#define __MENU_SCN_FLT_MAX __MENU_SCN_FLT_LANCZOS

#define __MENU_SCN_AA_OFF   500
#define __MENU_SCN_AA_MIN __MENU_SCN_AA_OFF
#define __MENU_SCN_AA_MAX __MENU_SCN_AA_OFF

#define __MENU_SCN_FX_OFF   600
#define __MENU_SCN_FX_MIN __MENU_SCN_FX_OFF
#define __MENU_SCN_FX_MAX __MENU_SCN_FX_OFF

#define __MENU_TEX_FLT_NEAREST  700
#define __MENU_TEX_FLT_BILINEAR 701
#define __MENU_TEX_FLT_BICUBIC  702
#define __MENU_TEX_FLT_GAUSS    703
#define __MENU_TEX_FLT_LANCZOS  704
#define __MENU_TEX_FLT_MIN __MENU_TEX_FLT_NEAREST
#define __MENU_TEX_FLT_MAX __MENU_TEX_FLT_LANCZOS

#define __MENU_SPR_FLT_NEAREST  800
#define __MENU_SPR_FLT_BILINEAR 801
#define __MENU_SPR_FLT_BICUBIC  802
#define __MENU_SPR_FLT_GAUSS    803
#define __MENU_SPR_FLT_LANCZOS  804
#define __MENU_SPR_FLT_MIN __MENU_SPR_FLT_NEAREST
#define __MENU_SPR_FLT_MAX __MENU_SPR_FLT_LANCZOS


# ifdef _WINDOWS
  // create main menu-bar
  scene::MenuManager::MenuManager(scene::Options& outSettings) {
    outSettings.api = scene::RenderingApi::openGL4;
    outSettings.mouseSensitivity = __MENU_MOUSE_SENSIV_AVG - __MENU_MOUSE_SENSIV_MIN;
    HMENU viewerPopup = CreatePopupMenu();
    AppendMenuW(viewerPopup, MF_STRING|MF_CHECKED, __MENU_API_D3D11, L"Direct3D 11");
    AppendMenuW(viewerPopup, MF_STRING|MF_GRAYED,  __MENU_API_OPENGL4, L"OpenGL 4");
    AppendMenuW(viewerPopup, MF_STRING|MF_GRAYED,  __MENU_API_OPENGLES3, L"OpenGLES 3");
    AppendMenuW(viewerPopup, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(viewerPopup, MF_STRING|MF_UNCHECKED, __MENU_MOUSE_SENSIV_LOW,  L"Low sensitivity");
    AppendMenuW(viewerPopup, MF_STRING|MF_CHECKED,   __MENU_MOUSE_SENSIV_AVG,  L"Average sensitivity");
    AppendMenuW(viewerPopup, MF_STRING|MF_UNCHECKED, __MENU_MOUSE_SENSIV_HIGH, L"High sensitivity");
    
    outSettings.scnFilter = scene::Interpolation::bilinear;
    outSettings.aa = scene::AntiAliasing::none;
    outSettings.fx = scene::VisualEffect::none;
    HMENU screenFilterPopup = CreatePopupMenu();
    AppendMenuW(screenFilterPopup, MF_STRING|MF_UNCHECKED, __MENU_SCN_FLT_NEAREST,  L"Nearest");
    AppendMenuW(screenFilterPopup, MF_STRING|MF_CHECKED,   __MENU_SCN_FLT_BILINEAR, L"Bilinear");
    AppendMenuW(screenFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_SCN_FLT_BICUBIC,  L"Bicubic");
    AppendMenuW(screenFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_SCN_FLT_GAUSS,    L"Gauss");
    AppendMenuW(screenFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_SCN_FLT_LANCZOS,  L"Lanczos");
    AppendMenuW(screenFilterPopup, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(screenFilterPopup, MF_STRING|MF_CHECKED, __MENU_SCN_AA_OFF, L"No AA");
    AppendMenuW(screenFilterPopup, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(screenFilterPopup, MF_STRING|MF_CHECKED, __MENU_SCN_FX_OFF, L"No effect");
    
    outSettings.texFilter = scene::Interpolation::bilinear;
    HMENU textureFilterPopup = CreatePopupMenu();
    AppendMenuW(textureFilterPopup, MF_STRING|MF_UNCHECKED, __MENU_TEX_FLT_NEAREST,  L"Nearest");
    AppendMenuW(textureFilterPopup, MF_STRING|MF_CHECKED,   __MENU_TEX_FLT_BILINEAR, L"Bilinear");
    AppendMenuW(textureFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_TEX_FLT_BICUBIC,  L"Bicubic");
    AppendMenuW(textureFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_TEX_FLT_GAUSS,    L"Gauss");
    AppendMenuW(textureFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_TEX_FLT_LANCZOS,  L"Lanczos");
    
    outSettings.sprFilter = scene::Interpolation::bilinear;
    HMENU spriteFilterPopup = CreatePopupMenu();
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_UNCHECKED, __MENU_SPR_FLT_NEAREST,  L"Nearest");
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_CHECKED,   __MENU_SPR_FLT_BILINEAR, L"Bilinear");
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_SPR_FLT_BICUBIC,  L"Bicubic");
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_SPR_FLT_GAUSS,    L"Gauss");
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_UNCHECKED,   __MENU_SPR_FLT_LANCZOS,  L"Lanczos");
    
    HMENU menuBar = CreateMenu();
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)viewerPopup, L"&Viewer");
    AppendMenuW(menuBar, MF_MENUBREAK, 0, nullptr);
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)screenFilterPopup, L"&Screen filter");
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)textureFilterPopup, L"&Texture filter");
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)spriteFilterPopup, L"S&prite filter");
    this->_resource = pandora::video::WindowResource::buildMenu(menuBar);
  }
  
  // adapt related checkboxes on change
  static void __selectMenuItem(HMENU menu, UINT id, UINT minVal, UINT maxVal) noexcept {
    for (UINT i = minVal; i <= maxVal; ++i) {
      if (i != id)
        CheckMenuItem(menu, i, MF_UNCHECKED);
    }
    CheckMenuItem(menu, id, MF_CHECKED);
  }

  // menu event management
  void scene::MenuManager::onMenuCommand(int32_t id) {
    switch (id/100) {
      case __MENU_API_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_API_MIN, __MENU_API_MAX);
        apiChangeHandler((scene::RenderingApi)(id - __MENU_API_MIN));
        break;
      case __MENU_MOUSE_SENSIV_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_MOUSE_SENSIV_MIN, __MENU_MOUSE_SENSIV_MAX);
        mouseSensivHandler(id - __MENU_MOUSE_SENSIV_MIN);
        break;
      case __MENU_SCN_AA_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_SCN_AA_MIN, __MENU_SCN_AA_MAX);
        aaChangeHandler((scene::AntiAliasing)(id - __MENU_SCN_AA_MIN));
        break;
      case __MENU_SCN_FX_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_SCN_FX_MIN, __MENU_SCN_FX_MAX);
        fxChangeHandler((scene::VisualEffect)(id - __MENU_SCN_FX_MIN));
        break;
      case __MENU_SCN_FLT_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_SCN_FLT_MIN, __MENU_SCN_FLT_MAX);
        screenFilterHandler((scene::Interpolation)(id - __MENU_SCN_FLT_MIN));
        break;
      case __MENU_TEX_FLT_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_TEX_FLT_MIN, __MENU_TEX_FLT_MAX);
        textureFilterHandler((scene::Interpolation)(id - __MENU_TEX_FLT_MIN));
        break;
      case __MENU_SPR_FLT_MIN/100:
        __selectMenuItem((HMENU)this->_resource->handle(), (UINT)id, __MENU_SPR_FLT_MIN, __MENU_SPR_FLT_MAX);
        spriteFilterHandler((scene::Interpolation)(id - __MENU_SPR_FLT_MIN));
        break;
      default: break;
    }
  }
  
#else
  // create main menu-bar
  scene::MenuManager::MenuManager(scene::Options& outSettings) {
  }

  // Menu handlers
  void scene::MenuManager::onMenuCommand(int32_t id) {
  }
#endif
