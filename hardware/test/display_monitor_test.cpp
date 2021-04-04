#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <hardware/display_monitor.h>
#ifdef _MSC_VER
# include <Windows.h>
#endif

using namespace pandora::hardware;

class DisplayMonitorTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- data structures --

TEST_F(DisplayMonitorTest, dataStructures) { 
  DisplayArea area1{ 0, 10, 800u, 600u };
  DisplayArea area2{ 0, 10, 800u, 600u };
  DisplayArea area3{ 10, 0, 600u, 800u };
  EXPECT_EQ(area1.x, area2.x);
  EXPECT_EQ(area1.y, area2.y);
  EXPECT_EQ(area1.width, area2.width);
  EXPECT_EQ(area1.height, area2.height);
  EXPECT_NE(area1.x, area3.x);
  EXPECT_NE(area1.y, area3.y);
  EXPECT_NE(area1.width, area3.width);
  EXPECT_NE(area1.height, area3.height);

  DisplayMode mode1{ 800u, 600u, 32u, 60u };
  DisplayMode mode2{ 800u, 600u, 32u, 60u };
  DisplayMode mode3{ 600u, 800u, 24u, undefinedRefreshRate() };
  EXPECT_EQ(mode1.width, mode2.width);
  EXPECT_EQ(mode1.height, mode2.height);
  EXPECT_EQ(mode1.bitDepth, mode2.bitDepth);
  EXPECT_EQ(mode1.refreshRate, mode2.refreshRate);
  EXPECT_NE(mode1.width, mode3.width);
  EXPECT_NE(mode1.height, mode3.height);
  EXPECT_NE(mode1.bitDepth, mode3.bitDepth);
  EXPECT_NE(mode1.refreshRate, mode3.refreshRate);
}


// -- display monitor --

TEST_F(DisplayMonitorTest, readPrimaryMonitorAreaAndDpi) { 
  DisplayMonitor::setDpiAwareness(true);

  DisplayMonitor monitor;
  // verify handle in tests: when executed on CI (without any monitor), the handle will be empty (and sizes will be 0)
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().id.empty() : monitor.attributes().id.empty());
  EXPECT_TRUE((monitor.handle()) ? (monitor.attributes().screenArea.height > 0) : (monitor.attributes().screenArea.height == 0));
  EXPECT_TRUE((monitor.handle()) ? (monitor.attributes().screenArea.width > 0) : (monitor.attributes().screenArea.width == 0));
  EXPECT_TRUE((monitor.handle()) ? (monitor.attributes().workArea.height > 0) : (monitor.attributes().workArea.height == 0));
  EXPECT_TRUE((monitor.handle()) ? (monitor.attributes().workArea.width > 0) : (monitor.attributes().workArea.width == 0));
  EXPECT_TRUE(monitor.attributes().isPrimary);

  uint32_t dpiX = 0, dpiY = 0;
  float scalingX = 0.0f, scalingY = 0.0f;
  monitor.getMonitorDpi(dpiX, dpiY);
  EXPECT_TRUE(dpiX > 0);
  EXPECT_TRUE(dpiY > 0);
  monitor.getMonitorScaling(scalingX, scalingY);
  EXPECT_TRUE(scalingX >= 1.0f);
  EXPECT_TRUE(scalingY >= 1.0f);
# ifdef _MSC_VER
    monitor.getMonitorDpi(dpiX, dpiY, GetConsoleWindow());
    EXPECT_TRUE(dpiX > 0);
    EXPECT_TRUE(dpiY > 0);
    monitor.getMonitorScaling(scalingX, scalingY, GetConsoleWindow());
    EXPECT_TRUE(scalingX >= 1.0f);
    EXPECT_TRUE(scalingY >= 1.0f);
# endif

  DisplayArea clientArea{ 30, 30, 600u, 480u };
  DisplayArea windowArea = monitor.convertClientAreaToWindowArea(clientArea, (DisplayMonitor::WindowHandle)0, false);
  EXPECT_TRUE(windowArea.x <= clientArea.x);
  EXPECT_TRUE(windowArea.y <= clientArea.y);
  EXPECT_TRUE(windowArea.width >= clientArea.width);
  EXPECT_TRUE(windowArea.height >= clientArea.height);
