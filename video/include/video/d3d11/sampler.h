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
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./_private/_shared_resource.h" // includes D3D11
# include "./renderer.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // -- sampler/filter state params --
        
        /// @class SamplerParams
        /// @brief Filter/sampler state configuration (texture filter/wrap, level-of-detail...)
        /// @remarks The same SamplerParams can be used to build multiple SamplerState instances (if needed).
        class SamplerParams final {
        public:
          static constexpr inline float highestLod() noexcept { return 0.f; } ///< Highest / most detailed level-of-detail.
          static constexpr inline float infiniteLod() noexcept { return D3D11_FLOAT32_MAX; } ///< Infinite level-of-detail: only for lodMax.
        
          // ---
        
          /// @brief Create default filter/sampler params: trilinear, clamp to edges, level-of-detail 1 to highest, no depth compared.
          SamplerParams() noexcept;
          
          /// @brief Initialize filter/sampler config params
          /// @param minifyFilter   Filter for minification (downscaling).
          /// @param magnifyFilter  Filter for magnification (upscaling).
          /// @param mipMapFilter   Filter between closest mip levels (mip-map).
          /// @param textureWrapUVW Texture out-of-range addressing modes (dimensions[3]: U/V/W). Default border color (with clampToBorder) is transparent.
          /// @param lodMin         Minimum level-of-detail: lower end of mip-map range (value >= 0, or 'highestLod()' for highest mip level).
          /// @param lodMax         Maximum level-of-detail: upper end of mip-map range
          ///                       (lodMax >= lodMin, or 'highestLod()' for highest mip level, or 'infiniteLod()' to have no limit).
          inline SamplerParams(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter,
                              const TextureWrap textureWrapUVW[3], float lodMin = highestLod(), float lodMax = infiniteLod()) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setFilter(minifyFilter, magnifyFilter, mipMapFilter);
          }
          /// @brief Initialize filter/sampler config params -- depth comparison
          /// @param depthComp  depth compare-mode with existing pixels ("compared" filter)
          inline SamplerParams(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter,
                              const TextureWrap textureWrapUVW[3], float lodMin, float lodMax, StencilCompare depthComp) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setFilter(minifyFilter, magnifyFilter, mipMapFilter, depthComp);
          }
          
          /// @brief Initialize anisotropic filter/sampler config params
          /// @param maxAnisotropy  Max anisotropy level (clamping value: 1 to 'maxSamplerAnisotropy()')
          /// @param textureWrapUVW Texture out-of-range addressing modes (dimensions[3]: U/V/W). Default border color (with clampToBorder) is transparent.
          /// @param lodMin         Minimum level-of-detail: lower end of mip-map range (value >= 0, or 'highestLod()' for highest mip level).
          /// @param lodMax         Maximum level-of-detail: upper end of mip-map range
          ///                       (lodMax >= lodMin, or 'highestLod()' for highest mip level, or 'infiniteLod()' to have no limit).
          inline SamplerParams(uint32_t maxAnisotropy, const TextureWrap textureWrapUVW[3], float lodMin = highestLod(), float lodMax = infiniteLod()) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setAnisotropicFilter(maxAnisotropy);
          }
          /// @brief Initialize anisotropic filter/sampler config params -- depth comparison
          /// @param depthComp  depth compare-mode with existing pixels ("compared" filter)
          inline SamplerParams(uint32_t maxAnisotropy, const TextureWrap textureWrapUVW[3], float lodMin, float lodMax, StencilCompare depthComp) noexcept {
            _init(textureWrapUVW, lodMin, lodMax);
            setAnisotropicFilter(maxAnisotropy, depthComp);
          }
          
          SamplerParams(const SamplerParams&) = default;
          SamplerParams& operator=(const SamplerParams&) = default;
          ~SamplerParams() noexcept = default;
          
          
          // -- filter type --
          
          /// @brief Use point/linear filter + set type for minification (downscaling), magnification (upscaling), between mip levels (mip-map) -- no depth comparison
          inline SamplerParams& setFilter(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter) noexcept {
            _params.Filter = _toFilterType(minifyFilter, magnifyFilter, mipMapFilter);
            _params.MaxAnisotropy = 1;
            _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            return *this; 
          }
          /// @brief Use point/linear filter + set type for minification (downscaling), magnification (upscaling), between mip levels (mip-map).
          ///        Enable depth compare-mode with existing pixels ("compared" filter)
          inline SamplerParams& setFilter(TextureFilter minifyFilter, TextureFilter magnifyFilter, TextureFilter mipMapFilter, StencilCompare depthComp) noexcept {
            _params.Filter = (D3D11_FILTER)((int)_toFilterType(minifyFilter, magnifyFilter, mipMapFilter) 
                                          + ((int)D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT - (int)D3D11_FILTER_MIN_MAG_MIP_POINT));
            _params.MaxAnisotropy = 1;
            _params.ComparisonFunc = (D3D11_COMPARISON_FUNC)depthComp;
            return *this; 
          }
          /// @brief Use anisotropic filter + set max anisotropy level (clamping value: 1 to 'maxSamplerAnisotropy()') -- no depth comparison
          inline SamplerParams& setAnisotropicFilter(uint32_t maxAnisotropy) noexcept {
            _params.Filter = D3D11_FILTER_ANISOTROPIC;
            _params.MaxAnisotropy = maxAnisotropy;
            _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            return *this; 
          }
          /// @brief Use anisotropic filter + set max anisotropy level (clamping value: 1 to 'maxSamplerAnisotropy()')
          ///        Enable depth compare-mode with existing pixels ("compared" filter)
          inline SamplerParams& setAnisotropicFilter(uint32_t maxAnisotropy, StencilCompare depthComp) noexcept {
            _params.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
            _params.MaxAnisotropy = maxAnisotropy;
            _params.ComparisonFunc = (D3D11_COMPARISON_FUNC)depthComp;
            return *this; 
          }

          
          // -- texture addressing / mip-map level-of-detail --
          
          /// @brief Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          inline SamplerParams& textureWrap(const TextureWrap textureWrapUVW[3]) noexcept {
            _params.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapUVW[0];
            _params.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapUVW[1];
            _params.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapUVW[2];
            return *this;
          }
          /// @brief Texture out-of-range addressing modes (dimensions[3]: U/V/W).
          inline SamplerParams& textureWrap(TextureWrap textureWrapU, TextureWrap textureWrapV, TextureWrap textureWrapW) noexcept {
            _params.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapU;
            _params.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapV;
            _params.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)textureWrapW;
            return *this;
          }
          /// @brief Border color used with TextureWrap::clampToBorder (color[4]: R/G/B/A) - color range == [0.0 ; 1.0] - NULL to use default (transparent).
          ///        Color should be gamma correct (see Renderer.sRgbToGammaCorrectColor).
          SamplerParams& borderColor(const ColorChannel rgba[4]) noexcept;
          
          /// @brief Set mip-map level-of-detail range
          /// @param lodMin  Minimum level-of-detail: lower end of mip-map range (value above 0, or 'highestLod()' for highest mip level).
          /// @param lodMax  Maximum level-of-detail: upper end of mip-map range
          inline SamplerParams& lod(float lodMin, float lodMax) noexcept { _params.MinLOD = lodMin; _params.MaxLOD = lodMax; return *this; }
          /// @brief Set offset added to calculated mip-map level
          inline SamplerParams& lodBias(float bias) noexcept { _params.MipLODBias = bias; return *this; }
          

          inline D3D11_SAMPLER_DESC& descriptor() noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline const D3D11_SAMPLER_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          
        private:
          void _init(const TextureWrap textureWrapUVW[3], float lodMin, float lodMax) noexcept;
          
          static constexpr inline D3D11_FILTER _toFilterType(TextureFilter minify, TextureFilter magnify, TextureFilter mip) noexcept {
            if (minify == TextureFilter::linear) {
              if (magnify == TextureFilter::linear)
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; // L-L-L / L-L-N
              else
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; // L-N-L / L-N-N
            }
            else {
              if (magnify == TextureFilter::linear)
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; // N-L-L / N-L-N
              else
                return (mip == TextureFilter::linear) ? D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT; // N-N-L / N-N-N
            }
          }
        
        private:
          D3D11_SAMPLER_DESC _params;
        };

        // ---

        /// @class SamplerBuilder
        /// @brief Utility to create samplers/filters
        class SamplerBuilder final {
        public:
          SamplerBuilder(Renderer& renderer) : _device(renderer.device()) {}
          SamplerBuilder(const SamplerBuilder&) = default;
          SamplerBuilder(SamplerBuilder&&) noexcept = default;
          SamplerBuilder& operator=(const SamplerBuilder&) = default;
          SamplerBuilder& operator=(SamplerBuilder&&) noexcept = default;
          ~SamplerBuilder() noexcept = default;

          /// @brief Create sampler filter state - can be used to change sampler filter state when needed (setSamplerState)
          /// @remarks The same SamplerParams can be used to build multiple SamplerState instances (if needed).
          /// @throws runtime_error on creation failure
          SamplerState create(const SamplerParams& params);
          
        private:
          DeviceHandle _device = nullptr;
        };
      }
    }
  }

#endif
