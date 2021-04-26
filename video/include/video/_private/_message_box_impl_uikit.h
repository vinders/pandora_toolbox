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




#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
