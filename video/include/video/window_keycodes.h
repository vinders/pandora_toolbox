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
--------------------------------------------------------------------------------
constants : mappings to virtual keycodes of all systems
warning : this file includes system libraries
          -> only include it in the source file that manages input keys!
          -> do not include it in header files
          -> on iOS, include it in .m or .mm files
*******************************************************************************/
#pragma once
#define __P_VK_UNK(index) (0xFF00 + index)


// -- WINDOWS -- ---------------------------------------------------------------
#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# ifdef MessageBox
#   undef MessageBox
# endif
# ifdef min
#   undef min
#   undef max
# endif

// -- virtual key codes - system --

# define _P_VK_UNKNOWN    0xFF
# define _P_VK_INTERRUPT  VK_CANCEL // CTRL+C on Windows

// -- virtual key codes - modifier keys --

# define _P_VK_L_ALT     VK_LMENU
# define _P_VK_R_ALT     VK_RMENU
# define _P_VK_L_CTRL    VK_LCONTROL
# define _P_VK_R_CTRL    VK_RCONTROL
# define _P_VK_L_SHIFT   VK_LSHIFT
# define _P_VK_R_SHIFT   VK_RSHIFT
# define _P_VK_L_SYSTEM  VK_LWIN
# define _P_VK_R_SYSTEM  VK_RWIN

# define _P_VK_CAPS_LOCK    VK_CAPITAL
# define _P_VK_NUM_LOCK     VK_NUMLOCK
# define _P_VK_SCROLL_LOCK  VK_SCROLL

// -- virtual key codes - command keys --

# define _P_VK_APP_MENU   VK_APPS
# define _P_VK_BACKSPACE  VK_BACK
# define _P_VK_CLEAR      VK_CLEAR
# define _P_VK_ENTER      VK_RETURN
# define _P_VK_ENTER_PAD  0x100 // returned by impl if "extended" VK_RETURN detected
# define _P_VK_EXECUTE    VK_EXECUTE
# define _P_VK_ESC        VK_ESCAPE
# define _P_VK_HELP       VK_HELP
# define _P_VK_PAUSE      VK_PAUSE
# define _P_VK_PRINT_SCR  VK_SNAPSHOT
# define _P_VK_SELECT     VK_SELECT // selection in menu
# define _P_VK_SPACE      VK_SPACE
# define _P_VK_TAB        VK_TAB

# define _P_VK_DELETE     VK_DELETE
# define _P_VK_END        VK_END
# define _P_VK_HOME       VK_HOME
# define _P_VK_INSERT     VK_INSERT
# define _P_VK_PG_DOWN    VK_NEXT
# define _P_VK_PG_UP      VK_PRIOR

# define _P_VK_ARROW_CENTER  __P_VK_UNK(2)
# define _P_VK_ARROW_DOWN    VK_DOWN
# define _P_VK_ARROW_LEFT    VK_LEFT
# define _P_VK_ARROW_RIGHT   VK_RIGHT
# define _P_VK_ARROW_UP      VK_UP

# define _P_VK_F1   VK_F1
# define _P_VK_F2   VK_F2
# define _P_VK_F3   VK_F3
# define _P_VK_F4   VK_F4
# define _P_VK_F5   VK_F5
# define _P_VK_F6   VK_F6
# define _P_VK_F7   VK_F7
# define _P_VK_F8   VK_F8
# define _P_VK_F9   VK_F9
# define _P_VK_F10  VK_F10
# define _P_VK_F11  VK_F11
# define _P_VK_F12  VK_F12
# define _P_VK_F13  VK_F13
# define _P_VK_F14  VK_F14
# define _P_VK_F15  VK_F15
# define _P_VK_F16  VK_F16
# define _P_VK_F17  VK_F17
# define _P_VK_F18  VK_F18
# define _P_VK_F19  VK_F19
# define _P_VK_F20  VK_F20
# define _P_VK_F21  VK_F21
# define _P_VK_F22  VK_F22
# define _P_VK_F23  VK_F23
# define _P_VK_F24  VK_F24

// -- virtual key codes - number keys --

# define _P_VK_0  0x30
# define _P_VK_1  (_P_VK_0 + 1)
# define _P_VK_2  (_P_VK_0 + 2)
# define _P_VK_3  (_P_VK_0 + 3)
# define _P_VK_4  (_P_VK_0 + 4)
# define _P_VK_5  (_P_VK_0 + 5)
# define _P_VK_6  (_P_VK_0 + 6)
# define _P_VK_7  (_P_VK_0 + 7)
# define _P_VK_8  (_P_VK_0 + 8)
# define _P_VK_9  (_P_VK_0 + 9)

# define _P_VK_NUMPAD_0  VK_NUMPAD0
# define _P_VK_NUMPAD_1  VK_NUMPAD1
# define _P_VK_NUMPAD_2  VK_NUMPAD2
# define _P_VK_NUMPAD_3  VK_NUMPAD3
# define _P_VK_NUMPAD_4  VK_NUMPAD4
# define _P_VK_NUMPAD_5  VK_NUMPAD5
# define _P_VK_NUMPAD_6  VK_NUMPAD6
# define _P_VK_NUMPAD_7  VK_NUMPAD7
# define _P_VK_NUMPAD_8  VK_NUMPAD8
# define _P_VK_NUMPAD_9  VK_NUMPAD9

# define _P_VK_DECIMAL    VK_DECIMAL   // decimal dot
# define _P_VK_SEPARATOR  VK_SEPARATOR // decimal comma

# define _P_VK_ADD       VK_ADD
# define _P_VK_DIVIDE    VK_DIVIDE
# define _P_VK_MULTIPLY  VK_MULTIPLY
# define _P_VK_SUBTRACT  VK_SUBTRACT

// -- virtual key codes - character keys --

