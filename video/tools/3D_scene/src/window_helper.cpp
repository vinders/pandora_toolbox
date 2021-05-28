/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <system/api/windows_api.h>
#include "window_helper.h"

#define __P_RES_MENU__VIEWER_D3D11     400
#define __P_RES_MENU__VIEWER_OPENGL4   401
#define __P_RES_MENU__VIEWER_OPENGLES3 402
#define __P_RES_MENU__VIEWER_API_MIN __P_RES_MENU__VIEWER_D3D11
#define __P_RES_MENU__VIEWER_API_MAX __P_RES_MENU__VIEWER_OPENGLES3

#define __P_RES_MENU__VIEWER_MOUSE_LOW  403
#define __P_RES_MENU__VIEWER_MOUSE_AVG  404
#define __P_RES_MENU__VIEWER_MOUSE_HIGH 405
#define __P_RES_MENU__VIEWER_MOUSE_MIN __P_RES_MENU__VIEWER_MOUSE_LOW
#define __P_RES_MENU__VIEWER_MOUSE_MAX __P_RES_MENU__VIEWER_MOUSE_HIGH

#define __P_RES_MENU__SCREEN_NEAREST  500
#define __P_RES_MENU__SCREEN_BILINEAR 501
#define __P_RES_MENU__SCREEN_MIN __P_RES_MENU__SCREEN_NEAREST
#define __P_RES_MENU__SCREEN_MAX __P_RES_MENU__SCREEN_BILINEAR

#define __P_RES_MENU__SCREEN_AA_OFF   550
#define __P_RES_MENU__SCREEN_FX_OFF   560

#define __P_RES_MENU__TEX_NEAREST  600
#define __P_RES_MENU__TEX_BILINEAR 601
#define __P_RES_MENU__TEX_MIN __P_RES_MENU__TEX_NEAREST
#define __P_RES_MENU__TEX_MAX __P_RES_MENU__TEX_BILINEAR

#define __P_RES_MENU__SPR_NEAREST  700
#define __P_RES_MENU__SPR_BILINEAR 701
#define __P_RES_MENU__SPR_MIN __P_RES_MENU__SPR_NEAREST
#define __P_RES_MENU__SPR_MAX __P_RES_MENU__SPR_BILINEAR


// Find appropriate window client size
pandora::video::PixelSize scene::computeWindowClientSize(std::shared_ptr<pandora::hardware::DisplayMonitor> primaryMonitor) {
  pandora::video::PixelSize windowSize{ _P_DEFAULT_WIDTH, _P_DEFAULT_HEIGHT };
  
  if (primaryMonitor && windowSize.height + 60 > primaryMonitor->attributes().workArea.height) {
    windowSize.height = primaryMonitor->attributes().workArea.height - 64;
    if (windowSize.height < _P_MIN_HEIGHT)
      windowSize.height = _P_MIN_HEIGHT;
    windowSize.width = static_cast<uint32_t>(_P_RATIO * (double)windowSize.height + 0.5);
  }
  return windowSize;
}

// ---

