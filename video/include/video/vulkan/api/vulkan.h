/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

// Mac OS / iOS
#if !defined(_WINDOWS) && defined(__APPLE__)
  // TODO - moltenVK
  //...

// Windows / Linux / Android
#else
//#define VK_NO_PROTOTYPES 1 // only when dynamic linking
# include <cstdint>
# include <vulkan/vulkan.h>
#endif