# ifdef _MSC_VER
    windowArea = monitor.convertClientAreaToWindowArea(clientArea, GetConsoleWindow(), false);
    EXPECT_TRUE(windowArea.x <= clientArea.x);
    EXPECT_TRUE(windowArea.y <= clientArea.y);
    EXPECT_TRUE(windowArea.width >= clientArea.width);
    EXPECT_TRUE(windowArea.height >= clientArea.height);
# endif
}

TEST_F(DisplayMonitorTest, listMonitors) {
  DisplayMonitor::setDpiAwareness(true);

  DisplayMonitor monitor;
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().id.empty() : monitor.attributes().id.empty());
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().description.empty() : monitor.attributes().description.empty());
  EXPECT_TRUE(monitor.attributes().isPrimary);

  auto monitors = DisplayMonitor::listAvailableMonitors();
  EXPECT_TRUE((monitor.handle()) ? !monitors.empty() : true);
  if (!monitors.empty() && monitor.handle()) {
    bool isFound = false;
    for (auto& it : monitors) {
      if (it.handle() == monitor.handle()) {
        isFound = true;
        break;
      }
    }
    EXPECT_TRUE(isFound);
  }
}

TEST_F(DisplayMonitorTest, getSetPrimaryMonitorDisplayModes) { 
  DisplayMonitor::setDpiAwareness(true);

  DisplayMonitor monitor;
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().id.empty() : monitor.attributes().id.empty());
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().description.empty() : monitor.attributes().description.empty());
  EXPECT_TRUE(monitor.attributes().isPrimary);
  if (!monitor.handle()) { // for CI with virtual monitor
    auto monitors = DisplayMonitor::listAvailableMonitors();
    if (!monitors.empty())
      monitor = std::move(monitors[0]);
  }

  auto modes = monitor.listAvailableDisplayModes();
  EXPECT_TRUE((monitor.handle()) ? modes.size() >= size_t{ 1u } : modes.empty());
  
  DisplayMode mode = monitor.getDisplayMode();
  EXPECT_TRUE((monitor.handle()) ? (mode.width > 0) : (mode.width == 0));
  EXPECT_TRUE((monitor.handle()) ? (mode.height > 0) : (mode.height == 0));
  EXPECT_TRUE((monitor.handle()) ? (mode.refreshRate != undefinedRefreshRate()) : (mode.refreshRate == undefinedRefreshRate()));

  bool isFound = false;
  for (auto& it : modes) {
    if (it.width == mode.width && it.height == mode.height && it.bitDepth == mode.bitDepth && it.refreshRate == mode.refreshRate) {
      isFound = true;
      break;
    }
  }
  EXPECT_TRUE(isFound || !monitor.handle());
  
  monitor.setDefaultDisplayMode();
  monitor.setDisplayMode(mode);
  DisplayMode mode2 = monitor.getDisplayMode();
  EXPECT_EQ(mode.width, mode2.width);
  EXPECT_EQ(mode.height, mode2.height);
  EXPECT_EQ(mode.bitDepth, mode2.bitDepth);
  EXPECT_EQ(mode.refreshRate, mode2.refreshRate);
}


// -- other constructors --

TEST_F(DisplayMonitorTest, moveInstance) { 
  DisplayMonitor monitor;
  auto handle = monitor.handle();
  DisplayMonitor::Attributes attr = monitor.attributes();

  DisplayMonitor moved(std::move(monitor));
  EXPECT_EQ(handle, moved.handle());
  EXPECT_EQ(attr.id, moved.attributes().id);
  EXPECT_EQ(attr.description, moved.attributes().description);
  EXPECT_EQ(attr.adapter, moved.attributes().adapter);
  EXPECT_EQ(0, memcmp((void*)&(attr.screenArea), (void*)&(moved.attributes().screenArea), sizeof(DisplayArea)));
  EXPECT_EQ(0, memcmp((void*)&(attr.workArea), (void*)&(moved.attributes().workArea), sizeof(DisplayArea)));
  EXPECT_EQ(attr.isPrimary, moved.attributes().isPrimary);

  monitor = std::move(moved);
  EXPECT_EQ(handle, monitor.handle());
  EXPECT_EQ(attr.id, monitor.attributes().id);
  EXPECT_EQ(attr.description, monitor.attributes().description);
  EXPECT_EQ(attr.adapter, monitor.attributes().adapter);
  EXPECT_EQ(0, memcmp((void*)&(attr.screenArea), (void*)&(monitor.attributes().screenArea), sizeof(DisplayArea)));
  EXPECT_EQ(0, memcmp((void*)&(attr.workArea), (void*)&(monitor.attributes().workArea), sizeof(DisplayArea)));
  EXPECT_EQ(attr.isPrimary, monitor.attributes().isPrimary);
}

