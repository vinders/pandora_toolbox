/*******************************************************************************
Main execution of test cases
*******************************************************************************/
#include "gtest/gtest.h"

//#define __P_TEST_MESSAGEBOX 1
#ifdef __P_TEST_MESSAGEBOX
# include "video/message_box.h"
  using pandora::video::MessageBox;
#endif

int main(int argc, char** argv) {
# ifdef __P_TEST_MESSAGEBOX
    auto res1 = MessageBox::show("Title @test !", "This is just a test message\nAnd another line!", MessageBox::ActionType::okCancel, MessageBox::IconType::none, true);
    printf("action: %d\n", (uint32_t)res1);
    auto res2 = MessageBox::show("Question", "Custom button labels x3", "Save", "Ignore", "Return", MessageBox::IconType::question, false);
    printf("action: %d\n", (uint32_t)res2);
    auto res3 = MessageBox::show("Error", "Custom button labels x2", "Continue", "Abort", nullptr, MessageBox::IconType::error, false);
    printf("action: %d\n", (uint32_t)res3);
    auto res4 = MessageBox::show("Warning", "This is the last message...", MessageBox::ActionType::ok, MessageBox::IconType::warning, false);
    printf("action: %d\n", (uint32_t)res4);
# endif

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
