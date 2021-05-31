#include <gtest/gtest.h>
#include <video/window.h>
#ifdef _WINDOWS
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../_img/test_win32/resources.h"
# define _STR(str) L"" str
#else
# define _STR(str) str
#endif

using namespace pandora::video;

struct WindowParams {
  WindowType mode = WindowType::window;
  WindowBehavior behavior = WindowBehavior::none;
  ResizeMode resizeMode = ResizeMode::fixed;
  bool hasParent = false;
  uint32_t scrollRangeX = 0;
  uint32_t scrollRangeY = 0;
  
  pandora::hardware::DisplayArea clientArea{ 0,0,0,0 };
};

// ---

class WindowTest : public testing::Test {
public:
  static void _testWindow(std::unique_ptr<Window>& window, const WindowParams& params, bool isUpdate = false);
  
  static bool _onWindowEvent(Window*, WindowEvent event, uint32_t, int32_t, int32_t, void*);
  static bool _onPositionEvent(Window* sender, PositionEvent event, int32_t, int32_t, uint32_t sizeX, uint32_t sizeY);
  static bool _onKeyboardEvent(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t);
  static bool _onMouseEvent(Window*, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t);

protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override { 
    _isCloseAllowed = false; 
#   ifdef _WINDOWS
      pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)GetModuleHandle(nullptr));
#   endif
  }
  void TearDown() override {}
  
  static bool _isCloseAllowed;
  static int32_t _lastPosX;
  static int32_t _lastPosY;
  static uint32_t _lastSizeX;
  static uint32_t _lastSizeY;
};
bool WindowTest::_isCloseAllowed = false;
int32_t WindowTest::_lastPosX = 0;
int32_t WindowTest::_lastPosY = 0;
uint32_t WindowTest::_lastSizeX = 0;
uint32_t WindowTest::_lastSizeY = 0;

// ---

// systems ready
#if defined(_WINDOWS)
# define __ENABLE_TESTS 1
#endif

