#include <gtest/gtest.h>
#include <cstring>
#include <video/window_resource.h>
#ifdef _WINDOWS
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../_img/test_win32/resources.h"
# define _SYSTEM_STR(str) L"" str
#else
# define _SYSTEM_STR(str) str
#endif

using namespace pandora::video;

class WindowResourceTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- helpers --

static void _validateResource(std::shared_ptr<WindowResource> res, WindowResource::Category expectedType) {
  ASSERT_TRUE(res != nullptr);
  EXPECT_TRUE(*res && res->isValid());
  EXPECT_TRUE(res->handle() != nullptr);
  EXPECT_EQ(expectedType, res->type());
}

// -- Icon resources --

TEST_F(WindowResourceTest, buildSystemIconRes) {
  _validateResource(WindowResource::buildIcon(SystemIcon::app), WindowResource::Category::icon);
  _validateResource(WindowResource::buildIcon(SystemIcon::info), WindowResource::Category::icon);
  _validateResource(WindowResource::buildIcon(SystemIcon::question), WindowResource::Category::icon);
  _validateResource(WindowResource::buildIcon(SystemIcon::warning), WindowResource::Category::icon);
  _validateResource(WindowResource::buildIcon(SystemIcon::error), WindowResource::Category::icon);
  _validateResource(WindowResource::buildIcon(SystemIcon::security), WindowResource::Category::icon);
  _validateResource(WindowResource::buildIcon(SystemIcon::system), WindowResource::Category::icon);
  EXPECT_TRUE(WindowResource::buildIcon((SystemIcon)999999) == nullptr);
}

TEST_F(WindowResourceTest, buildFileIconRes) {
# ifdef _WINDOWS
    _validateResource(WindowResource::buildIconFromFile(_P_TEST_EMBED_RESOURCE_DIR "/logo.ico"), WindowResource::Category::icon);
# else
    _validateResource(WindowResource::buildIconFromFile(_P_TEST_RESOURCE_DIR "/logo.png"), WindowResource::Category::icon);
# endif
}

TEST_F(WindowResourceTest, buildPackageIconRes) {
# ifdef _WINDOWS
    pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)GetModuleHandle(nullptr));
    _validateResource(WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_ICON)), WindowResource::Category::icon);
# else
    _validateResource(WindowResource::buildIconFromPackage("logo.png"), WindowResource::Category::icon);
# endif
}

// -- Cursor resources --

TEST_F(WindowResourceTest, buildSystemCursorRes) {
  _validateResource(WindowResource::buildCursor(SystemCursor::pointer), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::hand), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::wait), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::waitPointer), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::textIBeam), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::help), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::forbidden), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::crosshair), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::arrowV), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::doubleArrowV), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::doubleArrowH), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::doubleArrow_NE_SW), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::doubleArrow_NW_SE), WindowResource::Category::cursor);
  _validateResource(WindowResource::buildCursor(SystemCursor::fourPointArrow), WindowResource::Category::cursor);
  EXPECT_TRUE(WindowResource::buildCursor((SystemCursor)999999) == nullptr);
}

TEST_F(WindowResourceTest, buildFileCursorRes) {
# ifdef _WINDOWS
    _validateResource(WindowResource::buildCursorFromFile(_P_TEST_EMBED_RESOURCE_DIR "/base.cur"), WindowResource::Category::cursor);
# else
    _validateResource(WindowResource::buildCursorFromFile(_P_TEST_RESOURCE_DIR "/base_cur.png"), WindowResource::Category::cursor);
# endif
}

TEST_F(WindowResourceTest, buildPackageCursorRes) {
# ifdef _WINDOWS
    pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)GetModuleHandle(nullptr));
    _validateResource(WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR)), WindowResource::Category::cursor);
# else
    _validateResource(WindowResource::buildCursorFromPackage("base_cur.png"), WindowResource::Category::cursor);
# endif
}

// -- Color brush resources --

TEST_F(WindowResourceTest, buildColorBrushRes) {
  _validateResource(WindowResource::buildColorBrush(WindowResource::systemWindowColor()), WindowResource::Category::colorBrush);
  _validateResource(WindowResource::buildColorBrush(WindowResource::systemTextColor()), WindowResource::Category::colorBrush);
  _validateResource(WindowResource::buildColorBrush(WindowResource::transparentColor()), WindowResource::Category::colorBrush);
  _validateResource(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0)), WindowResource::Category::colorBrush);
  _validateResource(WindowResource::buildColorBrush(WindowResource::rgbColor(255,255,255)), WindowResource::Category::colorBrush);
  _validateResource(WindowResource::buildColorBrush(WindowResource::rgbColor(64,128,96)), WindowResource::Category::colorBrush);
}

