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
  EXPECT_EQ(0, memcmp((void*)&area1, (void*)&area2, sizeof(DisplayArea)));
  EXPECT_NE(0, memcmp((void*)&area1, (void*)&area3, sizeof(DisplayArea)));

  DisplayMode mode1{ 800u, 600u, 32u, 60u };
  DisplayMode mode2{ 800u, 600u, 32u, 60u };
  DisplayMode mode3{ 600u, 800u, 24u, undefinedRefreshRate() };
  EXPECT_EQ(0, memcmp((void*)&mode1, (void*)&mode2, sizeof(DisplayMode)));
  EXPECT_NE(0, memcmp((void*)&mode1, (void*)&mode3, sizeof(DisplayMode)));
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

TEST_F(DisplayMonitorTest, getSetPrimaryMonitorDisplayModes) { 
  DisplayMonitor::setDpiAwareness(true);

  DisplayMonitor monitor;
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().id.empty() : monitor.attributes().id.empty());
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().description.empty() : monitor.attributes().description.empty());
  EXPECT_TRUE((monitor.handle()) ? !monitor.attributes().adapter.empty() : monitor.attributes().adapter.empty());
  EXPECT_TRUE(monitor.attributes().isPrimary);

  auto modes = monitor.listAvailableDisplayModes();
  EXPECT_TRUE(modes.size() >= size_t{ 1u });
  
  DisplayMode mode = monitor.getDisplayMode();
  EXPECT_TRUE((monitor.handle()) ? (mode.width > 0) : (mode.width == 0));
  EXPECT_TRUE((monitor.handle()) ? (mode.height > 0) : (mode.height == 0));
  EXPECT_TRUE((monitor.handle()) ? (mode.refreshRate != undefinedRefreshRate()) : (mode.refreshRate == undefinedRefreshRate()));

  bool isFound = false;
  for (auto& it : modes) {
    if (it.width == mode.width && it.height == mode.height && it.bitDepth == mode.bitDepth && it.refreshRate == mode.refreshRate) {
      isFound = true;
    }
  }
  EXPECT_TRUE(isFound);
  
  monitor.setDefaultDisplayMode();
  monitor.setDisplayMode(mode);
  DisplayMode mode2 = monitor.getDisplayMode();
  EXPECT_EQ(mode.width, mode2.width);
  EXPECT_EQ(mode.height, mode2.height);
  EXPECT_EQ(mode.bitDepth, mode2.bitDepth);
  EXPECT_EQ(mode.refreshRate, mode2.refreshRate);
}

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
