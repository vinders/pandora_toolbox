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


// -- sampler params -- --------------------------------------------------------

  SamplerParams::SamplerParams() noexcept {
    _params.minFilter = _params.magFilter = VK_FILTER_LINEAR;
    _params.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    _params.addressModeU = _params.addressModeV = _params.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    _params.maxAnisotropy = 1.f;
    _params.compareOp = VK_COMPARE_OP_ALWAYS;
    _params.minLod = 1.f;
    _params.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  }
  
  // ---
  
  SamplerParams& SamplerParams::setFilter(TextureFilter minifyFilter, TextureFilter magnifyFilter,
                                          TextureFilter mipMapFilter, bool isCompared,
                                          StencilCompare compareOp) noexcept {
    _params.minFilter = (VkFilter)minifyFilter;
    _params.magFilter = (VkFilter)magnifyFilter;
    _params.mipmapMode = (mipMapFilter != TextureFilter::nearest)
                       ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
    _params.anisotropyEnable = VK_FALSE;
    _params.maxAnisotropy = 1.f;
    
    _params.compareEnable = isCompared ? VK_TRUE : VK_FALSE;
    _params.compareOp = (VkCompareOp)compareOp;
    return *this; 
  }
  
  SamplerParams& SamplerParams::setAnisotropicFilter(uint32_t maxAnisotropy, bool isCompared,
                                                     StencilCompare compareOp) noexcept {
    _params.minFilter = _params.magFilter = VK_FILTER_LINEAR;
    _params.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    _params.anisotropyEnable = VK_TRUE;
    _params.maxAnisotropy = static_cast<float>(maxAnisotropy);
    
    _params.compareEnable = isCompared ? VK_TRUE : VK_FALSE;
    _params.compareOp = (VkCompareOp)compareOp;
    return *this; 
  }
  
  // ---
  
  SamplerParams& SamplerParams::borderColor(const ColorFloat rgba[4]) noexcept {
    _params.pNext = nullptr;
    if (rgba && rgba[3] != 0.f) {
      if (rgba[0] == 0.f && rgba[1] == 0.f && rgba[2] == 0.f)
        _params.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
      else if (rgba[0] == 1.f && rgba[1] == 1.f && rgba[2] == 1.f)
        _params.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
      else {
        _params.pNext = &_borderColor;
        _params.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
        _borderColor.format = VK_FORMAT_UNDEFINED;
        memcpy(_borderColor.customBorderColor.float32, rgba, sizeof(float)*4u);
      }
    }
    else
      _params.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    return *this;
  }
  
  SamplerParams& SamplerParams::borderColor(const ColorInt rgba[4]) noexcept {
    _params.pNext = nullptr;
    if (rgba && rgba[3] != 0) {
      if (rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0)
        _params.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      else if (rgba[0] == 255 && rgba[1] == 255 && rgba[2] == 255)
        _params.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
      else {
        _params.pNext = &_borderColor;
        _params.borderColor = VK_BORDER_COLOR_INT_CUSTOM_EXT;
        _borderColor.format = VK_FORMAT_UNDEFINED;
        memcpy(_borderColor.customBorderColor.int32, rgba, sizeof(int32_t)*4u);
      }
    }
    else
      _params.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    return *this;
  }


// -- sampler builder -- -------------------------------------------------------

  // Create sampler state object
  Sampler Sampler::Builder::createSampler(const SamplerParams& params) {
    VkSampler sampler = VK_NULL_HANDLE;
    VkResult result = vkCreateSampler(_device->context(), &(params.descriptor()), nullptr, &sampler);
    if (result != VK_SUCCESS || sampler == VK_NULL_HANDLE)
      throwError(result, "Sampler: creation error");
    return Sampler(std::make_shared<ScopedResource<VkSampler> >(sampler, _device, vkDestroySampler));
  }

#endif
