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


// -- color spaces -- ----------------------------------------------------------

# define __P_DEFAULT_COLORSPACE_SRGB  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR

  // Verify if a buffer format is supported to create swap-chains and render targets
  bool DisplaySurface::isFormatSupported(DataFormat bufferFormat) const noexcept {
    if (this->_renderer != nullptr) {
      auto format = _getDataFormatComponents(bufferFormat);

      uint32_t formatCount = 0;
      if (vkGetPhysicalDeviceSurfaceFormatsKHR(this->_renderer->_deviceContext.device(), this->_windowSurface,
                                               &formatCount, nullptr) == VK_SUCCESS && formatCount) {
        auto formats = DynamicArray<VkSurfaceFormatKHR>(formatCount);
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(this->_renderer->_deviceContext.device(),
                                                               this->_windowSurface, &formatCount, formats.value);
        if (result == VK_SUCCESS || result == VK_INCOMPLETE) {
          for (const VkSurfaceFormatKHR* it = formats.value; formatCount; --formatCount, ++it) {
            if (it->format == format)
              return true;
          }
          return false;
        }
      }
    }
    return (bufferFormat == DataFormat::rgba8_unorm || bufferFormat == DataFormat::rgba8_sRGB);
  }

  // Find color space for a buffer format
  static __forceinline VkColorSpaceKHR __getColorSpace(VkFormat backBufferFormat) noexcept {
    switch (backBufferFormat) {
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return VK_COLOR_SPACE_HDR10_ST2084_EXT; // HDR-10
      case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
      case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        return VK_COLOR_SPACE_BT709_NONLINEAR_EXT; // HDR-scRGB12
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16_SFLOAT:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
        return VK_COLOR_SPACE_BT709_LINEAR_EXT; // HDR-scRGB16
      default: break;
    }
    return __P_DEFAULT_COLORSPACE_SRGB; // SDR-sRGB
  }

  // Find swap-chain color format and color space
  // returns: color spaces supported (true) or not
  static inline VkSurfaceFormatKHR __findSwapChainFormat(const VkPhysicalDevice& device, VkSurfaceKHR windowSurface,
                                                         VkFormat backBufferFormat) { // throws
    uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, nullptr);
    if (result == VK_SUCCESS && formatCount) {
      auto formats = DynamicArray<VkSurfaceFormatKHR>(formatCount);
      result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, formats.value);
      if (result == VK_SUCCESS || result == VK_INCOMPLETE) {

        VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
        VkColorSpaceKHR preferredColorSpace = __getColorSpace(backBufferFormat);
        for (const VkSurfaceFormatKHR* it = formats.value; formatCount; --formatCount, ++it) {
          if (it->format == backBufferFormat) {
            if (it->colorSpace == preferredColorSpace) { // preferred color space found -> exit loop
              colorSpace = it->colorSpace;
              break;
            }
            else if (colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR) // first available color space -> store it (in case preferred is not found)
              colorSpace = it->colorSpace;
          }
        }
        if (colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR)
          return VkSurfaceFormatKHR{ backBufferFormat, colorSpace };

        // format not found -> use first available format (only if default format requested)
        if (backBufferFormat == VK_FORMAT_R8G8B8A8_SRGB || backBufferFormat == VK_FORMAT_R8G8B8A8_UNORM)
          return VkSurfaceFormatKHR(formats.value[0]);
      }
    }

    // query failed -> only default format allowed
    if (backBufferFormat != VK_FORMAT_R8G8B8A8_SRGB && backBufferFormat != VK_FORMAT_R8G8B8A8_UNORM)
      throwError(result, "SwapChain: color space query");
    return VkSurfaceFormatKHR{ backBufferFormat, __P_DEFAULT_COLORSPACE_SRGB };
  }

  // Find swap-chain presentation mode
  inline VkPresentModeKHR SwapChain::_findPresentMode(pandora::video::PresentMode preferredMode, uint32_t framebufferCount) const { // throws
    uint32_t presentModeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(this->_renderer->device(), this->_windowSurface, &presentModeCount, nullptr);
    if (result != VK_SUCCESS || presentModeCount == 0)
      throwError(result, "SwapChain: can't count present modes");
    DynamicArray<VkPresentModeKHR> supportedModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(this->_renderer->device(), this->_windowSurface, &presentModeCount, supportedModes.value);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
      throwError(result, "SwapChain: can't read present modes");

    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    if (preferredMode == PresentMode::immediate) {
      for (size_t i = 0; i < presentModeCount; ++i) {
        if (supportedModes.value[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
          mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
          break;
        }
      }
    }
    else if (framebufferCount >= 3 && (this->_flags & SwapChain::OutputFlag::disableFlipSwap) == false) {
      for (size_t i = 0; i < presentModeCount; ++i) {
        if (supportedModes.value[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
          mode = VK_PRESENT_MODE_MAILBOX_KHR;
          break;
        }
      }
    }
    return mode;
  }

  // Find array index (in renderer) of presentation queue
  static inline uint32_t _findPresentQueueArrayIndex(const Renderer& renderer, VkSurfaceKHR windowSurface) { // throws
    uint32_t queueArrayIndex = (uint32_t)-1;

    auto* family = renderer.commandQueues().value;
    for (size_t i = 0; i < renderer.commandQueues().length(); ++i, ++family) {
      VkBool32 presentSupport = VK_FALSE;
      if (vkGetPhysicalDeviceSurfaceSupportKHR(renderer.device(), family->familyIndex,
                                               windowSurface, &presentSupport) == VK_SUCCESS && presentSupport != VK_FALSE) {
        queueArrayIndex = (uint32_t)i;
        break;
      }
    }
    if (queueArrayIndex == (uint32_t)-1)
      throw std::runtime_error("SwapChain: no compatible command queue family found");
    return queueArrayIndex;
  }

  // Force swap-chain size within min/max limits
  static inline void _constrainSwapChainExtents(const VkSurfaceCapabilitiesKHR& capabilities,
                                                uint32_t& inOutWidth, uint32_t& inOutHeight) noexcept {
    if (capabilities.minImageExtent.width != UINT32_MAX && capabilities.minImageExtent.width > inOutWidth)
      inOutWidth = capabilities.minImageExtent.width;
    else if (capabilities.maxImageExtent.width != 0 && capabilities.maxImageExtent.width < inOutWidth)
      inOutWidth = capabilities.maxImageExtent.width;
    if (capabilities.minImageExtent.height != UINT32_MAX && capabilities.minImageExtent.height > inOutHeight)
      inOutHeight = capabilities.minImageExtent.height;
    else if (capabilities.maxImageExtent.height != 0 && capabilities.maxImageExtent.height < inOutHeight)
      inOutHeight = capabilities.maxImageExtent.height;
  }


// -- swap-chain creation -- ---------------------------------------------------

  // Create output surface for a swap-chain
  DisplaySurface::DisplaySurface(Renderer& renderer, pandora::video::WindowHandle window)
    : _renderer(&renderer) {
    if (this->_renderer == nullptr || window == nullptr)
      throw std::invalid_argument("DisplaySurface: NULL window");

    VkResult result = VulkanLoader::instance().createWindowSurface(this->_renderer->vkInstance(), window, nullptr, this->_windowSurface);
    if (result != VK_SUCCESS || this->_windowSurface == VK_NULL_HANDLE)
      throwError(result, "DisplaySurface: creation failed");
  }

  // Destroy output surface
  DisplaySurface::~DisplaySurface() noexcept {
    if (this->_windowSurface != VK_NULL_HANDLE)
      vkDestroySurfaceKHR(this->_renderer->vkInstance(), this->_windowSurface, nullptr);
  }

  DisplaySurface& DisplaySurface::operator=(DisplaySurface&& rhs) noexcept {
    if (this->_windowSurface != VK_NULL_HANDLE)
      vkDestroySurfaceKHR(this->_renderer->vkInstance(), this->_windowSurface, nullptr);
    this->_renderer = rhs._renderer;
    this->_windowSurface = rhs._windowSurface;
    rhs._windowSurface = VK_NULL_HANDLE;
    return *this;
  }

  // ---

  // Create swap-chain resource for existing renderer
  void SwapChain::_createSwapChain(uint32_t clientWidth, uint32_t clientHeight, VkSwapchainKHR oldSwapchain) { // throws
    if (this->_windowSurface == VK_NULL_HANDLE)
      throw std::invalid_argument("SwapChain: NULL window surface");

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->_renderer->device(), this->_windowSurface, &capabilities);
    if (result != VK_SUCCESS)
      throwError(result, "SwapChain: can't read capabilities");
    
    // find resources + apply constraints
    this->_presentQueueArrayIndex = _findPresentQueueArrayIndex(*(this->_renderer), this->_windowSurface); // throws
    VkSurfaceFormatKHR surfaceFormat = __findSwapChainFormat(this->_renderer->device(), this->_windowSurface, this->_backBufferFormat); // throws
    this->_backBufferFormat = surfaceFormat.format;

    _constrainSwapChainExtents(capabilities, clientWidth, clientHeight);
    this->_pixelSize = _toPixelSize(clientWidth, clientHeight);

    if (capabilities.minImageCount != UINT32_MAX && capabilities.minImageCount > this->_framebufferCount)
      this->_framebufferCount = capabilities.minImageCount;
    else if (capabilities.maxImageCount != 0 && capabilities.maxImageCount < this->_framebufferCount)
      this->_framebufferCount = capabilities.maxImageCount;
    auto presentMode = _findPresentMode(this->_presentMode, this->_framebufferCount); // throws

    // build swap-chain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->_windowSurface;
    createInfo.minImageCount = this->_framebufferCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent.width = clientWidth;
    createInfo.imageExtent.height = clientHeight;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    const auto* cmdQueues = this->_renderer->commandQueues().value;
    uint32_t cmdQueueIndices[2] = { cmdQueues[0].familyIndex, cmdQueues[_presentQueueArrayIndex].familyIndex };
    createInfo.pQueueFamilyIndices = &cmdQueueIndices[0];
    if (this->_presentQueueArrayIndex != 0) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 1;
    }

    if ((this->_flags & SwapChain::OutputFlag::shaderInput) == true) {
      createInfo.flags = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;
      createInfo.imageUsage |= (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    }
    if ((this->_flags & SwapChain::OutputFlag::stereo) == true) {
      if (capabilities.maxImageArrayLayers < 2)
        throw std::invalid_argument("SwapChain: stereo image (2 layers) not supported");
      createInfo.imageArrayLayers = 2;
    }
    else
      createInfo.imageArrayLayers = 1;

    result = vkCreateSwapchainKHR(this->_renderer->context(), &createInfo, nullptr, &(this->_swapChain));
    if (result != VK_SUCCESS)
      throwError(result, "SwapChain: creation failed");
  }
  
  // ---
  
  // Create/refresh swap-chain render-target view
  void SwapChain::_createOrRefreshTargetViews() { // throws
    // retrieve swap-chain buffer images
    uint32_t imageCount = 0;
    VkResult result = vkGetSwapchainImagesKHR(this->_renderer->context(), this->_swapChain, &imageCount, nullptr);
    if (result != VK_SUCCESS || imageCount == 0)
      throwError(result, "SwapChain: failed to count buffer images");

    this->_bufferImages = DynamicArray<VkImage>(imageCount);
    result = vkGetSwapchainImagesKHR(this->_renderer->context(), this->_swapChain, &imageCount, this->_bufferImages.value);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
      throwError(result, "SwapChain: failed to obtain buffer images");

    // retrieve swap-chain target image views
    this->_renderTargetViews = DynamicArray<VkImageView>(imageCount);
    memset(this->_renderTargetViews.value, 0, imageCount*sizeof(VkImageView));
    uint32_t layerCount = ((this->_flags & SwapChain::OutputFlag::stereo) == true) ? 2 : 1;
    
    for (uint32_t i = 0; i < imageCount; ++i) {
      this->_renderTargetViews.value[i] = __createBufferView(this->_renderer->context(),
                                                            this->_bufferImages.value[i], this->_backBufferFormat,
                                                            VK_IMAGE_ASPECT_COLOR_BIT, layerCount, 1u, 0);
    }
  }


// -- swap-chain destruction/move -- -------------------------------------------

  // Destroy swap-chain
  void SwapChain::release() noexcept {
    try {
      if (this->_windowSurface != VK_NULL_HANDLE) {
        if (this->_swapChain != VK_NULL_HANDLE) {
          vkDeviceWaitIdle(this->_renderer->context());

          for (size_t i = 0; i < this->_renderTargetViews.length(); ++i) {
            if (this->_renderTargetViews.value[i] != VK_NULL_HANDLE)
              vkDestroyImageView(this->_renderer->context(), this->_renderTargetViews.value[i], nullptr);
          }
          this->_renderTargetViews.clear();
          this->_bufferImages.clear();

          vkDestroySwapchainKHR(this->_renderer->context(), this->_swapChain, nullptr);
        }
        vkDestroySurfaceKHR(this->_renderer->vkInstance(), this->_windowSurface, nullptr);
        this->_windowSurface = VK_NULL_HANDLE;
      }
      this->_renderer = nullptr;
    }
    catch (...) {}
  }

  SwapChain::SwapChain(SwapChain&& rhs) noexcept
    : _swapChain(rhs._swapChain),
      _flags(rhs._flags),
      _pixelSize(rhs._pixelSize),
      _framebufferCount(rhs._framebufferCount),
      _backBufferFormat(rhs._backBufferFormat),
      _renderer(std::move(rhs._renderer)),
      _windowSurface(rhs._windowSurface),
      _bufferImages(std::move(rhs._bufferImages)),
      _renderTargetViews(std::move(rhs._renderTargetViews)),
      _currentImageIndex(rhs._currentImageIndex),
      _presentQueueArrayIndex(rhs._presentQueueArrayIndex),
      _presentQueue(rhs._presentQueue) {
    rhs._swapChain = VK_NULL_HANDLE;
    rhs._renderer = nullptr;
    rhs._windowSurface = VK_NULL_HANDLE;
    rhs._presentQueue = VK_NULL_HANDLE;
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    release();
    this->_swapChain = rhs._swapChain;
    this->_flags = rhs._flags;
    this->_pixelSize = rhs._pixelSize;
    this->_framebufferCount = rhs._framebufferCount;
    this->_backBufferFormat = rhs._backBufferFormat;
    this->_renderer = std::move(rhs._renderer);
    this->_windowSurface = rhs._windowSurface;
    this->_bufferImages = std::move(rhs._bufferImages);
    this->_renderTargetViews = std::move(rhs._renderTargetViews);
    this->_currentImageIndex = rhs._currentImageIndex;
    this->_presentQueueArrayIndex = rhs._presentQueueArrayIndex;
    this->_presentQueue = rhs._presentQueue;
    rhs._swapChain = VK_NULL_HANDLE;
    rhs._renderer = nullptr;
    rhs._windowSurface = VK_NULL_HANDLE;
    rhs._presentQueue = VK_NULL_HANDLE;
    return *this;
  }


// -- swap-chain operations -- -------------------------------------------------

  // Change back-buffer(s) size + refresh color space
  bool SwapChain::resize(uint32_t width, uint32_t height) { // throws
    if (this->_swapChain != VK_NULL_HANDLE && this->_pixelSize != _toPixelSize(width, height)) {
      vkDeviceWaitIdle(this->_renderer->context());

      // clear previous size-specific context
      //...destroy framebuffers
      //...free command buffers
      for (size_t i = 0; i < this->_renderTargetViews.length(); ++i)
        vkDestroyImageView(this->_renderer->context(), this->_renderTargetViews.value[i], nullptr);
      this->_renderTargetViews.clear();
      this->_bufferImages.clear();
      
      // resize swap-chain
      auto oldSwapChain = this->_swapChain;
      _createSwapChain(width, height, oldSwapChain);
      vkDestroySwapchainKHR(this->_renderer->context(), oldSwapChain, nullptr);

      // create/refresh render-target-views
      _createOrRefreshTargetViews();
      return true;
    }
    return false;
  }
  
  // ---
  
  // Throw appropriate exception for 'swap buffers' error
  static void __processSwapError(VkResult result) {
    switch (result) {
      // minor issues -> ignore
      case VK_NOT_READY:
      case VK_EVENT_SET:
      case VK_EVENT_RESET:
      case VK_INCOMPLETE:
      case VK_OPERATION_DEFERRED_KHR:
      case VK_OPERATION_NOT_DEFERRED_KHR: break;
      // device lost
      case VK_ERROR_DEVICE_LOST:
      case VK_ERROR_SURFACE_LOST_KHR:
      case VK_ERROR_OUT_OF_DATE_KHR:
        throw std::domain_error("SwapChain: device lost");
      // invalid option / internal error
      default: throwError(result, "SwapChain: internal error"); break;
    }
  }

  // Swap back-buffer(s) and front-buffer, to display drawn content on screen
  void SwapChain::swapBuffers(DepthStencilView depthBuffer) {
    if (this->_swapChain == VK_NULL_HANDLE)
      return;

    VkResult result = VK_SUCCESS;//TODO
    //...
    //...

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      // don't throw during first call to swapBuffers following a resize (give the app an occasion to call 'resize')
      if (result == VK_ERROR_OUT_OF_DATE_KHR && this->_pixelSize != 0)
        this->_pixelSize = 0;
      else
        __processSwapError(result);
    }
  }

#endif