# define _P_VK_A  0x41
# define _P_VK_B  (_P_VK_A + 1)
# define _P_VK_C  (_P_VK_A + 2)
# define _P_VK_D  (_P_VK_A + 3)
# define _P_VK_E  (_P_VK_A + 4)
# define _P_VK_F  (_P_VK_A + 5)
# define _P_VK_G  (_P_VK_A + 6)
# define _P_VK_H  (_P_VK_A + 7)
# define _P_VK_I  (_P_VK_A + 8)
# define _P_VK_J  (_P_VK_A + 9)
# define _P_VK_K  (_P_VK_A + 10)
# define _P_VK_L  (_P_VK_A + 11)
# define _P_VK_M  (_P_VK_A + 12)
# define _P_VK_N  (_P_VK_A + 13)
# define _P_VK_O  (_P_VK_A + 14)
# define _P_VK_P  (_P_VK_A + 15)
# define _P_VK_Q  (_P_VK_A + 16)
# define _P_VK_R  (_P_VK_A + 17)
# define _P_VK_S  (_P_VK_A + 18)
# define _P_VK_T  (_P_VK_A + 19)
# define _P_VK_U  (_P_VK_A + 20)
# define _P_VK_V  (_P_VK_A + 21)
# define _P_VK_W  (_P_VK_A + 22)
# define _P_VK_X  (_P_VK_A + 23)
# define _P_VK_Y  (_P_VK_A + 24)
# define _P_VK_Z  (_P_VK_A + 25)

# define _P_VK_PUNCT_BRACK1  VK_OEM_4 // us: [{  azerty: ^¨[  others: variable
# define _P_VK_PUNCT_BRACK2  VK_OEM_6 // us: ]}  azerty: $*]  others: variable
# define _P_VK_PUNCT_COMMA   VK_OEM_COMMA
# define _P_VK_PUNCT_PERIOD  VK_OEM_PERIOD
# define _P_VK_PUNCT_MINUS   VK_OEM_MINUS
# define _P_VK_PUNCT_PLUS    VK_OEM_PLUS
# define _P_VK_PUNCT_MISC1   VK_OEM_1 // us: ;:  others: variable
# define _P_VK_PUNCT_MISC2   VK_OEM_2 // us: /?  others: variable
# define _P_VK_PUNCT_MISC3   VK_OEM_7 // us: '"  others: variable
# define _P_VK_PUNCT_MISC4   VK_OEM_8 // variable
# define _P_VK_PUNCT_MISC5   __P_VK_UNK(10)
# define _P_VK_PUNCT_MISC6   __P_VK_UNK(11)
# define _P_VK_GRAVE_EXP     VK_OEM_3 // us: `~  azerty: ²³   others: variable
# define _P_VK_BACKSLASH     VK_OEM_5 // us: backslash|  azerty: <>backslash

// -- virtual key codes - multi-media keys --

# define _P_VK_MEDIA_NEXT   VK_MEDIA_NEXT_TRACK
# define _P_VK_MEDIA_PREV   VK_MEDIA_PREV_TRACK
# define _P_VK_MEDIA_PLAY   VK_MEDIA_PLAY_PAUSE
# define _P_VK_MEDIA_STOP   VK_MEDIA_STOP
# define _P_VK_VOLUME_DOWN  VK_VOLUME_DOWN
# define _P_VK_VOLUME_MUTE  VK_VOLUME_MUTE
# define _P_VK_VOLUME_UP    VK_VOLUME_UP


// _____________________________________________________________________________

// -- MAC OS -- ----------------------------------------------------------------
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
#   include <Carbon/Carbon.h>
//#   include <HIToolbox/Events.h>

// -- virtual key codes - system --

#   define _P_VK_UNKNOWN    0xFFFF
#   define _P_VK_INTERRUPT  __P_VK_UNK(0)

// -- virtual key codes - modifier keys --

#   define _P_VK_L_ALT     kVK_Option
#   define _P_VK_R_ALT     kVK_RightOption
#   define _P_VK_L_CTRL    kVK_Control
#   define _P_VK_R_CTRL    kVK_RightControl
#   define _P_VK_L_SHIFT   kVK_Shift
#   define _P_VK_R_SHIFT   kVK_RightShift
#   define _P_VK_L_SYSTEM  kVK_Command
#   define _P_VK_R_SYSTEM  kVK_RightCommand

#   define _P_VK_CAPS_LOCK    kVK_CapsLock
#   define _P_VK_NUM_LOCK     __P_VK_UNK(1)
#   define _P_VK_SCROLL_LOCK  __P_VK_UNK(2)

// -- virtual key codes - command keys --

#   define _P_VK_APP_MENU   __P_VK_UNK(14)
#   define _P_VK_BACKSPACE  kVK_Delete
#   define _P_VK_CLEAR      kVK_ANSI_KeypadClear
#   define _P_VK_ENTER      kVK_Return
#   define _P_VK_ENTER_PAD  kVK_ANSI_KeypadEnter
#   define _P_VK_EXECUTE    __P_VK_UNK(3)
#   define _P_VK_ESC        kVK_Escape
#   define _P_VK_HELP       kVK_Help
#   define _P_VK_PAUSE      __P_VK_UNK(4)
#   define _P_VK_PRINT_SCR  __P_VK_UNK(5)
#   define _P_VK_SELECT     __P_VK_UNK(6)
#   define _P_VK_SPACE      kVK_Space
#   define _P_VK_TAB        kVK_Tab

#   define _P_VK_DELETE     kVK_ForwardDelete
#   define _P_VK_END        kVK_End
#   define _P_VK_HOME       kVK_Home
#   define _P_VK_INSERT     __P_VK_UNK(7)
#   define _P_VK_PG_DOWN    kVK_PageDown
#   define _P_VK_PG_UP      kVK_PageUp

#   define _P_VK_ARROW_CENTER  __P_VK_UNK(8)
#   define _P_VK_ARROW_DOWN    kVK_DownArrow
#   define _P_VK_ARROW_LEFT    kVK_LeftArrow
#   define _P_VK_ARROW_RIGHT   kVK_RightArrow
#   define _P_VK_ARROW_UP      kVK_UpArrow