// Create main menu-bar
std::shared_ptr<pandora::video::WindowResource> scene::createMainMenu() {
# ifdef _WINDOWS
    HMENU viewerPopup = CreatePopupMenu();
    AppendMenuW(viewerPopup, MF_STRING|MF_CHECKED, __P_RES_MENU__VIEWER_D3D11, L"&Direct3D 11");
    AppendMenuW(viewerPopup, MF_STRING|MF_GRAYED,  __P_RES_MENU__VIEWER_OPENGL4, L"&OpenGL 4");
    AppendMenuW(viewerPopup, MF_STRING|MF_GRAYED,  __P_RES_MENU__VIEWER_OPENGLES3, L"OpenGL&ES 3");
    AppendMenuW(viewerPopup, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(viewerPopup, MF_STRING|MF_UNCHECKED, __P_RES_MENU__VIEWER_MOUSE_LOW, L"&Low mouse sensitivity");
    AppendMenuW(viewerPopup, MF_STRING|MF_CHECKED,   __P_RES_MENU__VIEWER_MOUSE_AVG, L"&Average mouse sensitivity");
    AppendMenuW(viewerPopup, MF_STRING|MF_UNCHECKED, __P_RES_MENU__VIEWER_MOUSE_HIGH, L"&High mouse sensitivity");
    
    HMENU screenFilterPopup = CreatePopupMenu();
    AppendMenuW(screenFilterPopup, MF_STRING|MF_UNCHECKED, __P_RES_MENU__SCREEN_NEAREST, L"Nearest");
    AppendMenuW(screenFilterPopup, MF_STRING|MF_CHECKED,   __P_RES_MENU__SCREEN_BILINEAR, L"Bilinear");
    AppendMenuW(screenFilterPopup, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(screenFilterPopup, MF_STRING|MF_CHECKED, __P_RES_MENU__SCREEN_AA_OFF, L"No AA");
    AppendMenuW(screenFilterPopup, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(screenFilterPopup, MF_STRING|MF_CHECKED, __P_RES_MENU__SCREEN_FX_OFF, L"No effect");
    
    HMENU textureFilterPopup = CreatePopupMenu();
    AppendMenuW(textureFilterPopup, MF_STRING|MF_UNCHECKED, __P_RES_MENU__TEX_NEAREST, L"Nearest");
    AppendMenuW(textureFilterPopup, MF_STRING|MF_CHECKED,   __P_RES_MENU__TEX_BILINEAR, L"Bilinear");
    
    HMENU spriteFilterPopup = CreatePopupMenu();
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_UNCHECKED, __P_RES_MENU__SPR_NEAREST, L"Nearest");
    AppendMenuW(spriteFilterPopup, MF_STRING|MF_CHECKED,   __P_RES_MENU__SPR_BILINEAR, L"Bilinear");
    
    HMENU menuBar = CreateMenu();
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)viewerPopup, L"&Viewer");
    AppendMenuW(menuBar, MF_MENUBREAK, 0, nullptr);
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)screenFilterPopup, L"&Screen filter");
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)textureFilterPopup, L"&Texture filter");
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)spriteFilterPopup, L"S&prite filter");
    return pandora::video::WindowResource::buildMenu(menuBar);
    
# else
    return nullptr;
# endif
}

// Menu handlers
void scene::onMenuCommand(pandora::video::WindowResource& menu, int32_t id) noexcept {
# ifdef _WINDOWS
    UINT minDisabled = 0, maxDisabled = 0;
    switch (id) {
      case __P_RES_MENU__VIEWER_D3D11:
      case __P_RES_MENU__VIEWER_OPENGL4:
      case __P_RES_MENU__VIEWER_OPENGLES3: minDisabled = __P_RES_MENU__VIEWER_API_MIN; maxDisabled = __P_RES_MENU__VIEWER_API_MAX; break;
      case __P_RES_MENU__VIEWER_MOUSE_LOW: 
      case __P_RES_MENU__VIEWER_MOUSE_AVG: 
      case __P_RES_MENU__VIEWER_MOUSE_HIGH: minDisabled = __P_RES_MENU__VIEWER_MOUSE_MIN; maxDisabled = __P_RES_MENU__VIEWER_MOUSE_MAX; break;
      case __P_RES_MENU__SCREEN_NEAREST:
      case __P_RES_MENU__SCREEN_BILINEAR: minDisabled = __P_RES_MENU__SCREEN_MIN; maxDisabled = __P_RES_MENU__SCREEN_MAX; break;
      case __P_RES_MENU__SCREEN_AA_OFF: break;
      case __P_RES_MENU__SCREEN_FX_OFF: break;

      case __P_RES_MENU__TEX_NEAREST:
      case __P_RES_MENU__TEX_BILINEAR: minDisabled = __P_RES_MENU__TEX_MIN; maxDisabled = __P_RES_MENU__TEX_MAX; break;

      case __P_RES_MENU__SPR_NEAREST:
      case __P_RES_MENU__SPR_BILINEAR: minDisabled = __P_RES_MENU__SPR_MIN; maxDisabled = __P_RES_MENU__SPR_MAX; break;
      default: return;
    }

    for (UINT i = minDisabled; i <= maxDisabled; ++i) {
      if (i != (UINT)id)
        CheckMenuItem((HMENU)menu.handle(), i, MF_UNCHECKED);
    }
    CheckMenuItem((HMENU)menu.handle(), (UINT)id, MF_CHECKED);
# endif
}
void scene::onMenuKey(pandora::video::WindowResource& menu, int32_t charCode) noexcept {
# ifdef _WINDOWS
# endif
}
