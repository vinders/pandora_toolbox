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

#define __MENU_RENDER_NONE        500
#define __MENU_RENDER_TEXTURE     501
#define __MENU_RENDER_WIREFRAME   502
#define __MENU_RENDER_SPLIT_WIRE  503
#define __MENU_RENDER_SPLIT_NOFX  504

#define __MENU_SCN_LIGHT_NONE    600
#define __MENU_SCN_LIGHT_BASE    601
#define __MENU_SCN_LIGHT_AO      602
#define __MENU_SCN_LIGHT_AO_BASE 603

#define __MENU_SPECIAL_MAP_RENDER_TEXTURE 701
#define __MENU_SPECIAL_FIRE_PARTICLES     702
#define __MENU_SPECIAL_REFRACTION_SHPERE  703
#define __MENU_SPECIAL_MOTION_BLUR        704

#define __MENU_TEX_FLT_NEAREST  800
#define __MENU_TEX_FLT_BILINEAR 801
#define __MENU_TEX_FLT_GAUSS    802
#define __MENU_TEX_FLT_BESSEL   803
#define __MENU_TEX_FLT_LANCZOS  804
#define __MENU_TEX_FLT_SPLINE16 805

#define __MENU_SPR_FLT_NEAREST  900
#define __MENU_SPR_FLT_BILINEAR 901
#define __MENU_SPR_FLT_GAUSS    902
#define __MENU_SPR_FLT_BESSEL   903
#define __MENU_SPR_FLT_LANCZOS  904
#define __MENU_SPR_FLT_SPLINE16 905

#define __MENU_TEX_UP_NONE      1000

#define __MENU_TEX_MIP_NONE      1100
#define __MENU_TEX_MIP_NEAREST   1101
#define __MENU_TEX_MIP_LINEAR    1102
#define __MENU_TEX_MIP_ANISO     1103

#define __MENU_TEX_MAP_NONE  1200
#define __MENU_TEX_MAP_TANG  1201
#define __MENU_TEX_MAP_OBJ   1202
#define __MENU_TEX_MAP_WORLD 1203