#   define _P_VK_F1   kVK_F1
#   define _P_VK_F2   kVK_F2
#   define _P_VK_F3   kVK_F3
#   define _P_VK_F4   kVK_F4
#   define _P_VK_F5   kVK_F5
#   define _P_VK_F6   kVK_F6
#   define _P_VK_F7   kVK_F7
#   define _P_VK_F8   kVK_F8
#   define _P_VK_F9   kVK_F9
#   define _P_VK_F10  kVK_F10
#   define _P_VK_F11  kVK_F11
#   define _P_VK_F12  kVK_F12
#   define _P_VK_F13  kVK_F13
#   define _P_VK_F14  kVK_F14
#   define _P_VK_F15  kVK_F15
#   define _P_VK_F16  kVK_F16
#   define _P_VK_F17  kVK_F17
#   define _P_VK_F18  kVK_F18
#   define _P_VK_F19  kVK_F19
#   define _P_VK_F20  kVK_F20
#   define _P_VK_F21  __P_VK_UNK(9)
#   define _P_VK_F22  __P_VK_UNK(10)
#   define _P_VK_F23  __P_VK_UNK(11)
#   define _P_VK_F24  __P_VK_UNK(12)

// -- virtual key codes - number keys --

#   define _P_VK_0  kVK_ANSI_0
#   define _P_VK_1  kVK_ANSI_1
#   define _P_VK_2  kVK_ANSI_2
#   define _P_VK_3  kVK_ANSI_3
#   define _P_VK_4  kVK_ANSI_4
#   define _P_VK_5  kVK_ANSI_5
#   define _P_VK_6  kVK_ANSI_6
#   define _P_VK_7  kVK_ANSI_7
#   define _P_VK_8  kVK_ANSI_8
#   define _P_VK_9  kVK_ANSI_9

#   define _P_VK_NUMPAD_0  kVK_ANSI_Keypad0
#   define _P_VK_NUMPAD_1  kVK_ANSI_Keypad1
#   define _P_VK_NUMPAD_2  kVK_ANSI_Keypad2
#   define _P_VK_NUMPAD_3  kVK_ANSI_Keypad3
#   define _P_VK_NUMPAD_4  kVK_ANSI_Keypad4
#   define _P_VK_NUMPAD_5  kVK_ANSI_Keypad5
#   define _P_VK_NUMPAD_6  kVK_ANSI_Keypad6
#   define _P_VK_NUMPAD_7  kVK_ANSI_Keypad7
#   define _P_VK_NUMPAD_8  kVK_ANSI_Keypad8
#   define _P_VK_NUMPAD_9  kVK_ANSI_Keypad9

#   define _P_VK_DECIMAL    kVK_ANSI_KeypadDecimal
#   define _P_VK_SEPARATOR  __P_VK_UNK(15)

#   define _P_VK_ADD       kVK_ANSI_KeypadPlus
#   define _P_VK_DIVIDE    kVK_ANSI_KeypadDivide
#   define _P_VK_MULTIPLY  kVK_ANSI_KeypadMultiply
#   define _P_VK_SUBTRACT  kVK_ANSI_KeypadMinus

// -- virtual key codes - character keys --

#   define _P_VK_A  kVK_ANSI_A
#   define _P_VK_B  kVK_ANSI_B
#   define _P_VK_C  kVK_ANSI_C
#   define _P_VK_D  kVK_ANSI_D
#   define _P_VK_E  kVK_ANSI_E
#   define _P_VK_F  kVK_ANSI_F
#   define _P_VK_G  kVK_ANSI_G
#   define _P_VK_H  kVK_ANSI_H
#   define _P_VK_I  kVK_ANSI_I
#   define _P_VK_J  kVK_ANSI_J
#   define _P_VK_K  kVK_ANSI_K
#   define _P_VK_L  kVK_ANSI_L
#   define _P_VK_M  kVK_ANSI_M
#   define _P_VK_N  kVK_ANSI_N
#   define _P_VK_O  kVK_ANSI_O
#   define _P_VK_P  kVK_ANSI_P
#   define _P_VK_Q  kVK_ANSI_Q
#   define _P_VK_R  kVK_ANSI_R
#   define _P_VK_S  kVK_ANSI_S
#   define _P_VK_T  kVK_ANSI_T
#   define _P_VK_U  kVK_ANSI_U
#   define _P_VK_V  kVK_ANSI_V
#   define _P_VK_W  kVK_ANSI_W
#   define _P_VK_X  kVK_ANSI_X
#   define _P_VK_Y  kVK_ANSI_Y
#   define _P_VK_Z  kVK_ANSI_Z

#   define _P_VK_PUNCT_BRACK1  kVK_ANSI_LeftBracket
#   define _P_VK_PUNCT_BRACK2  kVK_ANSI_RightBracket
#   define _P_VK_PUNCT_COMMA   kVK_ANSI_Comma
#   define _P_VK_PUNCT_PERIOD  kVK_ANSI_Period
#   define _P_VK_PUNCT_MINUS   kVK_ANSI_Minus
#   define _P_VK_PUNCT_PLUS    kVK_ANSI_Equal
#   define _P_VK_PUNCT_MISC1   kVK_ANSI_Semicolon
#   define _P_VK_PUNCT_MISC2   kVK_ANSI_Slash
#   define _P_VK_PUNCT_MISC3   kVK_ANSI_Quote
#   define _P_VK_PUNCT_MISC4   kVK_ANSI_KeypadEquals
#   define _P_VK_PUNCT_MISC5   __P_VK_UNK(16)
#   define _P_VK_PUNCT_MISC6   __P_VK_UNK(17)
#   define _P_VK_GRAVE_EXP     kVK_ANSI_Grave
#   define _P_VK_BACKSLASH     kVK_ANSI_Backslash

// -- virtual key codes - multi-media keys --

#   define _P_VK_MEDIA_NEXT   __P_VK_UNK(20)
#   define _P_VK_MEDIA_PREV   __P_VK_UNK(21)
#   define _P_VK_MEDIA_PLAY   __P_VK_UNK(22)
#   define _P_VK_MEDIA_STOP   __P_VK_UNK(23)
#   define _P_VK_VOLUME_DOWN  kVK_VolumeDown
#   define _P_VK_VOLUME_MUTE  kVK_Mute
#   define _P_VK_VOLUME_UP    kVK_VolumeUp


// _____________________________________________________________________________

// -- iOS -- -------------------------------------------------------------------
# else // TARGET_OS_IPHONE
#   include <Foundation/Foundation.h>
#   include <UIKit/UIKit.h>

// -- virtual key codes - system --

