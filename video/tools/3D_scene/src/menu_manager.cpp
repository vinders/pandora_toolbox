/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include "menu_manager.h"
#if defined(_WINDOWS)
# define _SYSTEM_STR(str) L"" str
# define _P_MENU_LABEL(wstr,str) wstr
#else
# define _SYSTEM_STR(str) str
# define _P_MENU_LABEL(wstr,str) str
#endif

#define __MENU_API_VSYNC         190

#define __MENU_API_NONE       200
#define __MENU_API_OPENGL4    201
#define __MENU_API_OPENGLES3  202
#define __MENU_API_D3D11      203

#define __MENU_MOUSE_SENSIV_LOW   300
#define __MENU_MOUSE_SENSIV_AVG   301
#define __MENU_MOUSE_SENSIV_HIGH  302

#define __MENU_SCN_AA_OFF    400
#define __MENU_SCN_AA_FXAA   401
#define __MENU_SCN_AA_SMAA2  402
#define __MENU_SCN_AA_SMAA4  403
#define __MENU_SCN_AA_SMAA8  404
#define __MENU_SCN_AA_MSAA2  405
#define __MENU_SCN_AA_MSAA4  406
#define __MENU_SCN_AA_MSAA8  407

#define __MENU_SCN_FX_OFF         500
#define __MENU_SCN_FX_COLOR       501
#define __MENU_SCN_FX_CRTVISION   502
#define __MENU_SCN_FX_FOG         503
#define __MENU_SCN_FX_BLOOM       504
#define __MENU_SCN_FX_LUMASHARPEN 505
#define __MENU_SCN_FX_BROKENGLASS 506
#define __MENU_SCN_FX_STORYBOOK   507
#define __MENU_SCN_FX_PENCIL      508
#define __MENU_SCN_FX_NEON        509
#define __MENU_SCN_FX_CARTOON     510
#define __MENU_SCN_FX_MEDIAN      511
#define __MENU_SCN_FX_UNSHARPMASK 512

#define __MENU_SCN_FLT_NEAREST   600
#define __MENU_SCN_FLT_BILINEAR  601
#define __MENU_SCN_FLT_GAUSS     602
#define __MENU_SCN_FLT_BESSEL    603
#define __MENU_SCN_FLT_LANCZOS   604
#define __MENU_SCN_FLT_SPLINE16  605

#define __MENU_TEX_FLT_NEAREST  700
#define __MENU_TEX_FLT_BILINEAR 701
#define __MENU_TEX_FLT_GAUSS    702
#define __MENU_TEX_FLT_BESSEL   703
#define __MENU_TEX_FLT_LANCZOS  704
#define __MENU_TEX_FLT_SPLINE16 705

#define __MENU_SPR_FLT_NEAREST  800
#define __MENU_SPR_FLT_BILINEAR 801
#define __MENU_SPR_FLT_GAUSS    802
#define __MENU_SPR_FLT_BESSEL   803
#define __MENU_SPR_FLT_LANCZOS  804
#define __MENU_SPR_FLT_SPLINE16 805

#define __MENU_RENDER_NONE        900
#define __MENU_RENDER_TEXTURE     901
#define __MENU_RENDER_WIREFRAME   902
#define __MENU_RENDER_SPLIT_WIRE  903
#define __MENU_RENDER_SPLIT_NOFX  904

#define __MENU_TEX_UP_NONE      1000

#define __MENU_TEX_MAP_NONE     1100

#define __MENU_SPR_UP_NONE      1200

#define __MENU_SCN_LIGHT_NONE    1300
#define __MENU_SCN_LIGHT_BASE    1301
#define __MENU_SCN_LIGHT_AO      1302
#define __MENU_SCN_LIGHT_AO_BASE 1303

#define __MENU_SPECIAL_MAP_RENDER_TEXTURE 1401
#define __MENU_SPECIAL_FIRE_PARTICLES     1402
#define __MENU_SPECIAL_REFRACTION_SHPERE  1403

