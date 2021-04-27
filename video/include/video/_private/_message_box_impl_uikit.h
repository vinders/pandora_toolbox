/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifndef __OBJC__
# pragma once
#endif
#ifndef _MESSAGE_BOX_UIKIT_IMPL_H
# define _MESSAGE_BOX_UIKIT_IMPL_H 1

# if !defined(_WINDOWS) && defined(__APPLE__)
#   include <TargetConditionals.h>
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#     ifndef __OBJC__
        extern "C" {
#     endif
#     include <stdint.h>


      // -- bindings --

      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0

      enum UikitBoxIconId { 
        UIKIT_BOX_ICON_NONE = 0,
        UIKIT_BOX_ICON_INFO = 1,
        UIKIT_BOX_ICON_QUESTION = 2,
        UIKIT_BOX_ICON_WARNING = 3,
        UIKIT_BOX_ICON_ERROR = 4
      };
     
      
      // -- methods --
      
      // show modal message box (reverse order expected: cancellation first, confirmation last)
      uint32_t __showMessageBox_uikit(const char* caption, const char* message, enum UikitBoxIconId icon, 
                                      const char** actions, uint32_t length, void* parentWindow, char** outError);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
