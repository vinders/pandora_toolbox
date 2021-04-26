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

# include <stddef.h>
# include <string.h>
# import <UIKit/UIApplication.h>
# import <UIKit/UIScreen.h>
# import <UIKit/UIScreenMode.h>
# import <UIKit/UIView.h>
# import "video/_private/_message_box_impl_uikit.h"

#endif