TEST_F(DisplayMonitorTest, monitorByHandle) {
  EXPECT_THROW(DisplayMonitor((DisplayMonitor::Handle)0, false), std::invalid_argument);

  DisplayMonitor defaultMonitor;
  DisplayMonitor invalidUseDefault((DisplayMonitor::Handle)0, true);
  EXPECT_EQ(defaultMonitor.handle(), invalidUseDefault.handle());
  EXPECT_EQ(defaultMonitor.attributes().id, invalidUseDefault.attributes().id);
  EXPECT_EQ(defaultMonitor.attributes().isPrimary, invalidUseDefault.attributes().isPrimary);

  if (!defaultMonitor.handle()) { // for CI with virtual monitor
    auto monitors = DisplayMonitor::listAvailableMonitors();
    if (!monitors.empty())
      defaultMonitor = std::move(monitors[0]);
  }
  if (defaultMonitor.handle()) { // if a monitor exists
    EXPECT_EQ(defaultMonitor.handle(), DisplayMonitor(defaultMonitor.handle(), false).handle()); // would throw if invalid handle
  }
}

TEST_F(DisplayMonitorTest, monitorById) {
# ifdef _WINDOWS
    std::wstring invalidId = L"--DUMMY_INVALID_ID!!!--";
# else
    std::string invalidId = "--DUMMY_INVALID_ID!!!--";
# endif

  EXPECT_THROW(DisplayMonitor(invalidId, false), std::invalid_argument);

  DisplayMonitor defaultMonitor;
  DisplayMonitor invalidUseDefault(invalidId, true);
  EXPECT_EQ(defaultMonitor.handle(), invalidUseDefault.handle());
  EXPECT_EQ(defaultMonitor.attributes().id, invalidUseDefault.attributes().id);
  EXPECT_EQ(defaultMonitor.attributes().isPrimary, invalidUseDefault.attributes().isPrimary);

  if (!defaultMonitor.handle()) { // for CI with virtual monitor
    auto monitors = DisplayMonitor::listAvailableMonitors();
    if (!monitors.empty())
      defaultMonitor = std::move(monitors[0]);
  }
  if (defaultMonitor.handle() && !defaultMonitor.attributes().id.empty()) { // if a monitor exists
    EXPECT_EQ(defaultMonitor.handle(), DisplayMonitor(defaultMonitor.attributes().id, false).handle()); // would throw if invalid id
  }
}

TEST_F(DisplayMonitorTest, monitorByIndex) {
  EXPECT_THROW(DisplayMonitor(false, 999999999u), std::invalid_argument);

  DisplayMonitor defaultMonitor;
  DisplayMonitor invalidUseDefault(true, 999999999u);
  EXPECT_EQ(defaultMonitor.handle(), invalidUseDefault.handle());
  EXPECT_EQ(defaultMonitor.attributes().id, invalidUseDefault.attributes().id);
  EXPECT_EQ(defaultMonitor.attributes().isPrimary, invalidUseDefault.attributes().isPrimary);

  if (!defaultMonitor.handle()) { // for CI with virtual monitor
    auto monitors = DisplayMonitor::listAvailableMonitors();
    if (!monitors.empty())
      defaultMonitor = std::move(monitors[0]);
  }
  if (defaultMonitor.handle()) { // if a monitor exists
    DisplayMonitor target(false, 0u); // would throw if invalid index

    auto list = DisplayMonitor::listAvailableMonitors();
    bool isFound = false;
    for (auto& it : list) {
      if (it.handle() == target.handle()) {
        isFound = true;
        break;
      }
    }
    EXPECT_TRUE(isFound);
  }
}
