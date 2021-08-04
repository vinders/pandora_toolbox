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
#ifndef __OBJC__
# pragma once
#endif
#ifndef _MESSAGE_BOX_COCOA_IMPL_H
# define _MESSAGE_BOX_COCOA_IMPL_H 1

# if !defined(_WINDOWS) && defined(__APPLE__)
#   include <TargetConditionals.h>
#   if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
#     ifndef __OBJC__
        extern "C" {
#     endif
#     include <stdint.h>


      // -- bindings --

      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0

      enum CocoaBoxIconId { 
        COCOA_BOX_ICON_NONE = 0,
        COCOA_BOX_ICON_INFO = 1,
        COCOA_BOX_ICON_QUESTION = 2,
        COCOA_BOX_ICON_WARNING = 3,
        COCOA_BOX_ICON_ERROR = 4
      };
     
      
      // -- methods --
      
      // show modal message box (reverse order expected: cancellation first, confirmation last)
      uint32_t __showMessageBox_cocoa(const char* caption, const char* message, enum CocoaBoxIconId icon, 
                                      const char** actions, uint32_t length, Bool isTopMost, char** outError);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
