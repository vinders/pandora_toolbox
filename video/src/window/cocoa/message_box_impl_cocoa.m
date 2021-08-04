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
Description : Message box - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)

# include <assert.h>
# include <stddef.h>
# include <string.h>
# import <Cocoa/Cocoa.h>
# import <ApplicationServices/ApplicationServices.h>
# import "video/_private/_message_box_impl_cocoa.h"

  // set icon style of message box
  static void __setMessageBoxIcon(NSAlert* alert, enum CocoaBoxIconId icon) {
    if (@available(macOS 10.12, *)) {
      switch (icon) {
        case COCOA_BOX_ICON_INFO:
        case COCOA_BOX_ICON_QUESTION: [alert setAlertStyle:NSAlertStyleInformational]; break;
        case COCOA_BOX_ICON_WARNING: [alert setAlertStyle:NSAlertStyleWarning]; break;
        case COCOA_BOX_ICON_ERROR: [alert setAlertStyle:NSAlertStyleCritical]; break;
        default: [alert setIcon:[NSImage new]]; break;
      }
    }
#   if defined(MAC_OS_X_VERSION_MAX_ALLOWED) && MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
      else {
        switch (icon) {
          case COCOA_BOX_ICON_INFO:
          case COCOA_BOX_ICON_QUESTION: [alert setAlertStyle:NSInformationalAlertStyle]; break;
          case COCOA_BOX_ICON_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
          case COCOA_BOX_ICON_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
          default: [alert setIcon:[NSImage new]]; break;
        }
      }
#   endif
  }

  // error message allocation and copy
  static void __setErrorMessage(const char* message, char** outError) {
    uint32_t length = (uint32_t)strnlen(message, 512);
    *outError = calloc(length, sizeof(char));
    if (*outError)
      memcpy((void*)*outError, (void*)message, length*sizeof(char));
  }

  // ---

  // show modal message box (reverse order expected: cancellation first, confirmation last)
  uint32_t __showMessageBox_cocoa(const char* caption, const char* message, enum CocoaBoxIconId icon, 
                                  const char** actions, uint32_t length, Bool isTopMost, char** outError) {
    @autoreleasepool {
      @try {
        NSAlert* alert = [[[NSAlert alloc] init] autorelease];
        
        // content
        __setMessageBoxIcon(alert, icon);
        if (caption)
          [alert setMessageText:[NSString stringWithUTF8String:caption]];
        else
          [alert setMessageText:@"Error"];
        if (message)
          [alert setInformativeText:[NSString stringWithUTF8String:message]];

        // actions
        assert(length > 0);
        NSString* label = [NSString stringWithUTF8String:actions[0]];
        NSButton* lastButton = [alert addButtonWithTitle:NSLocalizedString(label, comment: label)];
        if (length > 1) {
          [lastButton setKeyEquivalent: @"\033"]; // Escape
          [lastButton setKeyEquivalentModifierMask:0];
          
          for (uint32_t i = 1; i < length; ++i) {
            label = [NSString stringWithUTF8String:actions[i]];
            lastButton = [alert addButtonWithTitle:NSLocalizedString(label, comment: label)];
            if (i + 1 < length)
              [lastButton setKeyEquivalent:@""];
          }
        }
        [lastButton setKeyEquivalent:@"\r"]; // Enter (return)
          
        // z-index
        if (isTopMost)
          [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps]; // top-most
        else
          [[NSRunningApplication currentApplication] activateWithOptions:0]; // top current app

        // show modal + wait for user action
        NSModalResponse result = [alert runModal];
        switch(result) {
          case NSAlertFirstButtonReturn:  return 1;
          case NSAlertSecondButtonReturn: if (length >= 2) { return 2; } break;
          case NSAlertThirdButtonReturn:  if (length >= 3) { return 3; } break;
        }
        __setErrorMessage("NSModalResponse: unidentified user action", outError); 
        return 0;
      }
      @catch (NSException* exc) { 
        __setErrorMessage([[exc reason] UTF8String], outError); 
        return 0; 
      }
    }
  }

#endif