#define __MENU_SCN_GAMMA_LOWER   1300
#define __MENU_SCN_GAMMA_CORRECT 1301
#define __MENU_SCN_GAMMA_HIGHER  1302
#define __MENU_SCN_BLEND_OFF   1400
#define __MENU_SCN_BLEND_GRAY  1401
#define __MENU_SCN_BLEND_RED   1402
#define __MENU_SCN_BLEND_GREEN 1403
#define __MENU_SCN_BLEND_BLUE  1404
#define __MENU_SCN_FOG_OFF   1500
#define __MENU_SCN_FOG_LIGHT 1501
#define __MENU_SCN_FOG_DARK  1502
#define __MENU_SCN_HDR_OFF        1600
#define __MENU_SCN_HDR_BLOOM      1601
#define __MENU_SCN_HDR_LUMA       1602
#define __MENU_SCN_HDR_BLOOM_LUMA 1603
#define __MENU_SCN_SHARP_OFF         1700
#define __MENU_SCN_SHARP_GAUSS       1701
#define __MENU_SCN_SHARP_MEDIAN      1702
#define __MENU_SCN_SHARP_UNSHARPMASK 1703
#define __MENU_SCN_ART_OFF         1800
#define __MENU_SCN_ART_PENCIL1     1801
#define __MENU_SCN_ART_PENCIL2     1802
#define __MENU_SCN_ART_CEL1        1803
#define __MENU_SCN_ART_CEL2        1804
#define __MENU_SCN_ART_SOBEL       1805
#define __MENU_SCN_ART_PREWITT     1806
#define __MENU_SCN_ART_STORYBOOK   1807
#define __MENU_SCN_ART_BROKENGLASS 1808
#define __MENU_SCN_ART_CRT         1809
#define __MENU_SCN_NOISE_OFF    1900
#define __MENU_SCN_NOISE_PHOTO  1901
#define __MENU_SCN_NOISE_TV     1902

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
  viewerPopup.insertSeparator();

  this->_settings.mouseSensitivity = __MENU_MOUSE_SENSIV_AVG - __MENU_MOUSE_SENSIV_LOW;
  auto mouseSensitivityPopup = WindowMenu(true);
  mouseSensitivityPopup.insertItem(__MENU_MOUSE_SENSIV_LOW,  _SYSTEM_STR("Low"), WindowMenu::ItemType::radioOff);
  mouseSensitivityPopup.insertItem(__MENU_MOUSE_SENSIV_AVG,  _SYSTEM_STR("Average"), WindowMenu::ItemType::radioOn);
  mouseSensitivityPopup.insertItem(__MENU_MOUSE_SENSIV_HIGH, _SYSTEM_STR("High"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertSubMenu(std::move(mouseSensitivityPopup), _SYSTEM_STR("Mouse sensitivity"));
  this->_settings.renderMode = scene::RenderingMode::textured;
  auto viewportsPopup = WindowMenu(true);
  viewportsPopup.insertItem(__MENU_RENDER_NONE,       _SYSTEM_STR("Shaded"), WindowMenu::ItemType::radioOff);
  viewportsPopup.insertItem(__MENU_RENDER_TEXTURE,    _SYSTEM_STR("Textured"), WindowMenu::ItemType::radioOn);
  viewportsPopup.insertItem(__MENU_RENDER_WIREFRAME,  _SYSTEM_STR("Wireframe"), WindowMenu::ItemType::radioOff);
  viewportsPopup.insertItem(__MENU_RENDER_SPLIT_WIRE, _SYSTEM_STR("Wireframe / textured"), WindowMenu::ItemType::radioOff);
  viewportsPopup.insertItem(__MENU_RENDER_SPLIT_NOFX, _SYSTEM_STR("Raw / effects"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertSubMenu(std::move(viewportsPopup), _SYSTEM_STR("Viewport(s)"));
  this->_settings.light = scene::LightMode::lights;
  auto lightsPopup = WindowMenu(true);
  lightsPopup.insertItem(__MENU_SCN_LIGHT_NONE,    _SYSTEM_STR("No lights"), WindowMenu::ItemType::radioOff);
  lightsPopup.insertItem(__MENU_SCN_LIGHT_BASE,    _SYSTEM_STR("Lighting"),  WindowMenu::ItemType::radioOn);
  lightsPopup.insertItem(__MENU_SCN_LIGHT_AO,      _SYSTEM_STR("Ambient occlusion"), WindowMenu::ItemType::radioOff);
  lightsPopup.insertItem(__MENU_SCN_LIGHT_AO_BASE, _SYSTEM_STR("Lighting + AO"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertSubMenu(std::move(lightsPopup), _SYSTEM_STR("Lighting"));
  this->_settings.aa = scene::AntiAliasing::none;
  auto aaPopup = WindowMenu(true);
  aaPopup.insertItem(__MENU_SCN_AA_OFF,   _SYSTEM_STR("No AA"), WindowMenu::ItemType::radioOn);
  aaPopup.insertItem(__MENU_SCN_AA_FXAA,  _SYSTEM_STR("FXAA (fast/blur)"), WindowMenu::ItemType::radioOff);
  aaPopup.insertItem(__MENU_SCN_AA_SMAA2, _SYSTEM_STR("SMAA 2x (morph.)"), WindowMenu::ItemType::radioOff);
  aaPopup.insertItem(__MENU_SCN_AA_SMAA4, _SYSTEM_STR("SMAA 4x (morph.)"), WindowMenu::ItemType::radioOff);
  aaPopup.insertItem(__MENU_SCN_AA_SMAA8, _SYSTEM_STR("SMAA 8x (morph.)"), WindowMenu::ItemType::radioOff);
  aaPopup.insertItem(__MENU_SCN_AA_MSAA2, _SYSTEM_STR("MSAA 2x (multisample)"), WindowMenu::ItemType::radioOff);
  aaPopup.insertItem(__MENU_SCN_AA_MSAA4, _SYSTEM_STR("MSAA 4x (multisample)"), WindowMenu::ItemType::radioOff);
  aaPopup.insertItem(__MENU_SCN_AA_MSAA8, _SYSTEM_STR("MSAA 8x (multisample)"), WindowMenu::ItemType::radioOff);
  viewerPopup.insertSubMenu(std::move(aaPopup), _SYSTEM_STR("Anti-aliasing"));

  this->_settings.useMiniMap = true;
  this->_settings.useParticles = this->_settings.useRefraction = this->_settings.useMotionBlur = false;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_SPECIAL_MAP_RENDER_TEXTURE, _SYSTEM_STR("Mini-map view"), WindowMenu::ItemType::checkboxOn);
  viewerPopup.insertItem(__MENU_SPECIAL_REFRACTION_SHPERE,  _SYSTEM_STR("Refraction globe"), WindowMenu::ItemType::checkboxOff);
  viewerPopup.insertItem(__MENU_SPECIAL_FIRE_PARTICLES,     _SYSTEM_STR("Fire / particles"), WindowMenu::ItemType::checkboxOff);
  viewerPopup.insertItem(__MENU_SPECIAL_MOTION_BLUR,        _SYSTEM_STR("Motion blur"), WindowMenu::ItemType::checkboxOff);
  this->_settings.useVsync = false;
  viewerPopup.insertSeparator();
  viewerPopup.insertItem(__MENU_API_VSYNC, _SYSTEM_STR("Vsync"), WindowMenu::ItemType::checkboxOff);

  auto screenFilterPopup = WindowMenu(true);
  this->_settings.gamma = 0;
  auto gammaPopup = WindowMenu(true);
  gammaPopup.insertItem(__MENU_SCN_GAMMA_LOWER,   _SYSTEM_STR("Lower"), WindowMenu::ItemType::radioOff);
  gammaPopup.insertItem(__MENU_SCN_GAMMA_CORRECT, _SYSTEM_STR("Gamma correct"), WindowMenu::ItemType::radioOn);
  gammaPopup.insertItem(__MENU_SCN_GAMMA_HIGHER,  _SYSTEM_STR("Higher"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(gammaPopup), _SYSTEM_STR("Gamma correction"));
  this->_settings.colorBlend = ColorBlending::none;
  auto colorBlendPopup = WindowMenu(true);
  colorBlendPopup.insertItem(__MENU_SCN_BLEND_OFF,   _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  colorBlendPopup.insertItem(__MENU_SCN_BLEND_GRAY,  _SYSTEM_STR("Grayscale"), WindowMenu::ItemType::radioOff);
  colorBlendPopup.insertItem(__MENU_SCN_BLEND_RED,   _SYSTEM_STR("Red tinted"), WindowMenu::ItemType::radioOff);
  colorBlendPopup.insertItem(__MENU_SCN_BLEND_GREEN, _SYSTEM_STR("Green tinted"), WindowMenu::ItemType::radioOff);
  colorBlendPopup.insertItem(__MENU_SCN_BLEND_BLUE,  _SYSTEM_STR("Blue tinted"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(colorBlendPopup), _SYSTEM_STR("Color blending"));
  screenFilterPopup.insertSeparator();
  this->_settings.fog = FogFx::none;
  auto fogPopup = WindowMenu(true);
  fogPopup.insertItem(__MENU_SCN_FOG_OFF,   _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  fogPopup.insertItem(__MENU_SCN_FOG_LIGHT, _SYSTEM_STR("Light fog"), WindowMenu::ItemType::radioOff);
  fogPopup.insertItem(__MENU_SCN_FOG_DARK,  _SYSTEM_STR("Dark fog"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(fogPopup), _SYSTEM_STR("Fog effect"));
  this->_settings.hdrFx = HdrFx::none;
  auto hdrPopup = WindowMenu(true);
  hdrPopup.insertItem(__MENU_SCN_HDR_OFF,       _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  hdrPopup.insertItem(__MENU_SCN_HDR_BLOOM,     _SYSTEM_STR("Bloom"), WindowMenu::ItemType::radioOff);
  hdrPopup.insertItem(__MENU_SCN_HDR_LUMA,      _SYSTEM_STR("Luma sharpen"), WindowMenu::ItemType::radioOff);
  hdrPopup.insertItem(__MENU_SCN_HDR_BLOOM_LUMA,_SYSTEM_STR("Bloom + Luma"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(hdrPopup), _SYSTEM_STR("HDR effects"));
  this->_settings.sharpFx = SharpFx::none;
  auto sharpPopup = WindowMenu(true);
  sharpPopup.insertItem(__MENU_SCN_SHARP_OFF,        _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  sharpPopup.insertItem(__MENU_SCN_SHARP_GAUSS,      _SYSTEM_STR("Gaussian blur"), WindowMenu::ItemType::radioOff);
  sharpPopup.insertItem(__MENU_SCN_SHARP_MEDIAN,     _SYSTEM_STR("Median filter"), WindowMenu::ItemType::radioOff);
  sharpPopup.insertItem(__MENU_SCN_SHARP_UNSHARPMASK,_SYSTEM_STR("Unsharp masking"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(sharpPopup), _SYSTEM_STR("Sharpen/blur"));
  this->_settings.artFx = ArtFx::none;
  auto artPopup = WindowMenu(true);
  artPopup.insertItem(__MENU_SCN_ART_OFF,        _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  artPopup.insertItem(__MENU_SCN_ART_PENCIL1,    _SYSTEM_STR("Pencil V1"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_PENCIL2,    _SYSTEM_STR("Pencil V2"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_CEL1,       _SYSTEM_STR("Cel-shading V1"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_CEL2,       _SYSTEM_STR("Cel-shading V2"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_SOBEL,      _SYSTEM_STR("Neon (Sobel)"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_PREWITT,    _SYSTEM_STR("Neon (Prewitt)"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_STORYBOOK,  _SYSTEM_STR("Storybook"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_BROKENGLASS,_SYSTEM_STR("Broken glass"), WindowMenu::ItemType::radioOff);
  artPopup.insertItem(__MENU_SCN_ART_CRT,        _SYSTEM_STR("CRT vision"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(artPopup), _SYSTEM_STR("Artistic effects"));
  this->_settings.noise = NoiseFx::none;
  auto noisePopup = WindowMenu(true);
  noisePopup.insertItem(__MENU_SCN_NOISE_OFF,   _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  noisePopup.insertItem(__MENU_SCN_NOISE_PHOTO, _SYSTEM_STR("Photographic"), WindowMenu::ItemType::radioOff);
  noisePopup.insertItem(__MENU_SCN_NOISE_TV,    _SYSTEM_STR("TV / dynamic"), WindowMenu::ItemType::radioOff);
  screenFilterPopup.insertSubMenu(std::move(noisePopup), _SYSTEM_STR("Noise effects"));

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
  this->_settings.texMip = MipMap::linear;
  auto mipmapPopup = WindowMenu(true);
  mipmapPopup.insertItem(__MENU_TEX_MIP_NONE,    _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOff);
  mipmapPopup.insertItem(__MENU_TEX_MIP_NEAREST, _SYSTEM_STR("Nearest"), WindowMenu::ItemType::radioOff);
  mipmapPopup.insertItem(__MENU_TEX_MIP_LINEAR,  _SYSTEM_STR("Linear"), WindowMenu::ItemType::radioOn);
  mipmapPopup.insertItem(__MENU_TEX_MIP_ANISO,   _SYSTEM_STR("Anisotropic"), WindowMenu::ItemType::radioOff);
  textureFilterPopup.insertSubMenu(std::move(mipmapPopup), _SYSTEM_STR("Mip-mapping"));
  auto mappingPopup = WindowMenu(true);
  mappingPopup.insertItem(__MENU_TEX_MAP_NONE,  _SYSTEM_STR("OFF"), WindowMenu::ItemType::radioOn);
  mappingPopup.insertItem(__MENU_TEX_MAP_TANG,  _SYSTEM_STR("Tangent space"), WindowMenu::ItemType::radioOff);
  mappingPopup.insertItem(__MENU_TEX_MAP_OBJ,   _SYSTEM_STR("Object space"), WindowMenu::ItemType::radioOff);
  mappingPopup.insertItem(__MENU_TEX_MAP_WORLD, _SYSTEM_STR("World space"), WindowMenu::ItemType::radioOff);
  textureFilterPopup.insertSubMenu(std::move(mappingPopup), _SYSTEM_STR("Normal mapping"));
  
  auto spriteFilterPopup = WindowMenu(true);
  this->_settings.sprFilter = scene::Interpolation::bilinear;
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_NEAREST,  _SYSTEM_STR("Nearest"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_BILINEAR, _SYSTEM_STR("Bilinear"), WindowMenu::ItemType::radioOn);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_GAUSS,    _SYSTEM_STR("Gauss"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_BESSEL,   _SYSTEM_STR("Bessel"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_LANCZOS,  _SYSTEM_STR("Lanczos"), WindowMenu::ItemType::radioOff);
  spriteFilterPopup.insertItem(__MENU_SPR_FLT_SPLINE16, _SYSTEM_STR("Spline16"), WindowMenu::ItemType::radioOff);

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
    case __MENU_SCN_AA_OFF/100:
      if (this->_settings.aa != (scene::AntiAliasing)(id - __MENU_SCN_AA_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.aa + __MENU_SCN_AA_OFF);
        this->_settings.aa = (scene::AntiAliasing)(id - __MENU_SCN_AA_OFF);
        apiChangeHandler(scene::ApiChangeType::viewportChange);
      }
      break;
    case __MENU_RENDER_NONE/100:
      if (this->_settings.renderMode != (scene::RenderingMode)(id - __MENU_RENDER_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.renderMode + __MENU_RENDER_NONE);
        this->_settings.renderMode = (scene::RenderingMode)(id - __MENU_RENDER_NONE);
        apiChangeHandler(scene::ApiChangeType::viewportChange);
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
        case __MENU_SPECIAL_MOTION_BLUR:        this->_settings.useMotionBlur ^= true; value = this->_settings.useMotionBlur; break;
        default: break;
      }
      WindowMenu::changeCheckItemState(this->_resource->handle(), id, value);
      filterChangeHandler();
      break;
    }
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
    case __MENU_TEX_MIP_NONE/100:
      if (this->_settings.texMip != (scene::MipMap)(id - __MENU_TEX_MIP_NONE)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.texMip + __MENU_TEX_MIP_NONE);
        this->_settings.texMip = (scene::MipMap)(id - __MENU_TEX_MIP_NONE);
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
    case __MENU_SCN_GAMMA_LOWER/100:
      if (this->_settings.gamma != (-1 + id - __MENU_SCN_GAMMA_LOWER)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, static_cast<uint32_t>(this->_settings.gamma + 1) + __MENU_SCN_GAMMA_LOWER);
        this->_settings.gamma = -1 + id - __MENU_SCN_GAMMA_LOWER;
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_BLEND_OFF/100:
      if (this->_settings.colorBlend != (scene::ColorBlending)(id - __MENU_SCN_BLEND_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.colorBlend + __MENU_SCN_BLEND_OFF);
        this->_settings.colorBlend = (scene::ColorBlending)(id - __MENU_SCN_BLEND_OFF);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_FOG_OFF/100:
      if (this->_settings.fog != (scene::FogFx)(id - __MENU_SCN_FOG_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.fog + __MENU_SCN_FOG_OFF);
        this->_settings.fog = (scene::FogFx)(id - __MENU_SCN_FOG_OFF);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_HDR_OFF/100:
      if (this->_settings.hdrFx != (scene::HdrFx)(id - __MENU_SCN_HDR_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.hdrFx + __MENU_SCN_HDR_OFF);
        this->_settings.hdrFx = (scene::HdrFx)(id - __MENU_SCN_HDR_OFF);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_SHARP_OFF/100:
      if (this->_settings.sharpFx != (scene::SharpFx)(id - __MENU_SCN_SHARP_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.sharpFx + __MENU_SCN_SHARP_OFF);
        this->_settings.sharpFx = (scene::SharpFx)(id - __MENU_SCN_SHARP_OFF);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_ART_OFF/100:
      if (this->_settings.artFx != (scene::ArtFx)(id - __MENU_SCN_ART_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.artFx + __MENU_SCN_ART_OFF);
        this->_settings.artFx = (scene::ArtFx)(id - __MENU_SCN_ART_OFF);
        filterChangeHandler();
      }
      break;
    case __MENU_SCN_NOISE_OFF/100:
      if (this->_settings.noise != (scene::NoiseFx)(id - __MENU_SCN_NOISE_OFF)) {
        WindowMenu::changeCheckItemState(this->_resource->handle(), id, (uint32_t)this->_settings.noise + __MENU_SCN_NOISE_OFF);
        this->_settings.noise = (scene::NoiseFx)(id - __MENU_SCN_NOISE_OFF);
        filterChangeHandler();
      }
      break;
    default: break;
  }
}