#   define _P_VK_UNKNOWN    0xFFFF
#   define _P_VK_INTERRUPT  __P_VK_UNK(0)

// -- virtual key codes - modifier keys --

#   define _P_VK_L_ALT     UIKeyboardHIDUsageKeyboardLeftAlt
#   define _P_VK_R_ALT     UIKeyboardHIDUsageKeyboardRightAlt
#   define _P_VK_L_CTRL    UIKeyboardHIDUsageKeyboardLeftControl
#   define _P_VK_R_CTRL    UIKeyboardHIDUsageKeyboardRightControl
#   define _P_VK_L_SHIFT   UIKeyboardHIDUsageKeyboardLeftShift
#   define _P_VK_R_SHIFT   UIKeyboardHIDUsageKeyboardRightShift
#   define _P_VK_L_SYSTEM  UIKeyboardHIDUsageKeyboardLeftGUI
#   define _P_VK_R_SYSTEM  UIKeyboardHIDUsageKeyboardRightGUI

#   define _P_VK_CAPS_LOCK    UIKeyboardHIDUsageKeyboardCapsLock
#   define _P_VK_NUM_LOCK     UIKeyboardHIDUsageKeypadNumLock
#   define _P_VK_SCROLL_LOCK  UIKeyboardHIDUsageKeyboardScrollLock

// -- virtual key codes - command keys --

#   define _P_VK_APP_MENU   UIKeyboardHIDUsageKeyboardApplication
#   define _P_VK_BACKSPACE  UIKeyboardHIDUsageKeyboardDeleteOrBackspace
#   define _P_VK_CLEAR      UIKeyboardHIDUsageKeyboardClear
#   define _P_VK_ENTER      UIKeyboardHIDUsageKeyboardReturn
#   define _P_VK_ENTER_PAD  UIKeyboardHIDUsageKeypadEnter
#   define _P_VK_EXECUTE    UIKeyboardHIDUsageKeyboardExecute
#   define _P_VK_ESC        UIKeyboardHIDUsageKeyboardEscape
#   define _P_VK_HELP       UIKeyboardHIDUsageKeyboardHelp
#   define _P_VK_PAUSE      UIKeyboardHIDUsageKeyboardPause
#   define _P_VK_PRINT_SCR  UIKeyboardHIDUsageKeyboardPrintScreen
#   define _P_VK_SELECT     UIKeyboardHIDUsageKeyboardSelect
#   define _P_VK_SPACE      UIKeyboardHIDUsageKeyboardSpacebar
#   define _P_VK_TAB        UIKeyboardHIDUsageKeyboardTab

#   define _P_VK_DELETE     UIKeyboardHIDUsageKeyboardDeleteForward
#   define _P_VK_END        UIKeyboardHIDUsageKeyboardEnd
#   define _P_VK_HOME       UIKeyboardHIDUsageKeyboardHome
#   define _P_VK_INSERT     UIKeyboardHIDUsageKeyboardInsert
#   define _P_VK_PG_DOWN    UIKeyboardHIDUsageKeyboardPageDown
#   define _P_VK_PG_UP      UIKeyboardHIDUsageKeyboardPageUp

#   define _P_VK_ARROW_CENTER  __P_VK_UNK(8)
#   define _P_VK_ARROW_DOWN    UIKeyboardHIDUsageKeyboardDownArrow
#   define _P_VK_ARROW_LEFT    UIKeyboardHIDUsageKeyboardLeftArrow
#   define _P_VK_ARROW_RIGHT   UIKeyboardHIDUsageKeyboardRightArrow
#   define _P_VK_ARROW_UP      UIKeyboardHIDUsageKeyboardUpArrow

#   define _P_VK_F1   UIKeyboardHIDUsageKeyboardF1
#   define _P_VK_F2   UIKeyboardHIDUsageKeyboardF2
#   define _P_VK_F3   UIKeyboardHIDUsageKeyboardF3
#   define _P_VK_F4   UIKeyboardHIDUsageKeyboardF4
#   define _P_VK_F5   UIKeyboardHIDUsageKeyboardF5
#   define _P_VK_F6   UIKeyboardHIDUsageKeyboardF6
#   define _P_VK_F7   UIKeyboardHIDUsageKeyboardF7
#   define _P_VK_F8   UIKeyboardHIDUsageKeyboardF8
#   define _P_VK_F9   UIKeyboardHIDUsageKeyboardF9
#   define _P_VK_F10  UIKeyboardHIDUsageKeyboardF10
#   define _P_VK_F11  UIKeyboardHIDUsageKeyboardF11
#   define _P_VK_F12  UIKeyboardHIDUsageKeyboardF12
#   define _P_VK_F13  UIKeyboardHIDUsageKeyboardF13
#   define _P_VK_F14  UIKeyboardHIDUsageKeyboardF14
#   define _P_VK_F15  UIKeyboardHIDUsageKeyboardF15
#   define _P_VK_F16  UIKeyboardHIDUsageKeyboardF16
#   define _P_VK_F17  UIKeyboardHIDUsageKeyboardF17
#   define _P_VK_F18  UIKeyboardHIDUsageKeyboardF18
#   define _P_VK_F19  UIKeyboardHIDUsageKeyboardF19
#   define _P_VK_F20  UIKeyboardHIDUsageKeyboardF20
#   define _P_VK_F21  UIKeyboardHIDUsageKeyboardF21
#   define _P_VK_F22  UIKeyboardHIDUsageKeyboardF22
#   define _P_VK_F23  UIKeyboardHIDUsageKeyboardF23
#   define _P_VK_F24  UIKeyboardHIDUsageKeyboardF24

// -- virtual key codes - number keys --

#   define _P_VK_0  UIKeyboardHIDUsageKeyboard0
#   define _P_VK_1  UIKeyboardHIDUsageKeyboard1
#   define _P_VK_2  UIKeyboardHIDUsageKeyboard2
#   define _P_VK_3  UIKeyboardHIDUsageKeyboard3
#   define _P_VK_4  UIKeyboardHIDUsageKeyboard4
#   define _P_VK_5  UIKeyboardHIDUsageKeyboard5
#   define _P_VK_6  UIKeyboardHIDUsageKeyboard6
#   define _P_VK_7  UIKeyboardHIDUsageKeyboard7
#   define _P_VK_8  UIKeyboardHIDUsageKeyboard8
#   define _P_VK_9  UIKeyboardHIDUsageKeyboard9

