#include <gtest/gtest.h>
#include <video/screensaver.h>

using namespace pandora::video;

class ScreenSaverTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- disable/restore screensaver --

TEST_F(ScreenSaverTest, disableRestoreScreenSaverTest) {
# ifndef _P_ENABLE_LINUX_WAYLAND
    EXPECT_TRUE(disableScreenSaver());
    EXPECT_TRUE(restoreScreenSaver());
    
    notifyScreenActivity();
    EXPECT_TRUE(disableScreenSaver());
    EXPECT_TRUE(restoreScreenSaver());
# else
    //... window surfaces not yet implemented for wayland
# endif
}
