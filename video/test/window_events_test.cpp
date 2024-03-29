/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#include <gtest/gtest.h>
#include <cstddef>
#include <cstring>
#include <video/window_events.h>
#include <video/window_keycodes.h>

using namespace pandora::video;

class WindowEventsTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- event handler data --

TEST_F(WindowEventsTest, verifyKeyCodeValues) {
  // command keys
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_UNKNOWN));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_L_ALT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_R_ALT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_L_CTRL));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_R_CTRL));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_L_SHIFT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_R_SHIFT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_L_SYSTEM));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_R_SYSTEM));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_CAPS_LOCK));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_NUM_LOCK));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_SCROLL_LOCK));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_APP_MENU));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_CLEAR));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_EXECUTE));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_HELP));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PAUSE));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PRINT_SCR));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_SELECT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_DELETE));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_END));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_HOME));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_INSERT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PG_DOWN));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PG_UP));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_ARROW_CENTER));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_ARROW_DOWN));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_ARROW_LEFT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_ARROW_RIGHT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_ARROW_UP));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F1));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F2));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F3));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F4));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F5));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F6));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F7));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F8));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F9));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F10));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F11));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F12));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F13));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F14));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F15));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F16));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F17));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F18));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F19));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F20));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F21));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F22));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F23));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_F24));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_MEDIA_NEXT));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_MEDIA_PREV));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_MEDIA_PLAY));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_MEDIA_STOP));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_VOLUME_DOWN));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_VOLUME_MUTE));
  EXPECT_EQ((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_VOLUME_UP));

# ifdef _WINDOWS
    // command keys with text equivalent
    auto val = virtualKeyToChar(_P_VK_ENTER);
    EXPECT_TRUE(val == L'\r' || val == L'\n');
    val = virtualKeyToChar(_P_VK_ENTER_PAD);
    EXPECT_TRUE(val == L'\r' || val == L'\n');
    EXPECT_EQ(L' ', virtualKeyToChar(_P_VK_SPACE));
    EXPECT_EQ(L'\t', virtualKeyToChar(_P_VK_TAB));
    EXPECT_EQ(L'\b', virtualKeyToChar(_P_VK_BACKSPACE));

    EXPECT_EQ((uint32_t)3, (uint32_t)virtualKeyToChar(_P_VK_INTERRUPT));
    val = virtualKeyToChar(_P_VK_ESC);
    EXPECT_TRUE(val == L'\0' || (uint32_t)val == (uint32_t)27);

    // numpad keys
    for (uint32_t code = _P_VK_NUMPAD_0; code <= _P_VK_NUMPAD_9; ++code) // OK because contiguous values on Windows
      EXPECT_EQ(L'0' + (wchar_t)(code - _P_VK_NUMPAD_0), virtualKeyToChar(code));
    EXPECT_EQ(L'.', virtualKeyToChar(_P_VK_DECIMAL));
    val = virtualKeyToChar(_P_VK_SEPARATOR);
    EXPECT_TRUE(val == L'\0' || val == L',' || val == L'.');
    EXPECT_EQ(L'+', virtualKeyToChar(_P_VK_ADD));
    EXPECT_EQ(L'/', virtualKeyToChar(_P_VK_DIVIDE));
    EXPECT_EQ(L'*', virtualKeyToChar(_P_VK_MULTIPLY));
    EXPECT_EQ(L'-', virtualKeyToChar(_P_VK_SUBTRACT));

    // standard keys
    for (uint32_t code = _P_VK_0; code <= _P_VK_9; ++code) // OK because contiguous values on Windows
      EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(code)); // not necessarily a letter -> depends if digits are lowercase/uppercase
    for (uint32_t code = _P_VK_A; code <= _P_VK_Z; ++code) // OK because contiguous values on Windows
      EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(code)); // not necessarily a letter -> depends on keyboard layout
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PUNCT_BRACK1));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PUNCT_BRACK2));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PUNCT_MINUS));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PUNCT_COMMA));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PUNCT_PERIOD));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_PUNCT_PLUS));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_GRAVE_EXP));
    EXPECT_NE((uint32_t)0, (uint32_t)virtualKeyToChar(_P_VK_BACKSLASH));
    virtualKeyToChar(_P_VK_PUNCT_MISC1);
    virtualKeyToChar(_P_VK_PUNCT_MISC2);
    virtualKeyToChar(_P_VK_PUNCT_MISC3);
    virtualKeyToChar(_P_VK_PUNCT_MISC4);
    virtualKeyToChar(_P_VK_PUNCT_MISC5);
    virtualKeyToChar(_P_VK_PUNCT_MISC6);

# else
    EXPECT_EQ((uint32_t)0, virtualKeyToChar(0)); // not yet implemented
# endif
}

TEST_F(WindowEventsTest, mouseAccessors) {
  EXPECT_NE(0, mouseWheelDelta());
  EXPECT_FALSE(isMouseButtonPressed(0, MouseButton::left));
  EXPECT_FALSE(isMouseButtonPressed(0, MouseButton::middle));
  EXPECT_FALSE(isMouseButtonPressed(0, MouseButton::right));
  EXPECT_FALSE(isMouseButtonPressed(0, MouseButton::button4));
  EXPECT_FALSE(isMouseButtonPressed(0, MouseButton::button5));
  EXPECT_TRUE(isMouseButtonPressed(0xFF, MouseButton::left));
  EXPECT_TRUE(isMouseButtonPressed(0xFF, MouseButton::middle));
  EXPECT_TRUE(isMouseButtonPressed(0xFF, MouseButton::right));
  EXPECT_TRUE(isMouseButtonPressed(0xFF, MouseButton::button4));
  EXPECT_TRUE(isMouseButtonPressed(0xFF, MouseButton::button5));
}