#   define _P_VK_NUMPAD_0  UIKeyboardHIDUsageKeypad0
#   define _P_VK_NUMPAD_1  UIKeyboardHIDUsageKeypad1
#   define _P_VK_NUMPAD_2  UIKeyboardHIDUsageKeypad2
#   define _P_VK_NUMPAD_3  UIKeyboardHIDUsageKeypad3
#   define _P_VK_NUMPAD_4  UIKeyboardHIDUsageKeypad4
#   define _P_VK_NUMPAD_5  UIKeyboardHIDUsageKeypad5
#   define _P_VK_NUMPAD_6  UIKeyboardHIDUsageKeypad6
#   define _P_VK_NUMPAD_7  UIKeyboardHIDUsageKeypad7
#   define _P_VK_NUMPAD_8  UIKeyboardHIDUsageKeypad8
#   define _P_VK_NUMPAD_9  UIKeyboardHIDUsageKeypad9

#   define _P_VK_DECIMAL    UIKeyboardHIDUsageKeypadPeriod
#   define _P_VK_SEPARATOR  UIKeyboardHIDUsageKeyboardSeparator

#   define _P_VK_ADD       UIKeyboardHIDUsageKeypadPlus
#   define _P_VK_DIVIDE    UIKeyboardHIDUsageKeypadSlash
#   define _P_VK_MULTIPLY  UIKeyboardHIDUsageKeypadAsterisk
#   define _P_VK_SUBTRACT  UIKeyboardHIDUsageKeypadHyphen

// -- virtual key codes - character keys --

#   define _P_VK_A  UIKeyboardHIDUsageKeyboardA
#   define _P_VK_B  UIKeyboardHIDUsageKeyboardB
#   define _P_VK_C  UIKeyboardHIDUsageKeyboardC
#   define _P_VK_D  UIKeyboardHIDUsageKeyboardD
#   define _P_VK_E  UIKeyboardHIDUsageKeyboardE
#   define _P_VK_F  UIKeyboardHIDUsageKeyboardF
#   define _P_VK_G  UIKeyboardHIDUsageKeyboardG
#   define _P_VK_H  UIKeyboardHIDUsageKeyboardH
#   define _P_VK_I  UIKeyboardHIDUsageKeyboardI
#   define _P_VK_J  UIKeyboardHIDUsageKeyboardJ
#   define _P_VK_K  UIKeyboardHIDUsageKeyboardK
#   define _P_VK_L  UIKeyboardHIDUsageKeyboardL
#   define _P_VK_M  UIKeyboardHIDUsageKeyboardM
#   define _P_VK_N  UIKeyboardHIDUsageKeyboardN
#   define _P_VK_O  UIKeyboardHIDUsageKeyboardO
#   define _P_VK_P  UIKeyboardHIDUsageKeyboardP
#   define _P_VK_Q  UIKeyboardHIDUsageKeyboardQ
#   define _P_VK_R  UIKeyboardHIDUsageKeyboardR
#   define _P_VK_S  UIKeyboardHIDUsageKeyboardS
#   define _P_VK_T  UIKeyboardHIDUsageKeyboardT
#   define _P_VK_U  UIKeyboardHIDUsageKeyboardU
#   define _P_VK_V  UIKeyboardHIDUsageKeyboardV
#   define _P_VK_W  UIKeyboardHIDUsageKeyboardW
#   define _P_VK_X  UIKeyboardHIDUsageKeyboardX
#   define _P_VK_Y  UIKeyboardHIDUsageKeyboardY
#   define _P_VK_Z  UIKeyboardHIDUsageKeyboardZ

#   define _P_VK_PUNCT_BRACK1  UIKeyboardHIDUsageKeyboardOpenBracket
#   define _P_VK_PUNCT_BRACK2  UIKeyboardHIDUsageKeyboardCloseBracket
#   define _P_VK_PUNCT_COMMA   UIKeyboardHIDUsageKeyboardComma
#   define _P_VK_PUNCT_PERIOD  UIKeyboardHIDUsageKeyboardPeriod
#   define _P_VK_PUNCT_MINUS   UIKeyboardHIDUsageKeyboardHyphen
#   define _P_VK_PUNCT_PLUS    UIKeyboardHIDUsageKeyboardEqualSign
#   define _P_VK_PUNCT_MISC1   UIKeyboardHIDUsageKeyboardSemicolon
#   define _P_VK_PUNCT_MISC2   UIKeyboardHIDUsageKeyboardSlash
#   define _P_VK_PUNCT_MISC3   UIKeyboardHIDUsageKeyboardQuote
#   define _P_VK_PUNCT_MISC4   UIKeyboardHIDUsageKeypadEqualSign
#   define _P_VK_PUNCT_MISC5   __P_VK_UNK(17)
#   define _P_VK_PUNCT_MISC6   __P_VK_UNK(18)
#   define _P_VK_GRAVE_EXP     UIKeyboardHIDUsageKeyboardGraveAccentAndTilde
#   define _P_VK_BACKSLASH     UIKeyboardHIDUsageKeyboardBackslash

// -- virtual key codes - multi-media keys --

#   define _P_VK_MEDIA_NEXT   __P_VK_UNK(20)
#   define _P_VK_MEDIA_PREV   __P_VK_UNK(21)
#   define _P_VK_MEDIA_PLAY   __P_VK_UNK(22)
#   define _P_VK_MEDIA_STOP   UIKeyboardHIDUsageKeyboardStop
#   define _P_VK_VOLUME_DOWN  UIKeyboardHIDUsageKeyboardVolumeDown
#   define _P_VK_VOLUME_MUTE  UIKeyboardHIDUsageKeyboardMute
#   define _P_VK_VOLUME_UP    UIKeyboardHIDUsageKeyboardVolumeUp
# endif // TARGET_OS_IPHONE


// _____________________________________________________________________________

// -- Android -- ---------------------------------------------------------------
#elif defined(__ANDROID)
# include <android/keycodes.h>

