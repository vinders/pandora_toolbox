/*******************************************************************************
MIT License
Copyright (c) 2022 Romain Vinders

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
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_VIDEO_VULKAN_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- error messages -- --------------------------------------------------------

  // Exception class with LightString
  class RuntimeException final : public std::runtime_error {
  public:
    RuntimeException(std::shared_ptr<pandora::memory::LightString>&& msg) noexcept : std::runtime_error(msg->c_str()), _message(std::move(msg)) {}
    RuntimeException(const RuntimeException& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException(RuntimeException&& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException& operator=(const RuntimeException& rhs) noexcept { std::runtime_error::operator=(rhs); _message = rhs._message; return *this; }
    virtual ~RuntimeException() noexcept { _message.reset(); }
  private:
    std::shared_ptr<pandora::memory::LightString> _message;
  };
  
  // ---
  
  static inline const char* __getVulkanError(VkResult result) {
    switch (result) {
      case VK_NOT_READY: return "NOT_READY";
      case VK_TIMEOUT: return "TIMEOUT";
      case VK_EVENT_SET: return "EVENT_SET";
      case VK_EVENT_RESET: return "EVENT_RESET";
      case VK_INCOMPLETE: return "INCOMPLETE";
      case VK_ERROR_OUT_OF_HOST_MEMORY: return "OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_INITIALIZATION_FAILED: return "INITIALIZATION_FAILED";
      case VK_ERROR_DEVICE_LOST: return "DEVICE_LOST";
      case VK_ERROR_MEMORY_MAP_FAILED: return "MEMORY_MAP_FAILED";
      case VK_ERROR_LAYER_NOT_PRESENT: return "LAYER_NOT_PRESENT";
      case VK_ERROR_EXTENSION_NOT_PRESENT: return "EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT: return "FEATURE_NOT_PRESENT";
      case VK_ERROR_INCOMPATIBLE_DRIVER: return "INCOMPATIBLE_DRIVER";
      case VK_ERROR_TOO_MANY_OBJECTS: return "TOO_MANY_OBJECTS";
      case VK_ERROR_FORMAT_NOT_SUPPORTED: return "FORMAT_NOT_SUPPORTED";
      case VK_ERROR_SURFACE_LOST_KHR: return "SURFACE_LOST_KHR";
      case VK_ERROR_OUT_OF_DATE_KHR: return "OUT_OF_DATE_KHR";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "INCOMPATIBLE_DISPLAY_KHR";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "NATIVE_WINDOW_IN_USE_KHR"; // window already used by a surface or another API
      case VK_ERROR_VALIDATION_FAILED_EXT: return "VALIDATION_FAILED_EXT";
      case VK_SUBOPTIMAL_KHR: return "SUBOPTIMAL_KHR"; // swapchain doesn't match surface properties exactly, but can still be used
      case VK_ERROR_FRAGMENTED_POOL: return "FRAGMENTED_POOL";
      case VK_ERROR_OUT_OF_POOL_MEMORY: return "OUT_OF_POOL_MEMORY";
      case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "INVALID_EXTERNAL_HANDLE";
      case VK_ERROR_FRAGMENTATION: return "FRAGMENTATION";
      default: return "INTERNAL_ERROR";
    }
  }

  void pandora::video::vulkan::throwError(VkResult result, const char* messagePrefix) {
    const char* vkError = __getVulkanError(result);

    // pre-compute total size to avoid having multiple dynamic allocs
    size_t prefixSize = strlen(messagePrefix);
    size_t errorSize = strlen(vkError);
    auto message = std::make_shared<pandora::memory::LightString>(prefixSize + 2u + errorSize);
    
    // copy message in preallocated string
    if (!message->empty()) { // if no alloc failure
      memcpy((void*)message->data(),                 messagePrefix, prefixSize*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize]),    ": ",        size_t{2u}*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize+2u]), vkError,     errorSize *sizeof(char));
    }
    throw RuntimeException(std::move(message));
  }

#endif
