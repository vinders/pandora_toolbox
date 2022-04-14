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
*******************************************************************************/
#pragma once

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cstdint>
# include "./_private/_shared_resource.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        // -- sampler/filter params --
        
        /// @class SamplerParams
        /// @brief Sampler configuration (texture filter/wrap, level-of-detail...)
        /// @remarks The same SamplerParams can be used to build multiple Sampler instances (if needed).
        class SamplerParams final {
        public:
          /// @brief Highest / most detailed level-of-detail.
          static constexpr inline float highestLod() noexcept { return 0.f; }
          /// @brief Infinite level-of-detail: only for lodMax.
          static constexpr inline float infiniteLod() noexcept { return VK_LOD_CLAMP_NONE; }
        
          // ---
        
          /// @brief Create default sampler params: trilinear, clamp to edges,
          ///        level-of-detail [1 ; highest], no depth compared.
          SamplerParams() noexcept;
          
          /// @brief Initialize sampler config params -- point/linear filter
          /// @param minifyFilter   Filter for minification (downscaling).
          /// @param magnifyFilter  Filter for magnification (upscaling).
          /// @param mipMapFilter   Filter between closest mip levels (mip-map).
          /// @param textureWrapUVW Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          ///                       Default border color (with 'clampToBorder') is transparent (float).
          /// @param lodMin         Minimum level-of-detail: lower end of mip-map range
          ///                       (value >= 0 / 'highestLod()' for highest level).
          /// @param lodMax         Maximum level-of-detail: upper end of mip-map range
          ///                       (value >= lodMin / 'highestLod()' -> highest level / 'infiniteLod()' -> no limit).
          /// @param isCompared     Enable comparison against reference value during lookups.
          /// @param compareOp      Comparison function applied to sampled data before filtering.
          inline SamplerParams(TextureFilter minifyFilter, TextureFilter magnifyFilter,
                               TextureFilter mipMapFilter, const TextureWrap textureWrapUVW[3],
                               float lodMin = highestLod(), float lodMax = infiniteLod(),
                               bool isCompared = false, StencilCompare compareOp = StencilCompare::always) noexcept {
            setFilter(minifyFilter, magnifyFilter, mipMapFilter, isCompared, compareOp);
            textureWrap(textureWrapUVW);
            _params.minLod = lodMin;
            _params.maxLod = lodMax;
          }
          /// @brief Initialize sampler config params -- anisotropic filter
          /// @param maxAnisotropy  Max anisotropy level (clamping value: 1 to 'maxSamplerAnisotropy()')
          /// @param textureWrapUVW Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          ///                       Default border color (with clampToBorder) is transparent.
          /// @param lodMin         Minimum level-of-detail: lower end of mip-map range
          ///                       (value >= 0 / 'highestLod()' for highest level).
          /// @param lodMax         Maximum level-of-detail: upper end of mip-map range
          ///                       (value >= lodMin / 'highestLod()' -> highest level / 'infiniteLod()' -> no limit).
          /// @param isCompared     Enable comparison against reference value during lookups.
          /// @param compareOp      Comparison function applied to sampled data before filtering.
          /// @warning To use anisotropic filters, anisotropy support and max level should be verified
          ///          (Renderer::maxSamplerAnisotropy(): above 1 if supported)
          inline SamplerParams(uint32_t maxAnisotropy, const TextureWrap textureWrapUVW[3],
                               float lodMin = highestLod(), float lodMax = infiniteLod(),
                               bool isCompared = false, StencilCompare compareOp = StencilCompare::always) noexcept {
            setAnisotropicFilter(maxAnisotropy, isCompared, compareOp);
            textureWrap(textureWrapUVW);
            _params.minLod = lodMin;
            _params.maxLod = lodMax;
          }
          
          SamplerParams(const SamplerParams& rhs) noexcept
            : _params(rhs._params), _borderColor(rhs._borderColor) {
            if (_params.pNext)
              _params.pNext = &_borderColor;
          }
          SamplerParams& operator=(const SamplerParams& rhs) noexcept {
            _params = rhs._params; _borderColor = rhs._borderColor;
            if (_params.pNext)
              _params.pNext = &_borderColor;
            return *this;
          }
          ~SamplerParams() noexcept = default;
          
          
          // -- filter type --
          
          /// @brief Set point/linear filter: minification (downscaling), magnification (upscaling), mip levels (mip-map)
          SamplerParams& setFilter(TextureFilter minifyFilter, TextureFilter magnifyFilter,
                                   TextureFilter mipMapFilter, bool isCompared = false,
                                   StencilCompare compareOp = StencilCompare::always) noexcept;
          /// @brief Set anisotropic filter: max anisotropy level (clamping value: 1 to 'maxSamplerAnisotropy()')
          /// @warning To use anisotropic filters, anisotropy support and max level should be verified
          ///          (Renderer::maxSamplerAnisotropy(): above 1 if supported)
          SamplerParams& setAnisotropicFilter(uint32_t maxAnisotropy, bool isCompared = false,
                                              StencilCompare compareOp = StencilCompare::always) noexcept;

          
          // -- texture addressing --
          
          /// @brief Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          inline SamplerParams& textureWrap(const TextureWrap textureWrapUVW[3]) noexcept {
            _params.addressModeU = (VkSamplerAddressMode)textureWrapUVW[0];
            _params.addressModeV = (VkSamplerAddressMode)textureWrapUVW[1];
            _params.addressModeW = (VkSamplerAddressMode)textureWrapUVW[2];
            return *this;
          }
          /// @brief Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          inline SamplerParams& textureWrap(TextureWrap textureWrapU, TextureWrap textureWrapV,
                                            TextureWrap textureWrapW) noexcept {
            _params.addressModeU = (VkSamplerAddressMode)textureWrapU;
            _params.addressModeV = (VkSamplerAddressMode)textureWrapV;
            _params.addressModeW = (VkSamplerAddressMode)textureWrapW;
            return *this;
          }
          
          /// @brief Border color used with TextureWrap::clampToBorder (color[4]: R/G/B/A)
          ///        Standard color range == [0.0 ; 1.0] - NULL to use default (transparent).
          ///        Color should be gamma correct (see Renderer.sRgbToGammaCorrectColor).
          /// @warning Custom colors (other than black, white or transparent) require Renderer.deviceFeatures().customBorderColor to be true.
          /// @remarks Normalized/float color should be used if image/attachments format is NOT integer.
          SamplerParams& borderColor(const ColorFloat rgba[4]) noexcept;
          /// @brief Integer border color used with TextureWrap::clampToBorder (color[4]: R/G/B/A)
          ///        Standard color range == [0 ; 255] - NULL to use default (transparent).
          /// @warning Custom colors (other than black, white or transparent) require Renderer.deviceFeatures().customBorderColor to be true.
          /// @remarks Integer color should be used if image/attachments format is signed-integer.
          SamplerParams& borderColor(const ColorInt rgba[4]) noexcept;
          /// @brief Unsigned-integer border color used with TextureWrap::clampToBorder (color[4]: R/G/B/A)
          ///        Standard color range == [0 ; 255] - NULL to use default (transparent).
          /// @warning Custom colors (other than black, white or transparent) require Renderer.deviceFeatures().customBorderColor to be true.
          /// @remarks Unsigned-integer color should be used if image/attachments format is unsigned-integer.
          SamplerParams& borderColor(const ColorUInt rgba[4]) noexcept {
            return borderColor((const ColorInt*)rgba);
          }
          
          
          // -- level-of-detail --
          
          /// @brief Set mip-map level-of-detail range
          /// @param lodMin Minimum level-of-detail: lower end of mip-map range
          ///               (value >= 0 / 'highestLod()' for highest level).
          /// @param lodMax Maximum level-of-detail: upper end of mip-map range
          ///               (value >= lodMin / 'highestLod()' -> highest level / 'infiniteLod()' -> no limit).
          inline SamplerParams& lod(float lodMin, float lodMax) noexcept {
            _params.minLod = lodMin; _params.maxLod = lodMax; return *this;
          }
          /// @brief Set offset added to calculated mip-map level
          inline SamplerParams& lodBias(float bias) noexcept {
            _params.mipLodBias = bias; return *this;
          }
          

          inline VkSamplerCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native Vulkan descriptor
          inline const VkSamplerCreateInfo& descriptor() const noexcept { return this->_params; } ///< Get native Vulkan descriptor
        
        private:
          VkSamplerCreateInfo _params{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
          VkSamplerCustomBorderColorCreateInfoEXT _borderColor{ VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT };
        };
        
        
        // ---------------------------------------------------------------------
        // sampler container/builder
        // ---------------------------------------------------------------------
        
        /// @class Sampler
        /// @brief GPU texture sampler (filter/wrap/level-of-detail state)
        class Sampler final {
        public:
          using Handle = VkSampler;
          
          /// @brief Create usable sampler state object -- reserved for internal use
          /// @remarks Prefer Sampler::Builder for standard usage
          /// @warning Sampler objects must be destroyed BEFORE the associated Renderer instance!
          Sampler(SharedResource<VkSampler> handle) : _handle(std::move(handle)) {}
          
          Sampler() = default; ///< Empty sampler -- not usable (only useful to store variable not immediately initialized)
          Sampler(const Sampler&) = default;
          Sampler(Sampler&&) noexcept = default;
          Sampler& operator=(const Sampler&) = default;
          Sampler& operator=(Sampler&&) noexcept = default;
          ~Sampler() noexcept { release(); }
          inline void release() noexcept { this->_handle = nullptr; } ///< Destroy sampler object

          inline Handle handle() const noexcept { return this->_handle->value(); } ///< Get native sampler handle
          /// @brief Get pointer to native sampler handle (usable as array of 1 item)
          inline const Handle* handlePtr() const noexcept { return _handle->ptr(); }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (this->_handle == nullptr); }


          // -- create samplers --
          
          /// @class Sampler.Builder
          /// @brief Sampler state creator
          class Builder final {
          public:
            /// @brief Initialize sampler builder
            Builder(DeviceResourceManager device) : _device(device) {}

            Builder() noexcept = default; ///< Empty buffer -- not usable (only useful for variables with deferred init)
            Builder(const Builder&) = delete;
            Builder(Builder&&) noexcept = default;
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&&) noexcept = default;
            ~Builder() noexcept = default;

            /// @brief Create sampler state object
            /// @remarks The same SamplerParams can be modified and used to build multiple SamplerState instances (if needed).
            /// @throws runtime_error on failure
            Sampler createSampler(const SamplerParams& params);

          private:
            DeviceResourceManager _device = nullptr;
          };
          
        private:
          SharedResource<VkSampler> _handle = nullptr;
        };
      }
    }
  }

#endif
