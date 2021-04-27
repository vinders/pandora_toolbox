/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - iOS implementation
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE

# include <assert.h>
# include <stddef.h>
# include <string.h>
# import <UIKit/UIApplication.h>

# ifdef __IPHONE_8_0
#   import <UIKit/UIWindow.h>
#   import <UIKit/UIScreen.h>
#   import <UIKit/UIViewController.h>
#   import <UIKit/UIAlertController.h>
#   import <UIKit/UIAlertAction.h>
# elif __IPHONE_OS_VERSION_MIN_REQUIRED < 80000
#   import <UIKit/UIAlertView.h>
#   import <UIKit/UIAlertViewDelegate.h>
# endif
# import "video/_private/_message_box_impl_uikit.h"

# define UIKitLocalizedString(key) [[NSBundle bundleWithIdentifier:@"com.apple.UIKit"] localizedStringForKey:key value:@"" table:nil]


  // error message allocation and copy
  static void __setErrorMessage(const char* message, char** outError) {
    uint32_t length = (uint32_t)strnlen(message, 512);
    *outError = calloc(length, sizeof(char));
    if (*outError)
      memcpy((void*)*outError, (void*)message, length*sizeof(char));
  }

  // ---
  
# ifdef __IPHONE_8_0
    // AlertController creation
    static uint32_t __showAlertController_uikit(NSString* caption, NSString* message, 
                                                NSString* buttons[3], uint32_t length, void* parentWindow, char** outError) {
      if (![UIAlertController class]) {
        __setErrorMessage("UIKit: Alerts not supported on current system...", outError); 
        return 0;
      }
      
      // create dialog + add buttons
      uint32_t __block userAction = 0;
      UIAlertController* alert = [UIAlertController alertControllerWithTitle:caption message:message
                                                    preferredStyle:UIAlertControllerStyleAlert];
      for (uint32_t i = 0; i < length; ++i) {
        UIAlertActionStyle buttonStyle = (i == 0 && length > 1) ? UIAlertActionStyleCancel : UIAlertActionStyleDefault;
        UIAlertAction* action = [UIAlertAction actionWithTitle:buttons[i] style:buttonStyle
                                               handler:^(UIAlertAction* action) { userAction = i + 1; }]; // action = button index + 1
        [alert addAction:action];
      }
      
      // get window context
      Bool isNewController = Bool_FALSE;
      UIWindow* window = (parentWindow) ? (UIWindow*)parentWindow : nil;
      
      if (window == nil || window.rootViewController == nil) {
        window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        if (window == nil) {
          __setErrorMessage("UIKit: Alert failure: could not access main window", outError);
          return 0;
        }
          
        window.rootViewController = [UIViewController new];
        window.windowLevel = UIWindowLevelAlert;
        [window makeKeyAndVisible];
        isNewController = Bool_TRUE;
      }

      // show modal dialog + wait for user action
      [window.rootViewController presentViewController:alert animated:YES completion:nil];
      while (userAction == 0)
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];

      if (isNewController)
        window.hidden = YES;
      return userAction;
    }
    
# elif __IPHONE_OS_VERSION_MIN_REQUIRED < 80000
    // delegate class for AlertView creation
    @interface __P_AlertViewDelegate : NSObject <UIAlertViewDelegate>
    @property (nonatomic, assign) int* userAction;
    @end

    @implementation __P_AlertViewDelegate
    - (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
    {
      if (_userAction != NULL)
        *_userAction = (int)buttonIndex + 1;
    }
    @end

    // AlertView creation
    static uint32_t __showAlertView_uikit(NSString* caption, NSString* message, 
                                          NSString* buttons[3], uint32_t length, char** outError) {
      int userAction = 0;
      __P_AlertViewDelegate* handler = [[__P_AlertViewDelegate alloc] init];
      handler.userAction = &userAction;
      
      // create dialog + add buttons
      UIAlertView* alert = NULL;
      switch (length) {
        case 1: alert = [[UIAlertView alloc] initWithTitle:message message:message delegate:handler 
                                             cancelButtonTitle:nil otherButtonTitles:buttons[0], nil];
                break;
        case 2: alert = [[UIAlertView alloc] initWithTitle:message message:message delegate:handler 
                                             cancelButtonTitle:buttons[0] otherButtonTitles:buttons[1], nil];
                break;
        case 3: alert = [[UIAlertView alloc] initWithTitle:message message:message delegate:handler 
                                             cancelButtonTitle:buttons[0] otherButtonTitles:buttons[1], buttons[2], nil];
                break;
      }
      if (alert == NULL) {
        __setErrorMessage("UIKit: Alert failure: could not create view", outError);
        return 0;
      }
      
      // show modal dialog + wait for user action
      [alert setAlertViewStyle:UIAlertViewStyleDefault];
      [alert show];

      // wait for user action
      while (userAction == 0)
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
      
      [alert setDelegate:nil];
      return static_cast<uint32_t>(userAction);
    }
# endif
  
  // ---
  
  // show modal message box (reverse order expected: cancellation first, confirmation last)
  uint32_t __showMessageBox_uikit(const char* caption, const char* message, enum UikitBoxIconId icon, 
                                  const char** actions, uint32_t length, void* parentWindow, char** outError) {
    @autoreleasepool {
      @try {
        // add icon before caption
        NSMutableString* captionStr = [[NSMutableString alloc]init];
        switch (icon) {
          case UIKIT_BOX_ICON_INFO:     [captionStr appendString:@"\u1F5CA "]; break;
          case UIKIT_BOX_ICON_QUESTION: [captionStr appendString:@"\uFFFD "]; break;
          case UIKIT_BOX_ICON_WARNING:  [captionStr appendString:@"\u26A0 "]; break;
          case UIKIT_BOX_ICON_ERROR:    [captionStr appendString:@"\u26A0 "]; break;
        }
        
        // content
        if (caption)
          [captionStr appendString:[NSString stringWithUTF8String:caption]];
        else
          [captionStr appendString:@"Error"];
        NSString* messageStr = (message) ? [NSString stringWithUTF8String:message] : @"";

        // actions
        NSString* buttons[3] = { NULL };
        for (uint32_t i = 0; i < length; ++i) {
          NSString* label = [NSString stringWithUTF8String:actions[i]];
          buttons[i] = UIKitLocalizedString(label);
          if (buttons[i] == NULL || [buttons[i] length] == 0)
            buttons[i] = label;
        }
        
        // show modal + wait for user action
#       ifdef __IPHONE_8_0
          return __showAlertController_uikit([NSString stringWithString:captionStr], messageStr, buttons, length, parentWindow, outError);
#       elif __IPHONE_OS_VERSION_MIN_REQUIRED < 80000
          return __showAlertView_uikit([NSString stringWithString:captionStr], messageStr, buttons, length, outError);
#       else
          __setErrorMessage("UIKit: Alerts not supported on current system...", outError); 
          return 0;
#       endif  
      }
      @catch (NSException* exc) { 
        __setErrorMessage([[exc reason] UTF8String], outError); 
        return 0; 
      }
    }
  }

#endif
