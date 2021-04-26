/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)

# include <stddef.h>
# include <string.h>
# import <Cocoa/Cocoa.h>
# import <ApplicationServices/ApplicationServices.h>
# import "video/_private/_message_box_impl_cocoa.h"


  // set icon style of message box
  static NSButton* __setMessageBoxIcon(NSAlert* alert, enum CocoaBoxIconId icon) {
    switch (icon) {
      case COCOA_BOX_ICON_INFO:
      case COCOA_BOX_ICON_QUESTION: [alert setAlertStyle:NSInformationalAlertStyle]; break;
      case COCOA_BOX_ICON_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
      case COCOA_BOX_ICON_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
      default: [alert setProperty:[NSImage new] forKey:@"image"]; break;
    }
  }
  // add action button to message box
  static NSButton* __addMessageBoxAction(NSAlert* alert, enum CocoaBoxButtonId action) {
    switch (action) {
      case COCOA_BOX_BUTTON_OK:     return [alert addButtonWithTitle:NSLocalizedString(@"OK", comment: "OK")];
      case COCOA_BOX_BUTTON_CANCEL: return [alert addButtonWithTitle:NSLocalizedString(@"Cancel", comment: "Cancel")];
      case COCOA_BOX_BUTTON_YES:    return [alert addButtonWithTitle:NSLocalizedString(@"Yes", comment: "Yes")];
      case COCOA_BOX_BUTTON_NO:     return [alert addButtonWithTitle:NSLocalizedString(@"No", comment: "No")];
      case COCOA_BOX_BUTTON_ABORT:  return [alert addButtonWithTitle:NSLocalizedString(@"Abort", comment: "Abort")];
      case COCOA_BOX_BUTTON_RETRY:  return [alert addButtonWithTitle:NSLocalizedString(@"Retry", comment: "Retry")];
      case COCOA_BOX_BUTTON_IGNORE: return [alert addButtonWithTitle:NSLocalizedString(@"Ignore", comment: "Ignore")];
      default: return [alert addButtonWithTitle:NSLocalizedString(@"OK", comment: "OK")];
    }
  }
  // error message allocation and copy
  static void __setErrorMessage(const char* message, char** outError) {
    uint32_t length = (uint32_t)strlen(message, 512);
    *outError = calloc(length, sizeof(char));
    if (*outError)
      memcpy((void*)*outError, (void*)message, length*sizeof(char));
  }

  // ---

  // show modal message box
  enum CocoaBoxButtonId __showMessageBox_cocoa(const char* caption, const char* message, enum CocoaBoxIconId icon, 
                                               enum CocoaBoxButtonId* actions, uint32_t actionsLength, Bool isTopMost, char** outError) {
    @autoreleasepool {
      @try {
        NSAlert* alert = [[[NSAlert alloc] init] autorelease];
        
        // content
        __setMessageBoxIcon(icon);
        if (caption)
          [alert setMessageText:[NSString stringWithUTF8String:caption]];
        else
          [alert setMessageText:@"Error"];
        if (message)
          [alert setInformativeText:[NSString stringWithUTF8String:message]];

        // actions
        assert(actionsLength > 0);
        NSButton* lastButton = __addMessageBoxAction(alert, actions[0]);
        if (actionsLength > 1) {
          [lastButton setKeyEquivalent: @"\033"]; // Escape
          [lastButton setKeyEquivalentModifierMask:0];
          
          for (uint32_t i = 1; i < actionsLength; ++i) {
            lastButton = __addMessageBoxAction(alert, actions[i]);
            if (i + 1 < actionsLength)
              [lastButton setKeyEquivalent:@""];
          }
        }
        [lastButton setKeyEquivalent:@"\r"]; // Enter (return)
          
        // z-index
        if (isTopMost)
          [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps]; // top-most
        else
          [[NSRunningApplication currentApplication] activateWithOptions:0];// top current app

        // show modal + wait for user action
        NSModalResponse result = [alert runModal];
        switch(result) {
          case NSAlertFirstButtonReturn:      return actions[0];
          case NSAlertSecondButtonReturn:     return (actionsLength > 1) ? actions[1] : actions[0];
          case NSAlertThirdButtonReturn:      return (actionsLength > 2) ? actions[2] : actions[actionsLength-1];
          case (NSAlertThirdButtonReturn +1): return (actionsLength > 3) ? actions[3] : actions[actionsLength-1];
          default: __setErrorMessage("NSModalResponse: unidentified user action", outError); return COCOA_BOX_BUTTON_FAILURE;
        }
      }
      @catch (NSException* exc) { 
        __setErrorMessage([[exc reason] UTF8String], outError); 
        return COCOA_BOX_BUTTON_FAILURE; 
      }
    }
  }

#endif