# define _P_VK_UNKNOWN    KEYCODE_UNKNOWN
# define _P_VK_INTERRUPT  __P_VK_UNK(0)

// -- virtual key codes - modifier keys --

# define _P_VK_L_ALT     KEYCODE_ALT_LEFT
# define _P_VK_R_ALT     KEYCODE_ALT_RIGHT
# define _P_VK_L_CTRL    KEYCODE_CTRL_LEFT
# define _P_VK_R_CTRL    KEYCODE_CTRL_RIGHT
# define _P_VK_L_SHIFT   KEYCODE_SHIFT_LEFT
# define _P_VK_R_SHIFT   KEYCODE_SHIFT_RIGHT
# define _P_VK_L_SYSTEM  KEYCODE_META_LEFT
# define _P_VK_R_SYSTEM  KEYCODE_META_RIGHT

# define _P_VK_CAPS_LOCK    __P_VK_UNK(1)
# define _P_VK_NUM_LOCK     __P_VK_UNK(2)
# define _P_VK_SCROLL_LOCK  __P_VK_UNK(3)

// -- virtual key codes - command keys --

# define _P_VK_APP_MENU   KEYCODE_MENU
# define _P_VK_BACKSPACE  KEYCODE_DEL
# define _P_VK_CLEAR      KEYCODE_CLEAR
# define _P_VK_ENTER      KEYCODE_ENTER
# define _P_VK_ENTER_PAD  KEYCODE_NUMPAD_ENTER
# define _P_VK_EXECUTE    __P_VK_UNK(4)
# define _P_VK_ESC        KEYCODE_ESCAPE
# define _P_VK_HELP       KEYCODE_HELP
# define _P_VK_PAUSE      KEYCODE_BREAK
# define _P_VK_PRINT_SCR  KEYCODE_SYSRQ
# define _P_VK_SELECT     KEYCODE_BUTTON_SELECT
# define _P_VK_SPACE      KEYCODE_SPACE
# define _P_VK_TAB        KEYCODE_TAB

# define _P_VK_DELETE     KEYCODE_FORWARD_DEL
# define _P_VK_END        KEYCODE_MOVE_END
# define _P_VK_HOME       KEYCODE_MOVE_HOME
# define _P_VK_INSERT     KEYCODE_INSERT
# define _P_VK_PG_DOWN    KEYCODE_PAGE_DOWN
# define _P_VK_PG_UP      KEYCODE_PAGE_UP

# define _P_VK_ARROW_CENTER  KEYCODE_DPAD_CENTER
# define _P_VK_ARROW_DOWN    KEYCODE_DPAD_DOWN
# define _P_VK_ARROW_LEFT    KEYCODE_DPAD_LEFT
# define _P_VK_ARROW_RIGHT   KEYCODE_DPAD_RIGHT
# define _P_VK_ARROW_UP      KEYCODE_DPAD_UP

# define _P_VK_F1   KEYCODE_F1
# define _P_VK_F2   KEYCODE_F2
# define _P_VK_F3   KEYCODE_F3
# define _P_VK_F4   KEYCODE_F4
# define _P_VK_F5   KEYCODE_F5
# define _P_VK_F6   KEYCODE_F6
# define _P_VK_F7   KEYCODE_F7
# define _P_VK_F8   KEYCODE_F8
# define _P_VK_F9   KEYCODE_F9
# define _P_VK_F10  KEYCODE_F10
# define _P_VK_F11  KEYCODE_F11
# define _P_VK_F12  KEYCODE_F12
# define _P_VK_F13  KEYCODE_F13
# define _P_VK_F14  KEYCODE_F14
# define _P_VK_F15  KEYCODE_F15
# define _P_VK_F16  KEYCODE_F16
# define _P_VK_F17  KEYCODE_F17
# define _P_VK_F18  KEYCODE_F18
# define _P_VK_F19  KEYCODE_F19
# define _P_VK_F20  KEYCODE_F20
# define _P_VK_F21  KEYCODE_F21
# define _P_VK_F22  KEYCODE_F22
# define _P_VK_F23  KEYCODE_F23
# define _P_VK_F24  KEYCODE_F24

// -- virtual key codes - number keys --

# define _P_VK_0  KEYCODE_0
# define _P_VK_1  KEYCODE_1
# define _P_VK_2  KEYCODE_2
# define _P_VK_3  KEYCODE_3
# define _P_VK_4  KEYCODE_4
# define _P_VK_5  KEYCODE_5
# define _P_VK_6  KEYCODE_6
# define _P_VK_7  KEYCODE_7
# define _P_VK_8  KEYCODE_8
# define _P_VK_9  KEYCODE_9

# define _P_VK_NUMPAD_0  KEYCODE_NUMPAD_0
# define _P_VK_NUMPAD_1  KEYCODE_NUMPAD_1
# define _P_VK_NUMPAD_2  KEYCODE_NUMPAD_2
# define _P_VK_NUMPAD_3  KEYCODE_NUMPAD_3
# define _P_VK_NUMPAD_4  KEYCODE_NUMPAD_4
# define _P_VK_NUMPAD_5  KEYCODE_NUMPAD_5
# define _P_VK_NUMPAD_6  KEYCODE_NUMPAD_6
# define _P_VK_NUMPAD_7  KEYCODE_NUMPAD_7
# define _P_VK_NUMPAD_8  KEYCODE_NUMPAD_8
# define _P_VK_NUMPAD_9  KEYCODE_NUMPAD_9

# define _P_VK_DECIMAL    KEYCODE_NUMPAD_DOT
# define _P_VK_SEPARATOR  KEYCODE_NUMPAD_COMMA

# define _P_VK_ADD       KEYCODE_NUMPAD_ADD
# define _P_VK_DIVIDE    KEYCODE_NUMPAD_DIVIDE
# define _P_VK_MULTIPLY  KEYCODE_NUMPAD_MULTIPLY
# define _P_VK_SUBTRACT  KEYCODE_NUMPAD_SUBTRACT

// -- virtual key codes - character keys --