#ifdef __ENABLE_TESTS
  // -- handlers --

  bool WindowTest::_onWindowEvent(Window*, WindowEvent event, uint32_t, int32_t, int32_t, void*) {
    switch (event) {
      case WindowEvent::windowClosed:
        if (!_isCloseAllowed)
          return true; // cancel close event
        break;
      default: break;
    }
    return false;
  }
  bool WindowTest::_onPositionEvent(Window*, PositionEvent event, int32_t posX, int32_t posY, uint32_t sizeX, uint32_t sizeY) {
    switch (event) {
      case PositionEvent::sizePositionChanged: 
        _lastPosX = posX;
        _lastPosY = posY;
        _lastSizeX = sizeX; 
        _lastSizeY = sizeY; 
        break;
      default: break;
    }
    return false;
  }
  bool WindowTest::_onKeyboardEvent(Window*, KeyboardEvent, uint32_t, uint32_t) { return false; }
  bool WindowTest::_onMouseEvent(Window*, MouseEvent, int32_t, int32_t, int32_t, uint8_t) { return false;
  }


  // -- window test procedure --
  
  void WindowTest::_testWindow(std::unique_ptr<Window>& window, const WindowParams& params, bool isUpdate) {
    ASSERT_TRUE(window != nullptr);
    EXPECT_NO_THROW(Window::getLastError());
    EXPECT_TRUE(Window::pollEvents());
    EXPECT_TRUE(window->pollCurrentWindowEvents());

    auto primaryMonitor = std::make_shared<pandora::hardware::DisplayMonitor>();
    const auto& workArea = primaryMonitor->attributes().workArea;
    int32_t offsetX = (params.mode == WindowType::fullscreen || params.mode == WindowType::borderless) ? 0 : workArea.x;
    int32_t offsetY = (params.mode == WindowType::fullscreen || params.mode == WindowType::borderless) ? 0 : workArea.y;
    
    //set handlers
    window->setWindowHandler(&WindowTest::_onWindowEvent);
    window->setPositionHandler(&WindowTest::_onPositionEvent);
    window->setKeyboardHandler(&WindowTest::_onKeyboardEvent);
    window->setMouseHandler(&WindowTest::_onMouseEvent);
    
    // verify style params
    EXPECT_TRUE(window->handle() != (WindowHandle)0);
    EXPECT_EQ(params.mode, window->displayMode());
    EXPECT_EQ(params.behavior, window->behavior());
    EXPECT_EQ(params.resizeMode, window->resizeMode());
    EXPECT_EQ(params.hasParent, window->hasParent());
    if (isUpdate) {
      EXPECT_TRUE((window->visibleState() & Window::VisibleState::visible) == true);
    }
    else {
      EXPECT_EQ(Window::VisibleState::none, window->visibleState());
    }

    EXPECT_TRUE(window->setMinClientAreaSize((params.clientArea.width != Window::Builder::defaultSize()) ? params.clientArea.width/2 : 250,
                                             (params.clientArea.height != Window::Builder::defaultSize()) ? params.clientArea.height/2 : 150) );
    if (params.scrollRangeX > 0 || params.scrollRangeY > 0) {
      EXPECT_TRUE(window->setScrollbarRange(0,0,(uint16_t)params.scrollRangeX,(uint16_t)params.scrollRangeY));
    }
    if (!isUpdate) {
      window->show();
      EXPECT_TRUE(window->pollCurrentWindowEvents());
      EXPECT_TRUE(window->handle() != (WindowHandle)0);
      EXPECT_EQ(params.mode, window->displayMode());
      EXPECT_EQ(params.behavior, window->behavior());
      EXPECT_EQ(params.resizeMode, window->resizeMode());
      EXPECT_EQ(params.hasParent, window->hasParent());
      EXPECT_EQ(primaryMonitor->attributes().id, window->displayMonitor().attributes().id);
      EXPECT_TRUE((window->visibleState() & Window::VisibleState::visible) == true);
    }
    
    // verify window metrics
    auto actualClientArea = window->getClientArea();
    auto actualWindowArea = window->getWindowArea();
    if (!params.hasParent) {
      if (params.clientArea.x != Window::Builder::defaultPosition() && params.clientArea.x != Window::Builder::centeredPosition()) {
        EXPECT_TRUE(params.clientArea.x + offsetX >= actualClientArea.x - 16 && params.clientArea.x + offsetX <= actualClientArea.x + 16);
      }
      else if (params.clientArea.x == Window::Builder::centeredPosition() && params.mode != WindowType::fullscreen) {
        EXPECT_TRUE(actualClientArea.x - offsetX != 0);
      }
      if (params.clientArea.y != Window::Builder::defaultPosition() && params.clientArea.y != Window::Builder::centeredPosition()) {
        EXPECT_TRUE(params.clientArea.y + offsetY >= actualClientArea.y - 64 && params.clientArea.y + offsetY <= actualClientArea.y + 64);
      }
      else if (params.clientArea.y == Window::Builder::centeredPosition() && params.mode != WindowType::fullscreen) {
        EXPECT_TRUE(actualClientArea.y - offsetY != 0);
      }
    }
    if (params.clientArea.width != Window::Builder::defaultSize()) {
      EXPECT_EQ(params.clientArea.width, actualClientArea.width);
    }
    if (params.clientArea.height != Window::Builder::defaultSize()) {
      EXPECT_EQ(params.clientArea.height, actualClientArea.height);
    }
    EXPECT_EQ(actualClientArea.width, window->getClientSize().width);
    EXPECT_EQ(actualClientArea.height, window->getClientSize().height);
    EXPECT_TRUE(actualClientArea.x >= actualWindowArea.x);
    EXPECT_TRUE(actualClientArea.y >= actualWindowArea.y);
    EXPECT_TRUE(actualClientArea.width <= actualWindowArea.width);
    EXPECT_TRUE(actualClientArea.height <= actualWindowArea.height);
    EXPECT_TRUE(window->contentScale() >= 1.0f);
    
    // verify cursor metrics
    EXPECT_EQ(window->getCursorPosition(Window::CursorPositionType::relative).x + actualClientArea.x, window->getCursorPosition(Window::CursorPositionType::absolute).x);
    EXPECT_EQ(window->getCursorPosition(Window::CursorPositionType::relative).y + actualClientArea.y, window->getCursorPosition(Window::CursorPositionType::absolute).y);
    
    EXPECT_TRUE(window->setCursorPosition(10,12,Window::CursorPositionType::relative));
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_EQ(10, window->getCursorPosition(Window::CursorPositionType::relative).x);
    EXPECT_EQ(12, window->getCursorPosition(Window::CursorPositionType::relative).y);
    EXPECT_TRUE(window->setCursorPosition(10,12,Window::CursorPositionType::absolute));
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_EQ(10, window->getCursorPosition(Window::CursorPositionType::absolute).x);
    EXPECT_EQ(12, window->getCursorPosition(Window::CursorPositionType::absolute).y);

    // verify scroll metrics
    EXPECT_TRUE(window->getScrollPositionH() <= (int32_t)params.scrollRangeX);
    EXPECT_TRUE(window->getScrollPositionV() <= (int32_t)params.scrollRangeY);
    EXPECT_EQ(window->getScrollPositionH(), window->getScrollPosition().x);
    EXPECT_EQ(window->getScrollPositionV(), window->getScrollPosition().y);
    
    if (params.scrollRangeX > 0 || params.scrollRangeY > 0) {
      EXPECT_TRUE(window->setScrollbarRange(5,10,600,800));
      EXPECT_FALSE(window->setScrollbarRange(600,600,5,5));
      EXPECT_TRUE(window->pollCurrentWindowEvents());
      EXPECT_EQ(5, window->getScrollPositionH());
      EXPECT_EQ(10, window->getScrollPositionV());
      EXPECT_TRUE(window->setScrollPositionH(20));
      EXPECT_TRUE(window->setScrollPositionV(30));
      EXPECT_TRUE(window->pollCurrentWindowEvents());
      EXPECT_EQ(20, window->getScrollPositionH());
      EXPECT_EQ(30, window->getScrollPositionV());
      EXPECT_FALSE(window->setScrollPositionH(2000));
      EXPECT_FALSE(window->setScrollPositionV(3000));
      EXPECT_EQ(20, window->getScrollPositionH());
      EXPECT_EQ(30, window->getScrollPositionV());
    }
    
    // resources
#   ifdef _WINDOWS
      EXPECT_TRUE(window->setCursor(WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR))));
