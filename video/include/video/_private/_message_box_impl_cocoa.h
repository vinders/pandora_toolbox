/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
      
      // show modal message box
      uint32_t __showMessageBox_cocoa(const char* caption, const char* message, enum CocoaBoxIconId icon, 
                                      const char* actions[3], uint32_t length, Bool isTopMost, char** outError);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