# define _P_VK_A  KEYCODE_A
# define _P_VK_B  KEYCODE_B
# define _P_VK_C  KEYCODE_C
# define _P_VK_D  KEYCODE_D
# define _P_VK_E  KEYCODE_E
# define _P_VK_F  KEYCODE_F
# define _P_VK_G  KEYCODE_G
# define _P_VK_H  KEYCODE_H
# define _P_VK_I  KEYCODE_I
# define _P_VK_J  KEYCODE_J
# define _P_VK_K  KEYCODE_K
# define _P_VK_L  KEYCODE_L
# define _P_VK_M  KEYCODE_M
# define _P_VK_N  KEYCODE_N
# define _P_VK_O  KEYCODE_O
# define _P_VK_P  KEYCODE_P
# define _P_VK_Q  KEYCODE_Q
# define _P_VK_R  KEYCODE_R
# define _P_VK_S  KEYCODE_S
# define _P_VK_T  KEYCODE_T
# define _P_VK_U  KEYCODE_U
# define _P_VK_V  KEYCODE_V
# define _P_VK_W  KEYCODE_W
# define _P_VK_X  KEYCODE_X
# define _P_VK_Y  KEYCODE_Y
# define _P_VK_Z  KEYCODE_Z

# define _P_VK_PUNCT_BRACK1  KEYCODE_LEFT_BRACKET
# define _P_VK_PUNCT_BRACK2  KEYCODE_RIGHT_BRACKET
# define _P_VK_PUNCT_COMMA   KEYCODE_COMMA
# define _P_VK_PUNCT_PERIOD  KEYCODE_PERIOD
# define _P_VK_PUNCT_MINUS   KEYCODE_MINUS
# define _P_VK_PUNCT_PLUS    KEYCODE_PLUS
# define _P_VK_PUNCT_MISC1   KEYCODE_SEMICOLON
# define _P_VK_PUNCT_MISC2   KEYCODE_SLASH
# define _P_VK_PUNCT_MISC3   KEYCODE_APOSTROPHE
# define _P_VK_PUNCT_MISC4   KEYCODE_EQUALS
# define _P_VK_PUNCT_MISC5   KEYCODE_AT
# define _P_VK_PUNCT_MISC6   KEYCODE_STAR
# define _P_VK_GRAVE_EXP     KEYCODE_GRAVE
# define _P_VK_BACKSLASH     KEYCODE_BACKSLASH

// -- virtual key codes - multi-media keys --

# define _P_VK_MEDIA_NEXT   KEYCODE_MEDIA_NEXT
# define _P_VK_MEDIA_PREV   KEYCODE_MEDIA_PREVIOUS
# define _P_VK_MEDIA_PLAY   KEYCODE_MEDIA_PLAY_PAUSE
# define _P_VK_MEDIA_STOP   KEYCODE_MEDIA_STOP
# define _P_VK_VOLUME_DOWN  KEYCODE_VOLUME_DOWN
# define _P_VK_VOLUME_MUTE  KEYCODE_MUTE
# define _P_VK_VOLUME_UP    KEYCODE_VOLUME_UP


// _____________________________________________________________________________

// -- Linux/Unix/BSD -- --------------------------------------------------------
#else
# include <cstddef>
# include <cstdint>

// X11 keycodes: TODO (XKeycodeToKeysym)
// Wayland keycodes: TODO (???)
//...

// -- virtual key codes - system --

# define _P_VK_UNKNOWN    0xFFFF
# define _P_VK_INTERRUPT  __P_VK_UNK(0)

// -- virtual key codes - modifier keys --

# define _P_VK_L_ALT     __P_VK_UNK(1)
# define _P_VK_R_ALT     __P_VK_UNK(2)
# define _P_VK_L_CTRL    __P_VK_UNK(3)
# define _P_VK_R_CTRL    __P_VK_UNK(4)
# define _P_VK_L_SHIFT   __P_VK_UNK(5)
# define _P_VK_R_SHIFT   __P_VK_UNK(6)
# define _P_VK_L_SYSTEM  __P_VK_UNK(7)
# define _P_VK_R_SYSTEM  __P_VK_UNK(8)

# define _P_VK_CAPS_LOCK    __P_VK_UNK(9)
# define _P_VK_NUM_LOCK     __P_VK_UNK(10)
# define _P_VK_SCROLL_LOCK  __P_VK_UNK(11)

// -- virtual key codes - command keys --

# define _P_VK_APP_MENU   __P_VK_UNK(12)
# define _P_VK_BACKSPACE  __P_VK_UNK(13)
# define _P_VK_CLEAR      __P_VK_UNK(14)
# define _P_VK_ENTER      __P_VK_UNK(15)
# define _P_VK_ENTER_PAD  __P_VK_UNK(16)
# define _P_VK_EXECUTE    __P_VK_UNK(17)
# define _P_VK_ESC        __P_VK_UNK(18)
# define _P_VK_HELP       __P_VK_UNK(19)
# define _P_VK_PAUSE      __P_VK_UNK(20)
# define _P_VK_PRINT_SCR  __P_VK_UNK(21)
# define _P_VK_SELECT     __P_VK_UNK(22)
# define _P_VK_SPACE      __P_VK_UNK(23)
# define _P_VK_TAB        __P_VK_UNK(24)

# define _P_VK_DELETE     __P_VK_UNK(25)
# define _P_VK_END        __P_VK_UNK(26)
# define _P_VK_HOME       __P_VK_UNK(27)
# define _P_VK_INSERT     __P_VK_UNK(28)
# define _P_VK_PG_DOWN    __P_VK_UNK(29)
# define _P_VK_PG_UP      __P_VK_UNK(30)

# define _P_VK_ARROW_CENTER  __P_VK_UNK(31)
# define _P_VK_ARROW_DOWN    __P_VK_UNK(32)
# define _P_VK_ARROW_LEFT    __P_VK_UNK(33)
# define _P_VK_ARROW_RIGHT   __P_VK_UNK(34)
# define _P_VK_ARROW_UP      __P_VK_UNK(35)