#   else
      EXPECT_TRUE(window->setCursor(WindowResource::buildCursorFromPackage("/base_cur.png")));
#   endif
    EXPECT_TRUE(window->setCursor(WindowResource::buildCursor(SystemCursor::pointer)));
    EXPECT_FALSE(window->setCursor(nullptr));
    
    EXPECT_TRUE(window->setCaption(nullptr));
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_TRUE(window->getCaption() == _STR(""));
    EXPECT_TRUE(window->setCaption(_STR("Abc 42")));
    EXPECT_TRUE(window->pollCurrentWindowEvents());
#   ifndef __MINGW32__
      EXPECT_TRUE(window->getCaption() == _STR("Abc 42"));
#   endif
    
    EXPECT_TRUE(window->setMenu(nullptr)); // remove menubar
    
    EXPECT_TRUE(window->setBackgroundColorBrush(WindowResource::buildColorBrush(WindowResource::systemWindowColor())));
    EXPECT_TRUE(window->setBackgroundColorBrush(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0))));
    EXPECT_FALSE(window->setBackgroundColorBrush(nullptr));
    
    // clear
    EXPECT_TRUE(window->clearClientArea());
    EXPECT_TRUE(window->clear(pandora::hardware::DisplayArea{ 10,10,20u,20u }));
    EXPECT_TRUE(window->pollCurrentWindowEvents());

    // move/resize
    if ((params.resizeMode & ResizeMode::resizable) == ResizeMode::resizable) {
      if (params.mode != WindowType::fullscreen) {
        if (!params.hasParent) {
          window->move(80, 30);
          EXPECT_TRUE(window->pollCurrentWindowEvents());
          auto movedClientArea = window->getClientArea();
          EXPECT_EQ(80 + offsetX, movedClientArea.x);
          EXPECT_EQ(30 + offsetY, movedClientArea.y);
          EXPECT_EQ(actualClientArea.width, movedClientArea.width);
          EXPECT_EQ(actualClientArea.height, movedClientArea.height);
          EXPECT_EQ(80 + offsetX, _lastPosX);
          EXPECT_EQ(30 + offsetY, _lastPosY);
          EXPECT_EQ(actualClientArea.width, _lastSizeX);
          EXPECT_EQ(actualClientArea.height, _lastSizeY);

          window->resize(pandora::hardware::DisplayArea{ 70,36,320,220 });
          EXPECT_TRUE(window->pollCurrentWindowEvents());
          movedClientArea = window->getClientArea();
          EXPECT_EQ(70 + offsetX, movedClientArea.x);
          EXPECT_EQ(36 + offsetY, movedClientArea.y);
          EXPECT_EQ((uint32_t)320, movedClientArea.width);
          EXPECT_EQ((uint32_t)220, movedClientArea.height);
          EXPECT_EQ(70 + offsetX, _lastPosX);
          EXPECT_EQ(36 + offsetY, _lastPosY);
          EXPECT_EQ((uint32_t)320, _lastSizeX);
          EXPECT_EQ((uint32_t)220, _lastSizeY);

          actualClientArea = movedClientArea;
        }

        window->resize(640,480);
        EXPECT_TRUE(window->pollCurrentWindowEvents());
        auto resizedClientArea = window->getClientArea();
        EXPECT_EQ(actualClientArea.x, resizedClientArea.x);
        EXPECT_EQ(actualClientArea.y, resizedClientArea.y);
        EXPECT_EQ((uint32_t)640, resizedClientArea.width);
        EXPECT_EQ((uint32_t)480, resizedClientArea.height);
      }
      
      if (params.mode != WindowType::fullscreen) {
        // maximize/restore
        EXPECT_TRUE(window->show(Window::VisibilityCommand::maximize));
        EXPECT_TRUE(window->pollCurrentWindowEvents());
        EXPECT_TRUE((window->visibleState() & (Window::VisibleState::visible | Window::VisibleState::maximized | Window::VisibleState::minimized)) 
                    == (Window::VisibleState::visible | Window::VisibleState::maximized));
        EXPECT_TRUE(window->show(Window::VisibilityCommand::restore));
        EXPECT_TRUE(window->pollCurrentWindowEvents());
        EXPECT_TRUE((window->visibleState() & (Window::VisibleState::visible | Window::VisibleState::maximized | Window::VisibleState::minimized)) 
                    == Window::VisibleState::visible);
        
        // minimize/restore
        EXPECT_TRUE(window->show(Window::VisibilityCommand::minimize));
        EXPECT_TRUE(window->pollCurrentWindowEvents());
        EXPECT_TRUE((window->visibleState() & (Window::VisibleState::visible | Window::VisibleState::maximized | Window::VisibleState::minimized)) 
                    == Window::VisibleState::minimized);
        EXPECT_TRUE(window->show(Window::VisibilityCommand::restore));
        EXPECT_TRUE(window->pollCurrentWindowEvents());
        EXPECT_TRUE((window->visibleState() & (Window::VisibleState::visible | Window::VisibleState::maximized | Window::VisibleState::minimized)) 
                    == Window::VisibleState::visible);
      }
    }
    if (params.mode != WindowType::fullscreen) {
      // hide/show
      EXPECT_TRUE(window->show(Window::VisibilityCommand::hide));
      EXPECT_TRUE(window->pollCurrentWindowEvents());
      EXPECT_TRUE((window->visibleState() & (Window::VisibleState::visible | Window::VisibleState::maximized | Window::VisibleState::minimized))
                  == Window::VisibleState::none);
      EXPECT_TRUE(window->show(Window::VisibilityCommand::show));
      EXPECT_TRUE(window->pollCurrentWindowEvents());
      EXPECT_TRUE((window->visibleState() & (Window::VisibleState::visible | Window::VisibleState::maximized | Window::VisibleState::minimized))
                  == Window::VisibleState::visible);
    }

    // cursor mode
    window->setCursorMode(Window::CursorMode::clipped);
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_TRUE(window->getCursorPosition(Window::CursorPositionType::relative).x >= 0);
    EXPECT_TRUE(window->getCursorPosition(Window::CursorPositionType::relative).y >= 0);
    EXPECT_EQ(Window::CursorMode::clipped, window->getCursorMode());
    
    window->setCursorMode(Window::CursorMode::hidden);
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_EQ(Window::CursorMode::hidden, window->getCursorMode());
    window->setCursorMode(Window::CursorMode::visible);
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_EQ(Window::CursorMode::visible, window->getCursorMode());
    
    // display mode
    WindowType mode = (params.mode != WindowType::window) ? WindowType::window : WindowType::dialog;
    pandora::hardware::DisplayArea newModeArea{ 30,20,320,220 };
    EXPECT_TRUE(window->setDisplayMode(mode, WindowBehavior::none, ResizeMode::fixed, newModeArea));
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_EQ(mode, window->displayMode());
    EXPECT_EQ(WindowBehavior::none, window->behavior());
    EXPECT_EQ(ResizeMode::fixed, window->resizeMode());
    auto newModeRealArea = window->getClientArea();
    if (!params.hasParent) {
      EXPECT_EQ(newModeArea.x + workArea.x, newModeRealArea.x);
      EXPECT_EQ(newModeArea.y + workArea.y, newModeRealArea.y);
    }
    EXPECT_EQ(newModeArea.width, newModeRealArea.width);
    EXPECT_EQ(newModeArea.height, newModeRealArea.height);

    // restore original mode
    EXPECT_TRUE(window->setDisplayMode(params.mode, params.behavior, params.resizeMode, params.clientArea, 60));
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_EQ(params.mode, window->displayMode());
    EXPECT_EQ(params.behavior, window->behavior());
    EXPECT_EQ(params.resizeMode, window->resizeMode());
    auto finalClientArea = window->getClientArea();
    if (!params.hasParent) {
      if (params.clientArea.x != Window::Builder::defaultPosition() && params.clientArea.x != Window::Builder::centeredPosition()) {
        EXPECT_EQ(params.clientArea.x + offsetX, finalClientArea.x);
      }
      if (params.clientArea.y != Window::Builder::defaultPosition() && params.clientArea.y != Window::Builder::centeredPosition()) {
        EXPECT_EQ(params.clientArea.y + offsetY, finalClientArea.y);
      }
    }
    if (params.clientArea.width != Window::Builder::defaultSize()) {
      EXPECT_EQ(params.clientArea.width, finalClientArea.width);
    }
    if (params.clientArea.height != Window::Builder::defaultSize()) {
      EXPECT_EQ(params.clientArea.height, finalClientArea.height);
    }
    
    // window close event
    _isCloseAllowed = false;
    Window::sendCloseEvent(window->handle());
    EXPECT_TRUE(window->pollCurrentWindowEvents());
    EXPECT_TRUE(Window::pollEvents());
    EXPECT_TRUE(window->handle() != (WindowHandle)0);
    
    _isCloseAllowed = true;
    Window::sendCloseEvent(window->handle());
    EXPECT_FALSE(window->pollCurrentWindowEvents());
    if (!params.hasParent && !isUpdate) {
      EXPECT_FALSE(Window::pollEvents());
      EXPECT_TRUE(window->handle() == (WindowHandle)0);
      window.reset();
      EXPECT_FALSE(Window::pollEvents());
    }
  }
  
  
  // -- window tests --

  TEST_F(WindowTest, contructionErrors) {
    Window::Builder builder;
    EXPECT_ANY_THROW(builder.create(nullptr, nullptr));
    EXPECT_ANY_THROW(builder.create(_STR(""), _STR("Test window")));
    EXPECT_FALSE(Window::pollEvents());
    
    auto window = builder.create(_STR("defaultWindow"), _STR("Test window"));
    EXPECT_TRUE(window != nullptr);
    window.reset();
    EXPECT_FALSE(Window::pollEvents());
  }
  
  TEST_F(WindowTest, testFullscreen) {
    WindowParams params;
    params.mode = WindowType::fullscreen;
    params.behavior = WindowBehavior::globalContext;
    params.resizeMode = ResizeMode::fixed;
    params.hasParent = false;
    params.scrollRangeX = 0;
    params.scrollRangeY = 0;
    params.clientArea = pandora::hardware::DisplayArea{ 0,0,Window::Builder::defaultSize(),Window::Builder::defaultSize() };
    
    Window::Builder builder;
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .setRefreshRate(60)
                         .setPosition(params.clientArea.x, params.clientArea.y)
                         .setSize(params.clientArea.width, params.clientArea.height)
                         .create(_STR("testFullscreen"), _STR("Test window"));
    _testWindow(window, params);
  }
  
  TEST_F(WindowTest, testWindowFixed) {
    WindowParams params;
    params.mode = WindowType::window;
    params.behavior = WindowBehavior::none;
    params.resizeMode = ResizeMode::fixed;
    params.hasParent = false;
    params.scrollRangeX = 0;
    params.scrollRangeY = 0;
    params.clientArea = pandora::hardware::DisplayArea{ 60,40,320,220 };
    
    Window::Builder builder;
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .setPosition(params.clientArea.x, params.clientArea.y)
                         .setSize(params.clientArea.width, params.clientArea.height)
                         .setIcon(WindowResource::buildIcon(SystemIcon::info), nullptr)
                         .setCursor(WindowResource::buildCursor(SystemCursor::wait))
                         .create(_STR("testWindowFixed"), _STR("Test window"));
    _testWindow(window, params);
  }
  
  TEST_F(WindowTest, testWindowResizableScrollable) {
    WindowParams params;
    params.mode = WindowType::window;
    params.behavior = (WindowBehavior::scrollH | WindowBehavior::scrollV);
    params.resizeMode = ResizeMode::resizable;
    params.hasParent = false;
    params.scrollRangeX = 3200;
    params.scrollRangeY = 2200;
    params.clientArea = pandora::hardware::DisplayArea{ 60,40,320,220 };
    
    Window::Builder builder;
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .setPosition(params.clientArea.x, params.clientArea.y)
                         .setSize(params.clientArea.width, params.clientArea.height)
                         .setIcon(WindowResource::buildIcon(SystemIcon::info), nullptr)
                         .setCursor(WindowResource::buildCursor(SystemCursor::wait))
                         .create(_STR("testWindowResizableScrollable"), _STR("Test window"));
    _testWindow(window, params);
  }

  TEST_F(WindowTest, testBorderless) {
    WindowParams params;
    params.mode = WindowType::borderless;
    params.behavior = (WindowBehavior::topMost | WindowBehavior::aboveTaskbar);
    params.resizeMode = ResizeMode::fixed;
    params.hasParent = false;
    params.scrollRangeX = 0;
    params.scrollRangeY = 0;
    params.clientArea = pandora::hardware::DisplayArea{ 0,0,Window::Builder::defaultSize(),Window::Builder::defaultSize() };

    Window::Builder builder;
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
      .setPosition(params.clientArea.x, params.clientArea.y)
      .setSize(params.clientArea.width, params.clientArea.height)
      .create(_STR("testBorderless"), _STR("Test window"));
    _testWindow(window, params);
  }

  TEST_F(WindowTest, testBordered) {
    WindowParams params;
    params.mode = WindowType::bordered;
    params.behavior = WindowBehavior::topMost;
    params.resizeMode = ResizeMode::resizable;
    params.hasParent = false;
    params.scrollRangeX = 0;
    params.scrollRangeY = 0;
    params.clientArea = pandora::hardware::DisplayArea{ Window::Builder::centeredPosition(),Window::Builder::centeredPosition(),300,200 };

    Window::Builder builder;
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
      .setPosition(params.clientArea.x, params.clientArea.y)
      .setSize(params.clientArea.width, params.clientArea.height)
      .create(_STR("testBordered"), _STR("Test window"));
    _testWindow(window, params);
  }
  
  TEST_F(WindowTest, testDialog) {
    WindowParams params;
    params.mode = WindowType::dialog;
    params.behavior = (WindowBehavior::topMost | WindowBehavior::dropShadow);
    params.resizeMode = ResizeMode::fixed;
    params.hasParent = false;
    params.scrollRangeX = 0;
    params.scrollRangeY = 0;
    params.clientArea = pandora::hardware::DisplayArea{ Window::Builder::centeredPosition(),Window::Builder::centeredPosition(),300,200 };
    
    Window::Builder builder;
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .setPosition(params.clientArea.x, params.clientArea.y)
                         .setSize(params.clientArea.width, params.clientArea.height)
                         .create(_STR("testDialog"), _STR("Test window"));
    _testWindow(window, params);
  }
  
  TEST_F(WindowTest, testDialogWithParent) {
    WindowParams params;
    params.mode = WindowType::window;
    params.behavior = WindowBehavior::none;
    params.resizeMode = ResizeMode::resizableX;
    Window::Builder builder;
    auto parent = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .create(_STR("testDialogWithParent_parent"), _STR("Test parent"));
    ASSERT_TRUE(parent != nullptr);
    parent->show();
    
    params.mode = WindowType::dialog;
    params.behavior = WindowBehavior::topMost;
    params.resizeMode = ResizeMode::fixed;
    params.hasParent = true;
    params.scrollRangeX = 0;
    params.scrollRangeY = 0;
    params.clientArea = pandora::hardware::DisplayArea{ Window::Builder::defaultPosition(),Window::Builder::defaultPosition(),
                                                        Window::Builder::defaultSize(),Window::Builder::defaultSize() };
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .setPosition(params.clientArea.x, params.clientArea.y)
                         .setSize(params.clientArea.width, params.clientArea.height)
                         .create(_STR("testDialogWithParent"), _STR("Test window"), parent->handle());
    _testWindow(window, params);
    parent.reset();
    EXPECT_FALSE(Window::pollEvents());
  }
  
  TEST_F(WindowTest, testUpdatedWindow) {
    WindowParams params;
    params.mode = WindowType::window;
    params.behavior = WindowBehavior::none;
    params.resizeMode = ResizeMode::fixed;
    Window::Builder builder;
    auto owner = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .create(_STR("testUpdatedWindow"), _STR("Test window"));
    ASSERT_TRUE(owner != nullptr);
    owner->show();
    
    params.mode = WindowType::dialog;
    params.behavior = (WindowBehavior::scrollV | WindowBehavior::scrollH);
    params.resizeMode = (ResizeMode::resizable | ResizeMode::homothety);
    params.hasParent = false;
    params.scrollRangeX = 1000;
    params.scrollRangeY = 2000;
    params.clientArea = pandora::hardware::DisplayArea{ Window::Builder::defaultPosition(),Window::Builder::centeredPosition(),300,200 };
    
    auto window = builder.setDisplayMode(params.mode, params.behavior, params.resizeMode)
                         .setPosition(params.clientArea.x, params.clientArea.y)
                         .setSize(params.clientArea.width, params.clientArea.height)
                         .setIcon(WindowResource::buildIcon(SystemIcon::info), nullptr)
                         .setCursor(WindowResource::buildCursor(SystemCursor::wait))
                         .update(owner->handle());
    _testWindow(window, params, true);
    owner.reset();
    EXPECT_FALSE(Window::pollEvents());
  }

#else
  TEST_F(WindowTest, windowNotImplemented) {
    Window::Builder builder;
    EXPECT_ANY_THROW(builder.create(_STR("WINDOW0"), _STR("")));
  }
#endif
