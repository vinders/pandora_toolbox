#include <gtest/gtest.h>
#include <cstring>
#include <video/window_resource.h>
#ifdef _WINDOWS
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../_img/test_win32/resources.h"
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
}

TEST_F(WindowResourceTest, buildFileIconRes) {
# ifdef _WINDOWS
    _validateResource(WindowResource::buildIconFromFile(_P_TEST_RESOURCE_DIR "/logo.ico"), WindowResource::Category::icon);
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
}

TEST_F(WindowResourceTest, buildFileCursorRes) {
# ifdef _WINDOWS
    _validateResource(WindowResource::buildCursorFromFile(_P_TEST_RESOURCE_DIR "/base.cur"), WindowResource::Category::cursor);
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

TEST_F(WindowResourceTest, menuRes) {
# ifdef _WINDOWS
    _validateResource(WindowResource::buildMenu(CreateMenu()), WindowResource::Category::menu);
# else
    auto menu = WindowResource::buildMenu((void*)12345)
    _validateResource(menu, WindowResource::Category::menu);
    if (menu != nullptr)
      memset((void*)&(*menu), 0, sizeof(WindowResource)); // set handle to 0 to avoid destruction of fake handle
# endif
}