# define _P_VK_F1   __P_VK_UNK(40)
# define _P_VK_F2   __P_VK_UNK(41)
# define _P_VK_F3   __P_VK_UNK(42)
# define _P_VK_F4   __P_VK_UNK(43)
# define _P_VK_F5   __P_VK_UNK(44)
# define _P_VK_F6   __P_VK_UNK(45)
# define _P_VK_F7   __P_VK_UNK(46)
# define _P_VK_F8   __P_VK_UNK(47)
# define _P_VK_F9   __P_VK_UNK(48)
# define _P_VK_F10  __P_VK_UNK(49)
# define _P_VK_F11  __P_VK_UNK(50)
# define _P_VK_F12  __P_VK_UNK(51)
# define _P_VK_F13  __P_VK_UNK(52)
# define _P_VK_F14  __P_VK_UNK(53)
# define _P_VK_F15  __P_VK_UNK(54)
# define _P_VK_F16  __P_VK_UNK(55)
# define _P_VK_F17  __P_VK_UNK(56)
# define _P_VK_F18  __P_VK_UNK(57)
# define _P_VK_F19  __P_VK_UNK(58)
# define _P_VK_F20  __P_VK_UNK(59)
# define _P_VK_F21  __P_VK_UNK(60)
# define _P_VK_F22  __P_VK_UNK(61)
# define _P_VK_F23  __P_VK_UNK(62)
# define _P_VK_F24  __P_VK_UNK(63)

// -- virtual key codes - number keys --

# define _P_VK_0  __P_VK_UNK(70)
# define _P_VK_1  (_P_VK_0 + 1)
# define _P_VK_2  (_P_VK_0 + 2)
# define _P_VK_3  (_P_VK_0 + 3)
# define _P_VK_4  (_P_VK_0 + 4)
# define _P_VK_5  (_P_VK_0 + 5)
# define _P_VK_6  (_P_VK_0 + 6)
# define _P_VK_7  (_P_VK_0 + 7)
# define _P_VK_8  (_P_VK_0 + 8)
# define _P_VK_9  (_P_VK_0 + 9)

# define _P_VK_NUMPAD_0  __P_VK_UNK(80)
# define _P_VK_NUMPAD_1  __P_VK_UNK(81)
# define _P_VK_NUMPAD_2  __P_VK_UNK(82)
# define _P_VK_NUMPAD_3  __P_VK_UNK(83)
# define _P_VK_NUMPAD_4  __P_VK_UNK(84)
# define _P_VK_NUMPAD_5  __P_VK_UNK(85)
# define _P_VK_NUMPAD_6  __P_VK_UNK(86)
# define _P_VK_NUMPAD_7  __P_VK_UNK(87)
# define _P_VK_NUMPAD_8  __P_VK_UNK(88)
# define _P_VK_NUMPAD_9  __P_VK_UNK(89)

# define _P_VK_DECIMAL    __P_VK_UNK(90)
# define _P_VK_SEPARATOR  __P_VK_UNK(91)

# define _P_VK_ADD       __P_VK_UNK(92)
# define _P_VK_DIVIDE    __P_VK_UNK(93)
# define _P_VK_MULTIPLY  __P_VK_UNK(94)
# define _P_VK_SUBTRACT  __P_VK_UNK(95)

// -- virtual key codes - character keys --

# define _P_VK_A  __P_VK_UNK(100)
# define _P_VK_B  (_P_VK_A + 1)
# define _P_VK_C  (_P_VK_A + 2)
# define _P_VK_D  (_P_VK_A + 3)
# define _P_VK_E  (_P_VK_A + 4)
# define _P_VK_F  (_P_VK_A + 5)
# define _P_VK_G  (_P_VK_A + 6)
# define _P_VK_H  (_P_VK_A + 7)
# define _P_VK_I  (_P_VK_A + 8)
# define _P_VK_J  (_P_VK_A + 9)
# define _P_VK_K  (_P_VK_A + 10)
# define _P_VK_L  (_P_VK_A + 11)
# define _P_VK_M  (_P_VK_A + 12)
# define _P_VK_N  (_P_VK_A + 13)
# define _P_VK_O  (_P_VK_A + 14)
# define _P_VK_P  (_P_VK_A + 15)
# define _P_VK_Q  (_P_VK_A + 16)
# define _P_VK_R  (_P_VK_A + 17)
# define _P_VK_S  (_P_VK_A + 18)
# define _P_VK_T  (_P_VK_A + 19)
# define _P_VK_U  (_P_VK_A + 20)
# define _P_VK_V  (_P_VK_A + 21)
# define _P_VK_W  (_P_VK_A + 22)
# define _P_VK_X  (_P_VK_A + 23)
# define _P_VK_Y  (_P_VK_A + 24)
# define _P_VK_Z  (_P_VK_A + 25)

# define _P_VK_PUNCT_BRACK1  __P_VK_UNK(130)
# define _P_VK_PUNCT_BRACK2  __P_VK_UNK(131)
# define _P_VK_PUNCT_COMMA   __P_VK_UNK(132)
# define _P_VK_PUNCT_PERIOD  __P_VK_UNK(133)
# define _P_VK_PUNCT_MINUS   __P_VK_UNK(134)
# define _P_VK_PUNCT_PLUS    __P_VK_UNK(135)
# define _P_VK_PUNCT_MISC1   __P_VK_UNK(136)
# define _P_VK_PUNCT_MISC2   __P_VK_UNK(137)
# define _P_VK_PUNCT_MISC3   __P_VK_UNK(138)
# define _P_VK_PUNCT_MISC4   __P_VK_UNK(139)
# define _P_VK_PUNCT_MISC5   __P_VK_UNK(140)
# define _P_VK_PUNCT_MISC6   __P_VK_UNK(141)
# define _P_VK_GRAVE_EXP     __P_VK_UNK(142)
# define _P_VK_BACKSLASH     __P_VK_UNK(143)

// -- virtual key codes - multi-media keys --

# define _P_VK_MEDIA_NEXT   __P_VK_UNK(150)
# define _P_VK_MEDIA_PREV   __P_VK_UNK(151)
# define _P_VK_MEDIA_PLAY   __P_VK_UNK(152)
# define _P_VK_MEDIA_STOP   __P_VK_UNK(153)
# define _P_VK_VOLUME_DOWN  __P_VK_UNK(154)
# define _P_VK_VOLUME_MUTE  __P_VK_UNK(155)
# define _P_VK_VOLUME_UP    __P_VK_UNK(156)

#endif
