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

      enum CocoaBoxButtonId { 
        COCOA_BOX_BUTTON_FAILURE = -1, 
        COCOA_BOX_BUTTON_OK = 0, 
        COCOA_BOX_BUTTON_CANCEL = 1, 
        COCOA_BOX_BUTTON_YES = 2, 
        COCOA_BOX_BUTTON_NO = 3, 
        COCOA_BOX_BUTTON_ABORT = 4, 
        COCOA_BOX_BUTTON_RETRY = 5, 
        COCOA_BOX_BUTTON_IGNORE = 6
      };
      enum CocoaBoxIconId { 
        COCOA_BOX_ICON_NONE = 0,
        COCOA_BOX_ICON_INFO = 1,
        COCOA_BOX_ICON_QUESTION = 2,
        COCOA_BOX_ICON_WARNING = 3,
        COCOA_BOX_ICON_ERROR = 4
      };
      
      
      // -- methods --
      
      // show modal message box
      enum CocoaBoxButtonId __showMessageBox_cocoa(const char* caption, const char* message, enum CocoaBoxIconId icon, 
                                                   enum CocoaBoxButtonId* actions, uint32_t actionsLength, Bool isTopMost, char** outError);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