using namespace pandora::video;


// create main menu-bar
scene::MenuManager::MenuManager() {
  auto viewerPopup = WindowMenu(true);

# if defined(_VIDEO_D3D11_SUPPORT)
    this->_settings.api = scene::RenderingApi::d3d11;
    viewerPopup.insertItem(__MENU_API_D3D11, _SYSTEM_STR("Direct3D 11"), WindowMenu::ItemType::radioOn);
#   if defined(_VIDEO_OPENGL4_SUPPORT)
      viewerPopup.insertItem(__MENU_API_OPENGL4, _SYSTEM_STR("OpenGL 4"), WindowMenu::ItemType::radioOff);
#   endif
#   if defined(_VIDEO_OPENGLES3_SUPPORT)
      viewerPopup.insertItem(__MENU_API_OPENGL4, _SYSTEM_STR("OpenGLES 3"), WindowMenu::ItemType::radioOff);
#   endif

# elif defined(_VIDEO_OPENGL4_SUPPORT)
    this->_settings.api = scene::RenderingApi::openGL4;
    viewerPopup.insertItem(__MENU_API_OPENGL4, _SYSTEM_STR("OpenGL 4"), WindowMenu::ItemType::radioOn);
#   if defined(_VIDEO_OPENGLES3_SUPPORT)
      viewerPopup.insertItem(__MENU_API_OPENGL4, _SYSTEM_STR("OpenGLES 3"), WindowMenu::ItemType::radioOff);
#   endif

# elif defined(_VIDEO_OPENGLES3_SUPPORT)
    this->_settings.api = scene::RenderingApi::openGLES3;
    viewerPopup.insertItem(__MENU_API_OPENGLES3, _SYSTEM_STR("OpenGLES 3"), WindowMenu::ItemType::radioOn);
# else
    this->_settings.api = scene::RenderingApi::none;
    viewerPopup.insertItem(__MENU_API_NONE, _SYSTEM_STR("No renderer"), WindowMenu::ItemType::radioOn);
# endif

  this->_settings.mouseSensitivity = __MENU_MOUSE_SENSIV_AVG - __MENU_MOUSE_SENSIV_LOW;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_MOUSE_SENSIV_LOW,  _SYSTEM_STR("Low sensitivity"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertItem(__MENU_MOUSE_SENSIV_AVG,  _SYSTEM_STR("Average sensitivity"), WindowMenu::ItemType::radioOn);
  viewerPopup.insertItem(__MENU_MOUSE_SENSIV_HIGH,  _SYSTEM_STR("High sensitivity"), WindowMenu::ItemType::radioOff);
  this->_settings.renderMode = scene::RenderingMode::textured;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_RENDER_NONE,       _SYSTEM_STR("Shaded"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertItem(__MENU_RENDER_TEXTURE,    _SYSTEM_STR("Textured"), WindowMenu::ItemType::radioOn);
  viewerPopup.insertItem(__MENU_RENDER_WIREFRAME,  _SYSTEM_STR("Wireframe"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertItem(__MENU_RENDER_SPLIT_WIRE, _SYSTEM_STR("Wireframe / textured"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertItem(__MENU_RENDER_SPLIT_NOFX, _SYSTEM_STR("Raw / effects"), WindowMenu::ItemType::radioOff);
  this->_settings.light = scene::LightMode::lights;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_SCN_LIGHT_NONE,    _SYSTEM_STR("No lights"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertItem(__MENU_SCN_LIGHT_BASE,    _SYSTEM_STR("Lighting"),  WindowMenu::ItemType::radioOn);
  viewerPopup.insertItem(__MENU_SCN_LIGHT_AO,      _SYSTEM_STR("Ambient occlusion"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertItem(__MENU_SCN_LIGHT_AO_BASE, _SYSTEM_STR("Lighting + AO"), WindowMenu::ItemType::radioOff);
  this->_settings.useMiniMap = this->_settings.useParticles =this->_settings.useRefraction = false;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_SPECIAL_MAP_RENDER_TEXTURE, _SYSTEM_STR("Mini-map view"), WindowMenu::ItemType::checkboxOff);
  viewerPopup.insertItem(__MENU_SPECIAL_FIRE_PARTICLES,     _SYSTEM_STR("Fire / particles"), WindowMenu::ItemType::checkboxOff);
  viewerPopup.insertItem(__MENU_SPECIAL_REFRACTION_SHPERE,  _SYSTEM_STR("Refraction globe"), WindowMenu::ItemType::checkboxOff);
  this->_settings.useVsync = false;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_API_VSYNC, _SYSTEM_STR("Vsync"), WindowMenu::ItemType::checkboxOff);

  auto screenFilterPopup = WindowMenu(true);

  this->_settings.aa = scene::AntiAliasing::none;
  screenFilterPopup.insertItem(__MENU_SCN_AA_OFF,   _SYSTEM_STR("No AA"), WindowMenu::ItemType::radioOn);
  screenFilterPopup.insertItem(__MENU_SCN_AA_FXAA,  _SYSTEM_STR("FXAA (fast/blur)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_AA_SMAA2, _SYSTEM_STR("SMAA 2x (morph.)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_AA_SMAA4, _SYSTEM_STR("SMAA 4x (morph.)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_AA_SMAA8, _SYSTEM_STR("SMAA 8x (morph.)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_AA_MSAA2, _SYSTEM_STR("MSAA 2x (multisample)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_AA_MSAA4, _SYSTEM_STR("MSAA 4x (multisample)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_AA_MSAA8, _SYSTEM_STR("MSAA 8x (multisample)"), WindowMenu::ItemType::radioOff);
  this->_settings.fx = scene::VisualEffect::none;
  screenFilterPopup.insertSeparator();
  screenFilterPopup.insertItem(__MENU_SCN_FX_OFF,         _SYSTEM_STR("No effect"), WindowMenu::ItemType::radioOn);
  screenFilterPopup.insertItem(__MENU_SCN_FX_COLOR,       _SYSTEM_STR("Color blending"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_CRTVISION,   _SYSTEM_STR("CRT colors/scan-lines"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_FOG,         _SYSTEM_STR("Fog effect"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_BLOOM,       _SYSTEM_STR("Bloom effect"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_LUMASHARPEN, _SYSTEM_STR("Luma sharpen effect"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_BROKENGLASS, _SYSTEM_STR("Broken glass"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_STORYBOOK,   _SYSTEM_STR("Storybook mode"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_PENCIL,      _SYSTEM_STR("Pencil mode"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_NEON,        _SYSTEM_STR("Neon mode (edges)"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_CARTOON,     _SYSTEM_STR("Cartoon mode"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_MEDIAN,      _SYSTEM_STR("Median filter"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertItem(__MENU_SCN_FX_UNSHARPMASK, _SYSTEM_STR("Unsharp masking"), WindowMenu::ItemType::radioOff);

  auto textureFilterPopup = WindowMenu(true);

  this->_settings.texFilter = scene::Interpolation::bilinear;
  textureFilterPopup.insertItem(__MENU_TEX_FLT_NEAREST,  _SYSTEM_STR("Nearest"), WindowMenu::ItemType::radioOff);
  textureFilterPopup.insertItem(__MENU_TEX_FLT_BILINEAR, _SYSTEM_STR("Bilinear"), WindowMenu::ItemType::radioOn);
  textureFilterPopup.insertItem(__MENU_TEX_FLT_GAUSS,    _SYSTEM_STR("Gauss"), WindowMenu::ItemType::radioOff);
  textureFilterPopup.insertItem(__MENU_TEX_FLT_BESSEL,   _SYSTEM_STR("Bessel"), WindowMenu::ItemType::radioOff);
  textureFilterPopup.insertItem(__MENU_TEX_FLT_LANCZOS,  _SYSTEM_STR("Lanczos"), WindowMenu::ItemType::radioOff);
  textureFilterPopup.insertItem(__MENU_TEX_FLT_SPLINE16, _SYSTEM_STR("Spline16"), WindowMenu::ItemType::radioOff);
  this->_settings.texUpscale = Upscaling::none;
  textureFilterPopup.insertSeparator();
  textureFilterPopup.insertItem(__MENU_TEX_UP_NONE, _SYSTEM_STR("1x"), WindowMenu::ItemType::radioOn);
  this->_settings.texMapping = Mapping::none;
  textureFilterPopup.insertSeparator();
  textureFilterPopup.insertItem(__MENU_TEX_MAP_NONE, _SYSTEM_STR("No mapping"), WindowMenu::ItemType::radioOn);
  
  auto spriteFilterPopup = WindowMenu(true);

  this->_settings.sprFilter = scene::Interpolation::bilinear;
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_NEAREST,  _SYSTEM_STR("Nearest"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_BILINEAR, _SYSTEM_STR("Bilinear"), WindowMenu::ItemType::radioOn);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_GAUSS,    _SYSTEM_STR("Gauss"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_BESSEL,   _SYSTEM_STR("Bessel"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_LANCZOS,  _SYSTEM_STR("Lanczos"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_SPLINE16, _SYSTEM_STR("Spline16"), WindowMenu::ItemType::radioOff);
  this->_settings.sprUpscale = Upscaling::none;
  spriteFilterPopup.insertSeparator();
  spriteFilterPopup.insertItem(__MENU_SPR_UP_NONE, _SYSTEM_STR("1x"), WindowMenu::ItemType::radioOn);

  auto menuBar = WindowMenu(false);
  menuBar.insertSubMenu(std::move(viewerPopup), _P_MENU_LABEL(L"&Viewer", "Viewer"));
  menuBar.insertSeparator();
  menuBar.insertSubMenu(std::move(screenFilterPopup), _P_MENU_LABEL(L"&Screen filter", "Screen filter"));
  menuBar.insertSubMenu(std::move(textureFilterPopup), _P_MENU_LABEL(L"&Texture filter", "Texture filter"));
  menuBar.insertSubMenu(std::move(spriteFilterPopup), _P_MENU_LABEL(L"S&prite filter", "Sprite filter"));
  this->_resource = pandora::video::WindowResource::buildMenu(std::move(menuBar));
}

// menu event management
void scene::MenuManager::onMenuCommand(int32_t id) {
  switch (id/100) {
    case __MENU_API_NONE/100: {
      if (this->_settings.api != (scene::RenderingApi)(id - __MENU_API_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.api + __MENU_API_NONE);
        this->_settings.api = (scene::RenderingApi)(id - __MENU_API_NONE);
        apiChangeHandler(scene::ApiChangeType::rendererChange);
      }
      break;
    }
    case __MENU_API_VSYNC/100: {
      this->_settings.useVsync ^= true;
      WindowMenu::changeCheckItemState(this->_resource->handle(), id, this->_settings.useVsync);
      apiChangeHandler(scene::ApiChangeType::vsyncChange);
      break;
    }
    case __MENU_MOUSE_SENSIV_LOW/100:
      if (this->_settings.mouseSensitivity != (id - __MENU_MOUSE_SENSIV_LOW)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.mouseSensitivity + __MENU_MOUSE_SENSIV_LOW);
        this->_settings.mouseSensitivity = (id - __MENU_MOUSE_SENSIV_LOW);
        filterChangeHandler();
      }
      break;
    case __MENU_RENDER_NONE/100:
      if (this->_settings.renderMode != (scene::RenderingMode)(id - __MENU_RENDER_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.renderMode + __MENU_RENDER_NONE);
        this->_settings.renderMode = (scene::RenderingMode)(id - __MENU_RENDER_NONE);
        apiChangeHandler(scene::ApiChangeType::viewportChange);
      }
      break;
    case __MENU_SCN_AA_OFF/100:
      if (this->_settings.aa != (scene::AntiAliasing)(id - __MENU_SCN_AA_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.aa + __MENU_SCN_AA_OFF);
        this->_settings.aa = (scene::AntiAliasing)(id - __MENU_SCN_AA_OFF);
        apiChangeHandler(scene::ApiChangeType::viewportChange);
      }
      break;
    case __MENU_SCN_FX_OFF/100:
      if (this->_settings.fx != (scene::VisualEffect)(id - __MENU_SCN_FX_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.fx + __MENU_SCN_FX_OFF);
        this->_settings.fx = (scene::VisualEffect)(id - __MENU_SCN_FX_OFF);
        filterChangeHandler();
      }
      break;
    case __MENU_TEX_FLT_NEAREST/100:
      if (this->_settings.texFilter != (scene::Interpolation)(id - __MENU_TEX_FLT_NEAREST)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.texFilter + __MENU_TEX_FLT_NEAREST);
        this->_settings.texFilter = (scene::Interpolation)(id - __MENU_TEX_FLT_NEAREST);
        filterChangeHandler();
      }
      break;
    case __MENU_TEX_UP_NONE/100:
      if (this->_settings.texUpscale != (scene::Upscaling)(id - __MENU_TEX_UP_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.texUpscale + __MENU_TEX_UP_NONE);
        this->_settings.texUpscale = (scene::Upscaling)(id - __MENU_TEX_UP_NONE);
        filterChangeHandler();
      }
      break;
    case __MENU_TEX_MAP_NONE/100:
      if (this->_settings.texMapping != (scene::Mapping)(id - __MENU_TEX_MAP_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.texMapping + __MENU_TEX_MAP_NONE);
        this->_settings.texMapping = (scene::Mapping)(id - __MENU_TEX_MAP_NONE);
        filterChangeHandler();
      }
      break;
    case __MENU_SPR_FLT_NEAREST/100:
      if (this->_settings.sprFilter != (scene::Interpolation)(id - __MENU_SPR_FLT_NEAREST)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.sprFilter + __MENU_SPR_FLT_NEAREST);
        this->_settings.sprFilter = (scene::Interpolation)(id - __MENU_SPR_FLT_NEAREST);
        filterChangeHandler();
      }
      break;
    case __MENU_SPR_UP_NONE/100:
      if (this->_settings.sprUpscale != (scene::Upscaling)(id - __MENU_SPR_UP_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.sprUpscale + __MENU_SPR_UP_NONE);
        this->_settings.sprUpscale = (scene::Upscaling)(id - __MENU_SPR_UP_NONE);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_LIGHT_NONE/100:
      if (this->_settings.light != (scene::LightMode)(id - __MENU_SCN_LIGHT_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.light + __MENU_SCN_LIGHT_NONE);
        this->_settings.light = (scene::LightMode)(id - __MENU_SCN_LIGHT_NONE);
        filterChangeHandler();
      }
      break;
    case __MENU_SPECIAL_MAP_RENDER_TEXTURE/100: {
      bool value = false;
      switch (id) {
        case __MENU_SPECIAL_MAP_RENDER_TEXTURE: this->_settings.useMiniMap ^= true; value = this->_settings.useMiniMap; break;
        case __MENU_SPECIAL_FIRE_PARTICLES:     this->_settings.useParticles ^= true; value = this->_settings.useParticles; break;
        case __MENU_SPECIAL_REFRACTION_SHPERE:  this->_settings.useRefraction ^= true; value = this->_settings.useRefraction; break;
        default: break;
      }
      WindowMenu::changeCheckItemState(this->_resource->handle(), id, value);
      filterChangeHandler();
      break;
    }
    default: break;
  }
}