// -- Menu resources --

TEST_F(WindowResourceTest, portableMenuRes) {
  WindowMenu menubar(false);
  WindowMenu otherMenuBar(false);
  WindowMenu subMenu1(true);
  WindowMenu subMenu2(true);

  EXPECT_FALSE(menubar.isSubMenu());
  EXPECT_FALSE(otherMenuBar.isSubMenu());
  EXPECT_TRUE(subMenu1.isSubMenu());
  EXPECT_TRUE(subMenu2.isSubMenu());

  EXPECT_TRUE(subMenu1.insertItem(201u, _SYSTEM_STR("Text ON"), WindowMenu::ItemType::text, true));
  EXPECT_TRUE(subMenu1.insertItem(202u, _SYSTEM_STR("Text OFF"), WindowMenu::ItemType::text, false));
  subMenu1.insertSeparator();
  EXPECT_TRUE(subMenu1.insertItem(203u, _SYSTEM_STR("Check ON-ON"), WindowMenu::ItemType::checkboxOn, true));
  EXPECT_TRUE(subMenu1.insertItem(204u, _SYSTEM_STR("Check OFF-ON"), WindowMenu::ItemType::checkboxOff, true));
  EXPECT_TRUE(subMenu1.insertItem(205u, _SYSTEM_STR("Check OFF-OFF"), WindowMenu::ItemType::checkboxOff, false));
  subMenu1.insertSeparator();
  EXPECT_TRUE(subMenu1.insertItem(206u, _SYSTEM_STR("Check ON-OFF"), WindowMenu::ItemType::checkboxOn, false));

  EXPECT_TRUE(subMenu2.insertItem(207u, _SYSTEM_STR("Radio ON-ON"), WindowMenu::ItemType::radioOn, true));
  EXPECT_TRUE(subMenu2.insertItem(208u, _SYSTEM_STR("Radio OFF-ON"), WindowMenu::ItemType::radioOff, true));
  EXPECT_TRUE(subMenu2.insertItem(209u, _SYSTEM_STR("Radio OFF-OFF"), WindowMenu::ItemType::radioOff, false));
  subMenu2.insertSeparator();
  EXPECT_TRUE(subMenu2.insertItem(210u, _SYSTEM_STR("Radio ON-OFF"), WindowMenu::ItemType::radioOn, false));

  EXPECT_TRUE(menubar.insertSubMenu(std::move(subMenu1), _SYSTEM_STR("Test1")));
  EXPECT_TRUE(menubar.insertSubMenu(std::move(subMenu2), _SYSTEM_STR("Test2")));
  EXPECT_FALSE(menubar.insertSubMenu(std::move(otherMenuBar), _SYSTEM_STR("Test3")));
  menubar.insertSeparator();
  EXPECT_TRUE(menubar.insertItem(125u, _SYSTEM_STR("MainText"), WindowMenu::ItemType::text));
  auto resource = WindowResource::buildMenu(std::move(menubar));
  _validateResource(resource, WindowResource::Category::menu);

  WindowMenu moved = std::move(otherMenuBar);
  EXPECT_FALSE(moved.isSubMenu());
  otherMenuBar = std::move(moved);
  EXPECT_FALSE(otherMenuBar.isSubMenu());

  WindowMenu::changeItemState(resource->handle(), 201u, false);
  WindowMenu::changeItemState(resource->handle(), 202u, true);
  WindowMenu::changeCheckItemState(resource->handle(), 203u, false, false);
  WindowMenu::changeCheckItemState(resource->handle(), 204u, true, false);
  WindowMenu::changeCheckItemState(resource->handle(), 205u, true, true);
  WindowMenu::changeCheckItemState(resource->handle(), 206u, false, true);
  WindowMenu::changeCheckItemState(resource->handle(), 208u, 207u, true);
  WindowMenu::changeCheckItemState(resource->handle(), 209u, 208u, false);
  WindowMenu::changeCheckItemState(resource->handle(), 210u, false, true);

  WindowMenu otherSubMenu(true);
  EXPECT_TRUE(WindowResource::buildMenu(std::move(otherSubMenu)) == nullptr);
}

TEST_F(WindowResourceTest, nativeMenuRes) {
# ifdef _WINDOWS
    _validateResource(WindowResource::buildMenu(CreateMenu()), WindowResource::Category::menu);
# else
    auto menu = WindowResource::buildMenu((void*)12345);
    _validateResource(menu, WindowResource::Category::menu);
    if (menu != nullptr)
      memset((void*)&(*menu), 0, sizeof(WindowResource)); // set handle to 0 to avoid destruction of fake handle
# endif
}

