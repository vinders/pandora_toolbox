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
          static constexpr inline uint32_t maxAnisotropy() noexcept { return (uint32_t)D3D11_MAX_MAXANISOTROPY; } ///< Max anisotropy level value (usually 8 or 16)
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
          /// @param maxAnisotropy  Max anisotropy level (clamping value: 1 to 'maxAnisotropy()')
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
          /// @brief Use anisotropic filter + set max anisotropy level (clamping value: 1 to 'maxAnisotropy()') -- no depth comparison
          inline SamplerParams& setAnisotropicFilter(uint32_t maxAnisotropy) noexcept {
            _params.Filter = D3D11_FILTER_ANISOTROPIC;
            _params.MaxAnisotropy = maxAnisotropy;
            _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            return *this; 
          }
          /// @brief Use anisotropic filter + set max anisotropy level (clamping value: 1 to 'maxAnisotropy()')
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


        // -- texture params - 1D --
        
        /// @class Texture1DParams
        /// @brief Texture1D/Texture1DArray resource configuration (useful for gradients, UI decorations, stats, and particle effects)
        /// @remarks The same Texture1DParams can (and should) be used to build multiple Texture1D/Texture1DArray instances (if needed).
        class Texture1DParams final {
        public:
          /// @brief Initialize texture params
          /// @param width      Texel width of the texture (can't be 0).
          /// @param format     Color format of each texture pixel.
          /// @param arraySize  Number of sub-textures in texture array (if value > 1, params only usable to create Texture1DArray instances)
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @param usageType  Memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///                   With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///                   Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///                   (staging content is typically copied to/from other 'staticGpu' texture).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          Texture1DParams(uint32_t width, DataFormat format = DataFormat::rgba8_sRGB, uint32_t arraySize = 1u, 
                          uint32_t mipLevels = 1u, uint32_t mostDetailedViewedMip = 0, ResourceUsage usageType = ResourceUsage::staticGpu) noexcept;

          Texture1DParams(const Texture1DParams&) = default;
          Texture1DParams& operator=(const Texture1DParams&) = default;
          ~Texture1DParams() noexcept = default;
          
          inline Texture1DParams& size(uint32_t width) noexcept { _params.Width = (UINT)width; return *this; } ///< Set texture width
          inline Texture1DParams& texelFormat(DataFormat format) noexcept { ///< Set color format of each texture pixel
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams); return *this;
          }
          
          /// @brief Set number of sub-textures: array length + mip-map level
          /// @param arraySize  Number of sub-textures in texture array (if value > 1, params only usable to create Texture1DArray instances)
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          Texture1DParams& arrayLength(uint32_t arraySize, uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept;
          static uint32_t maxMipLevels(uint32_t width) noexcept; ///< Compute max available mip level (1 + floor(log2(max_dimension)))

          /// @brief Choose memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///        With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///        Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///        (staging content is typically copied to/from other 'staticGpu' texture).
          inline Texture1DParams& usage(ResourceUsage type) noexcept { _setTextureUsage(type, _params); return *this; }
          /// @brief Texture resource data can be shared between multiple Renderer/devices (should only be enabled for multi-device rendering).
          inline Texture1DParams& flags(bool isShared) noexcept { _params.MiscFlags = isShared ? (UINT)D3D11_RESOURCE_MISC_SHARED : 0; return *this; }
          
          
          inline D3D11_TEXTURE1D_DESC& descriptor() noexcept { return this->_params; }             ///< Get native Direct3D descriptor
          inline const D3D11_TEXTURE1D_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() noexcept { return this->_viewParams; }             ///< Get native Direct3D view descriptor
          inline const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() const noexcept { return this->_viewParams; } ///< Get native Direct3D view descriptor
          inline uint32_t texelBytes() const noexcept { return this->_texelBytes; }
          
        private:
          D3D11_TEXTURE1D_DESC _params;
          D3D11_SHADER_RESOURCE_VIEW_DESC _viewParams;
          uint32_t _texelBytes = 0;
        };
        
        
        // -- texture params - 2D --
        
        /// @class Texture2DParams
        /// @brief Texture2D/Texture2DArray resource configuration (texture image for polygons, sprites, backgrounds, fonts...)
        /// @remarks The same Texture2DParams can (and should) be used to build multiple Texture2D/Texture2DArray instances (if needed).
        class Texture2DParams final {
        public:
          /// @brief Initialize texture params
          /// @param width      Texel width of the texture (can't be 0).
          /// @param height     Texel height of one face of the cube (can't be 0).
          /// @param format     Color format of each texture pixel.
          /// @param arraySize  Number of sub-textures in texture array (if value > 1, params only usable to create Texture2DArray instances)
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @param usageType  Memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///                   With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///                   Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///                   (staging content is typically copied to/from other 'staticGpu' texture).
          /// @param sampleCount  Sample count for multisampling (anti-aliasing). Use 1 to disable multisampling.
          ///                     Call Renderer.is{Color/Depth/Stencil}SampleCountAvailable to make sure the value is supported.
          /// @warning With 'dynamicCpu' usage or with 'sampleCount' > 1, mipLevels value must always be '1' (0 not supported).
          Texture2DParams(uint32_t width, uint32_t height, DataFormat format = DataFormat::rgba8_sRGB,
                          uint32_t arraySize = 1u, uint32_t mipLevels = 1u, uint32_t mostDetailedViewedMip = 0,
                          ResourceUsage usageType = ResourceUsage::staticGpu, uint32_t sampleCount = 1u) noexcept;
          
          Texture2DParams(const Texture2DParams&) = default;
          Texture2DParams& operator=(const Texture2DParams&) = default;
          ~Texture2DParams() noexcept = default;
          
          inline Texture2DParams& size(uint32_t width, uint32_t height) noexcept {///< Set texture width/height
            _params.Width=(UINT)width; _params.Height=(UINT)height;
            return *this;
          }
          inline Texture2DParams& texelFormat(DataFormat format) noexcept { ///< Set color format of each texture pixel
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams); return *this;
          }
          /// @brief Set sample count for multisampling (or 1 to disable it)
          /// @warning Changing the sample count will reset mip level params.
          Texture2DParams& sampleCount(uint32_t count) noexcept;
          
          /// @brief Set number of sub-textures: array length + mip-map level
          /// @param arraySize  Number of sub-textures in texture array (if value > 1, params only usable to create Texture2DArray instances)
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          Texture2DParams& arrayLength(uint32_t arraySize, uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept;
          static uint32_t maxMipLevels(uint32_t width, uint32_t height) noexcept; ///< Compute max available mip level (1 + floor(log2(max_dimension)))

          /// @brief Choose memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///        With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///        Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///        (staging content is typically copied to/from other 'staticGpu' texture).
          inline Texture2DParams& usage(ResourceUsage type) noexcept { _setTextureUsage(type, _params); return *this; }
          /// @brief Texture resource data can be shared between multiple Renderer/devices (should only be enabled for multi-device rendering).
          inline Texture2DParams& flags(bool isShared) noexcept { _params.MiscFlags = isShared ? (UINT)D3D11_RESOURCE_MISC_SHARED : 0; return *this; }
          
          
          inline D3D11_TEXTURE2D_DESC& descriptor() noexcept { return this->_params; }             ///< Get native Direct3D descriptor
          inline const D3D11_TEXTURE2D_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() noexcept { return this->_viewParams; }             ///< Get native Direct3D view descriptor
          inline const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() const noexcept { return this->_viewParams; } ///< Get native Direct3D view descriptor
          inline uint32_t texelBytes() const noexcept { return this->_texelBytes; }
          
        private:
          D3D11_TEXTURE2D_DESC _params;
          D3D11_SHADER_RESOURCE_VIEW_DESC _viewParams;
          uint32_t _texelBytes = 0;
        };
        
        
        // -- texture params - 2D/cube --
        
        /// @class TextureCube2DParams
        /// @brief Texture2DArray cube resource configuration (texture image for cubes and sky-boxes)
        /// @remarks The same TextureCube2DParams can (and should) be used to build multiple Texture2DArray instances (if needed).
        class TextureCube2DParams final {
        public:
          /// @brief Initialize texture params
          /// @param width      Texel width of one face of the cube (can't be 0).
          /// @param height     Texel height of one face of the cube (can't be 0).
          /// @param format     Color format of each texture pixel.
          /// @param nbCubes    Number of cubes in texture-cube array.
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @param usageType  Memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///                   With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///                   Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///                   (staging content is typically copied to/from other 'staticGpu' texture).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          TextureCube2DParams(uint32_t width, uint32_t height, DataFormat format = DataFormat::rgba8_sRGB,
                              uint32_t nbCubes = 1u, uint32_t mipLevels = 1u, uint32_t mostDetailedViewedMip = 0,
                              ResourceUsage usageType = ResourceUsage::staticGpu) noexcept;
          
          TextureCube2DParams(const TextureCube2DParams&) = default;
          TextureCube2DParams& operator=(const TextureCube2DParams&) = default;
          ~TextureCube2DParams() noexcept = default;
          
          inline TextureCube2DParams& size(uint32_t width, uint32_t height) noexcept {///< Set cube face width/height
            _params.Width=(UINT)width; _params.Height=(UINT)height;
            return *this;
          }
          inline TextureCube2DParams& texelFormat(DataFormat format) noexcept { ///< Set color format of each texture pixel
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams); return *this;
          }
          
          /// @brief Set number of sub-textures: array length + mip-map level
          /// @param nbCubes    Number of cubes in texture-cube array.
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          TextureCube2DParams& arrayLength(uint32_t nbCubes, uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept;
          /// @brief Compute max available mip level (1 + floor(log2(max_dimension)))
          static inline uint32_t maxMipLevels(uint32_t width, uint32_t height) noexcept { return Texture2DParams::maxMipLevels(width, height); }

          /// @brief Choose memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///        With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///        Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///        (staging content is typically copied to/from other 'staticGpu' texture).
          inline TextureCube2DParams& usage(ResourceUsage type) noexcept { _setTextureUsage(type, _params); return *this; }
          /// @brief Texture resource data can be shared between multiple Renderer/devices (should only be enabled for multi-device rendering).
          inline TextureCube2DParams& flags(bool isShared) noexcept {
            _params.MiscFlags = isShared ? (UINT)(D3D11_RESOURCE_MISC_SHARED|D3D11_RESOURCE_MISC_TEXTURECUBE) : D3D11_RESOURCE_MISC_TEXTURECUBE;
            return *this;
          }
          
          
          inline D3D11_TEXTURE2D_DESC& descriptor() noexcept { return this->_params; }             ///< Get native Direct3D descriptor
          inline const D3D11_TEXTURE2D_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() noexcept { return this->_viewParams; }             ///< Get native Direct3D view descriptor
          inline const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() const noexcept { return this->_viewParams; } ///< Get native Direct3D view descriptor
          inline uint32_t texelBytes() const noexcept { return this->_texelBytes; }
          
        private:
          D3D11_TEXTURE2D_DESC _params;
          D3D11_SHADER_RESOURCE_VIEW_DESC _viewParams;
          uint32_t _texelBytes = 0;
        };
        
        
        // -- texture params - 3D --
        
        /// @class Texture3DParams
        /// @brief Texture3D resource configuration(useful for light/glow effects and particle effects, or for animated 2D textures)
        /// @remarks The same Texture3DParams can (and should) be used to build multiple Texture3D instances (if needed).
        class Texture3DParams final {
        public:
          /// @brief Initialize texture params
          /// @param width      Texel width of the texture (can't be 0).
          /// @param height     Texel height of the texture (can't be 0).
          /// @param depth      Texel depth of the texture (can't be 0).
          /// @param format     Color format of each texture pixel.
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @param usageType  Memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///                   With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///                   Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///                   (staging content is typically copied to/from other 'staticGpu' texture).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          Texture3DParams(uint32_t width, uint32_t height, uint32_t depth, DataFormat format = DataFormat::rgba8_sRGB, 
                          uint32_t mipLevels = 1u, uint32_t mostDetailedViewedMip = 0, ResourceUsage usageType = ResourceUsage::staticGpu) noexcept;
          
          Texture3DParams(const Texture3DParams&) = default;
          Texture3DParams& operator=(const Texture3DParams&) = default;
          ~Texture3DParams() noexcept = default;
          
          inline Texture3DParams& size(uint32_t width, uint32_t height, uint32_t depth) noexcept { ///< Set texture width/height/depth
            _params.Width=(UINT)width; _params.Height=(UINT)height; _params.Depth=(UINT)depth; return *this;
          }
          inline Texture3DParams& texelFormat(DataFormat format) noexcept { ///< Set color format of each texture pixel
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams); return *this;
          }
          
          /// @brief Set number of generated mip-map levels + most detailed mip to use.
          /// @param mipLevels              Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          /// @warning With 'dynamicCpu' usage, mipLevels value must always be '1' (0 not supported).
          inline Texture3DParams& mips(uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept {
            _params.MipLevels = (UINT)mipLevels;
            _viewParams.Texture3D.MostDetailedMip = (UINT)mostDetailedViewedMip;
            return *this;
          }
          static uint32_t maxMipLevels(uint32_t width, uint32_t height, uint32_t depth) noexcept; ///< Compute max available mip level (1 + floor(log2(max_dimension)))

          /// @brief Choose memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///        With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///        Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///        (staging content is typically copied to/from other 'staticGpu' texture).
          inline Texture3DParams& usage(ResourceUsage type) noexcept { _setTextureUsage(type, _params); return *this; }
          /// @brief Texture resource data can be shared between multiple Renderer/devices (should only be enabled for multi-device rendering).
          inline Texture3DParams& flags(bool isShared) noexcept { _params.MiscFlags = isShared ? (UINT)D3D11_RESOURCE_MISC_SHARED : 0; return *this; }
          
          
          inline D3D11_TEXTURE3D_DESC& descriptor() noexcept { return this->_params; }             ///< Get native Direct3D descriptor
          inline const D3D11_TEXTURE3D_DESC& descriptor() const noexcept { return this->_params; } ///< Get native Direct3D descriptor
          inline D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() noexcept { return this->_viewParams; }             ///< Get native Direct3D view descriptor
          inline const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor() const noexcept { return this->_viewParams; } ///< Get native Direct3D view descriptor
          inline uint32_t texelBytes() const noexcept { return this->_texelBytes; }
          
        private:
          D3D11_TEXTURE3D_DESC _params;
          D3D11_SHADER_RESOURCE_VIEW_DESC _viewParams;
          uint32_t _texelBytes = 0;
        };
        
        
        // ---------------------------------------------------------------------
        // texture containers
        // ---------------------------------------------------------------------
        
        // -- single-texture containers --

        /// @class Texture1D
        /// @brief One-dimensional texture data/view container, built from Texture1DParams
        /// @remarks Common usages: gradients, UI decorations, stats, particle effects...
        /// @warning For texture arrays, prefer Texture1DArray instead (to be able to get array size).
        class Texture1D {
        public:
          /// @brief Create texture resource and view from params
          /// @warning - 'initData' is required with 'immutable' usage.
          ///          - 'initData' is an array: if mipLevels > 1 or arraySize > 1, it must contain one index for each sub-resource
          ///            (ordered from most detailed mip to smallest, then next array item, and so on).
          ///          - To only initialize one level or item, set 'initData' to NULL and use TextureWriter (note: mip levels can be generated with D3DXFilterTexture).
          ///          - The length of each entry in 'initData' must be rowBytes (adjust for each mip level).
          /// @throws runtime_error on creation failure
          inline Texture1D(Renderer& renderer, const Texture1DParams& params, const uint8_t** initData = nullptr)
            : Texture1D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData) {}

          /// @brief Store texture resource and view created with external tool (such as DDSTextureLoader in DirectXTK)
          inline Texture1D(TextureHandle texture, TextureView resourceView, uint32_t rowBytes, uint8_t mipLevels,
                           ResourceUsage usage = ResourceUsage::staticGpu)
            : _texture((TextureHandle1D)texture), _resourceView(resourceView), _rowBytes(rowBytes), _mipLevels(mipLevels) {
            if (usage == ResourceUsage::dynamicCpu)   _writeMode = D3D11_MAP_WRITE_DISCARD;
            else if (usage == ResourceUsage::staging) _writeMode = D3D11_MAP_WRITE;
          }
          
          Texture1D() noexcept = default;
          Texture1D(const Texture1D&) = delete;
          Texture1D(Texture1D&& rhs) noexcept
            : _texture(rhs._texture), _resourceView(rhs._resourceView), _writeMode(rhs._writeMode),
              _rowBytes(rhs._rowBytes), _mipLevels(rhs._mipLevels) { rhs._texture = nullptr; rhs._resourceView = nullptr; }
          Texture1D& operator=(const Texture1D&) = delete;
          Texture1D& operator=(Texture1D&& rhs) noexcept {
            release();
            _texture=rhs._texture; _resourceView=rhs._resourceView;
            _writeMode=rhs._writeMode; _rowBytes=rhs._rowBytes; _mipLevels=rhs._mipLevels;
            rhs._texture = nullptr; rhs._resourceView = nullptr;
            return *this;
          }
          inline ~Texture1D() noexcept { release(); } // no need for virtual (only base class releases handles, not child "array" classes)
          void release() noexcept; ///< Destroy resources
          
          inline TextureHandle1D handle() const noexcept { return this->_texture; } ///< Get Direct3D texture handle
          inline TextureView resourceView() const noexcept { return this->_resourceView; } ///< Get texture resource view handle (or NULL with 'staging' usage)
        
          inline bool isEmpty() const noexcept { return (this->_texture == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          inline uint32_t rowBytes() const noexcept { return this->_rowBytes; }        ///< Get byte size of one row of texels
          inline uint8_t mipLevels() const noexcept { return this->_mipLevels; }       ///< Get number of mip levels per sub-texture
          inline D3D11_MAP writeMode() const noexcept { return this->_writeMode; }     ///< Get mappable write mode (or 0 if static/immutable)
          
        protected:
          Texture1D(DeviceHandle device, const D3D11_TEXTURE1D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor, 
                    uint32_t texelBytes, const uint8_t** initData);
        protected:
          TextureHandle1D _texture = nullptr;
          TextureView _resourceView = nullptr;
          D3D11_MAP _writeMode = (D3D11_MAP)0;
          uint32_t _rowBytes = 0;
          uint8_t _mipLevels = 0;
        };
        
        /// @class Texture2D
        /// @brief Two-dimensional texture data/view container, built from Texture2DParams (TextureCube2DParams must be used with Texture2DArray instead)
        /// @remarks Common usages: texture for polygons, sprites, backgrounds, fonts...
        /// @warning For texture arrays, prefer Texture2DArray instead (to be able to get array size).
        class Texture2D {
        public:
          /// @brief Create texture resource and view from params
          /// @warning - 'initData' is required with 'immutable' usage.
          ///          - 'initData' is an array: if mipLevels > 1 or arraySize > 1, it must contain one index for each sub-resource
          ///            (ordered from most detailed mip to smallest, then next array item, and so on).
          ///          - To only initialize one level or item, set 'initData' to NULL and use TextureWriter (note: mip levels can be generated with D3DXFilterTexture).
          ///          - The length of each entry in 'initData' must be rowBytes*height (adjust for each mip level).
          /// @throws runtime_error on creation failure
          inline Texture2D(Renderer& renderer, const Texture2DParams& params, const uint8_t** initData = nullptr)
            : Texture2D(renderer, params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData) {}

          /// @brief Store texture resource and view created with external tool (such as DDSTextureLoader in DirectXTK)
          inline Texture2D(TextureHandle texture, TextureView resourceView, uint32_t rowBytes, uint32_t height,
                           uint8_t mipLevels, ResourceUsage usage = ResourceUsage::staticGpu)
            : _texture((TextureHandle2D)texture), _resourceView(resourceView), _rowBytes(rowBytes), _height(height), _mipLevels(mipLevels) {
            if (usage == ResourceUsage::dynamicCpu)   _writeMode = D3D11_MAP_WRITE_DISCARD;
            else if (usage == ResourceUsage::staging) _writeMode = D3D11_MAP_WRITE;
          }
          
          Texture2D() noexcept = default;
          Texture2D(const Texture2D&) = delete;
          Texture2D(Texture2D&& rhs) noexcept
            : _texture(rhs._texture), _resourceView(rhs._resourceView), _writeMode(rhs._writeMode),
              _rowBytes(rhs._rowBytes), _height(rhs._height), _mipLevels(rhs._mipLevels) { rhs._texture = nullptr; rhs._resourceView = nullptr; }
          Texture2D& operator=(const Texture2D&) = delete;
          Texture2D& operator=(Texture2D&& rhs) noexcept {
            release();
            _texture=rhs._texture; _resourceView=rhs._resourceView;
            _writeMode=rhs._writeMode; _rowBytes=rhs._rowBytes; _height=rhs._height; _mipLevels=rhs._mipLevels;
            rhs._texture = nullptr; rhs._resourceView = nullptr;
            return *this;
          }
          inline ~Texture2D() noexcept { release(); } // no need for virtual (only base class releases handles, not child "array" classes)
          void release() noexcept; ///< Destroy resources
          
          inline TextureHandle2D handle() const noexcept { return this->_texture; } ///< Get Direct3D texture handle
          inline TextureView resourceView() const noexcept { return this->_resourceView; } ///< Get texture resource view handle (or NULL with 'staging' usage)
        
          inline bool isEmpty() const noexcept { return (this->_texture == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          inline uint32_t rowBytes() const noexcept { return this->_rowBytes; }        ///< Get byte size of one row of texels
          inline uint32_t height() const noexcept { return this->_height; }            ///< Get number of rows per texture (total rows if not an array)
          inline uint8_t mipLevels() const noexcept { return this->_mipLevels; }       ///< Get number of mip levels per sub-texture
          inline D3D11_MAP writeMode() const noexcept { return this->_writeMode; }     ///< Get mappable write mode (or 0 if static/immutable)
        
        protected:
          Texture2D(Renderer& renderer, const D3D11_TEXTURE2D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor,
                    uint32_t texelBytes, const uint8_t** initData);
        protected:
          TextureHandle2D _texture = nullptr;
          TextureView _resourceView = nullptr;
          D3D11_MAP _writeMode = (D3D11_MAP)0;
          uint32_t _rowBytes = 0;
          uint32_t _height = 0;
          uint8_t _mipLevels = 0;
        };

        /// @class Texture3D
        /// @brief Three-dimensional texture data/view container, built from Texture3DParams
        /// @remarks Common usages: light/glow effects, particle effects, animated 2D textures...
        class Texture3D final {
        public:
          /// @brief Create texture resource and view from params
          /// @warning - 'initData' is required with 'immutable' usage.
          ///          - 'initData' is an array: if mipLevels > 1, it must contain one index for each sub-resource (ordered from most detailed mip to smallest).
          ///          - To only initialize one item, set 'initData' to NULL and use TextureWriter (note: mip levels can be generated with D3DXFilterTexture).
          ///          - The length of each entry in 'initData' must be rowBytes*height*depth (adjust for each mip level).
          /// @throws runtime_error on creation failure
          inline Texture3D(Renderer& renderer, const Texture3DParams& params, const uint8_t** initData = nullptr)
            : Texture3D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData) {}

          /// @brief Store texture resource and view created with external tool (such as DDSTextureLoader in DirectXTK)
          inline Texture3D(TextureHandle texture, TextureView resourceView, uint32_t rowBytes, uint32_t height,
                           uint32_t depth, uint8_t mipLevels, ResourceUsage usage = ResourceUsage::staticGpu)
            : _texture((TextureHandle3D)texture), _resourceView(resourceView),
              _rowBytes(rowBytes), _height(height), _depth(depth), _mipLevels(mipLevels) {
            if (usage == ResourceUsage::dynamicCpu)   _writeMode = D3D11_MAP_WRITE_DISCARD;
            else if (usage == ResourceUsage::staging) _writeMode = D3D11_MAP_WRITE;
          }
          
          Texture3D() noexcept = default;
          Texture3D(const Texture3D&) = delete;
          Texture3D(Texture3D&& rhs) noexcept
            : _texture(rhs._texture), _resourceView(rhs._resourceView), _writeMode(rhs._writeMode), _rowBytes(rhs._rowBytes),
              _height(rhs._height), _depth(rhs._depth), _mipLevels(rhs._mipLevels) { rhs._texture = nullptr; rhs._resourceView = nullptr; }
          Texture3D& operator=(const Texture3D&) = delete;
          Texture3D& operator=(Texture3D&& rhs) noexcept {
            release();
            _texture=rhs._texture; _resourceView=rhs._resourceView;
            _writeMode=rhs._writeMode; _rowBytes=rhs._rowBytes; _height=rhs._height; _depth=rhs._depth; _mipLevels=rhs._mipLevels;
            rhs._texture = nullptr; rhs._resourceView = nullptr;
            return *this;
          }
          inline ~Texture3D() noexcept { release(); }
          void release() noexcept; ///< Destroy resources
          
          inline TextureHandle3D handle() const noexcept { return this->_texture; } ///< Get Direct3D texture handle
          inline TextureView resourceView() const noexcept { return this->_resourceView; } ///< Get texture resource view handle (or NULL with 'staging' usage)
        
          inline bool isEmpty() const noexcept { return (this->_texture == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          inline uint32_t rowBytes() const noexcept { return this->_rowBytes; }        ///< Get byte size of one row of texels
          inline uint32_t height() const noexcept { return this->_height; }            ///< Get number of rows per depth-slice
          inline uint32_t depth() const noexcept { return this->_depth; }              ///< Get number of depth-slices
          inline uint8_t mipLevels() const noexcept { return this->_mipLevels; }       ///< Get number of mip levels
          inline D3D11_MAP writeMode() const noexcept { return this->_writeMode; }     ///< Get mappable write mode (or 0 if static/immutable)
        
        private:
          Texture3D(DeviceHandle device, const D3D11_TEXTURE3D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor,
                    uint32_t texelBytes, const uint8_t** initData);
        private:
          TextureHandle3D _texture = nullptr;
          TextureView _resourceView = nullptr;
          D3D11_MAP _writeMode = (D3D11_MAP)0;
          uint32_t _rowBytes = 0;
          uint32_t _height = 0;
          uint32_t _depth = 0;
          uint8_t _mipLevels = 0;
        };
        
        
        // -- texture array containers --
        
        /// @class Texture1DArray
        /// @brief Array container of one-dimensional textures/views, built from Texture1DParams
        /// @remarks Common usages: multi-layer gradients, stats, particle effects...
        class Texture1DArray final : public Texture1D {
        public:
          /// @brief Create texture resource array and view from params
          /// @warning - 'initData' is required with 'immutable' usage.
          ///          - 'initData' is an array: if mipLevels > 1 or arraySize > 1, it must contain one index for each sub-resource
          ///            (ordered from most detailed mip to smallest, then next array item, and so on).
          ///          - To only initialize one level or item, set 'initData' to NULL and use TextureWriter (note: mip levels can be generated with D3DXFilterTexture).
          ///          - The length of each entry in 'initData' must be rowBytes (adjust for each mip level).
          /// @throws runtime_error on creation failure
          inline Texture1DArray(Renderer& renderer, const Texture1DParams& params, const uint8_t** initData = nullptr)
            : Texture1D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData),
              _arraySize((uint8_t)params.descriptor().ArraySize) {}
          
          Texture1DArray() noexcept = default;
          Texture1DArray(const Texture1DArray&) = delete;
          Texture1DArray(Texture1DArray&& rhs) noexcept : Texture1D(std::move(rhs)), _arraySize(rhs._arraySize) {}
          Texture1DArray& operator=(const Texture1DArray&) = delete;
          Texture1DArray& operator=(Texture1DArray&& rhs) noexcept {
            Texture1D::operator=(std::move(rhs));
            _arraySize = rhs._arraySize;
            return *this;
          }
          ~Texture1DArray() noexcept = default;
          
          inline uint8_t arraySize() const noexcept { return this->_arraySize; } ///< Get number of sub-textures in array
          
        private:
          uint8_t _arraySize = 0;
        };
        
        /// @class Texture2DArray
        /// @brief Array container of two-dimensional textures/views, built from Texture2DParams/TextureCube2DParams
        /// @remarks Common usages: multi-layer texture for polygons, texture blending, textured cubes, sky-boxes...
        class Texture2DArray final : public Texture2D {
        public:
          /// @brief Create texture resource array and view from params - simple array of 2D textures
          /// @warning - 'initData' is required with 'immutable' usage.
          ///          - 'initData' is an array: if mipLevels > 1 or arraySize > 1, it must contain one index for each sub-resource
          ///            (ordered from most detailed mip to smallest, then next array item, and so on).
          ///          - To only initialize one level or item, set 'initData' to NULL and use TextureWriter (note: mip levels can be generated with D3DXFilterTexture).
          ///          - The length of each entry in 'initData' must be rowBytes*height (adjust for each mip level).
          /// @throws runtime_error on creation failure
          inline Texture2DArray(Renderer& renderer, const Texture2DParams& params, const uint8_t** initData = nullptr)
            : Texture2D(renderer, params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData),
              _arraySize((uint8_t)params.descriptor().ArraySize) {}
          /// @brief Create texture resource array and view from params - 2D cube texture (or array of 2D cubes)
          /// @warning - 'initData' is required with 'immutable' usage.
          ///          - 'initData' is an array: if mipLevels > 1 or arraySize > 1, it must contain one index for each sub-resource
          ///            (ordered from most detailed mip to smallest, then next array item, and so on).
          ///          - To only initialize one level or item, set 'initData' to NULL and use TextureWriter (note: mip levels can be generated with D3DXFilterTexture).
          ///          - The length of each entry in 'initData' must be rowBytes*height (adjust for each mip level).
          /// @throws runtime_error on creation failure
          inline Texture2DArray(Renderer& renderer, const TextureCube2DParams& params, const uint8_t** initData = nullptr)
            : Texture2D(renderer, params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData),
              _arraySize((uint8_t)params.descriptor().ArraySize) {}
          
          Texture2DArray() noexcept = default;
          Texture2DArray(const Texture2DArray&) = delete;
          Texture2DArray(Texture2DArray&& rhs) noexcept : Texture2D(std::move(rhs)), _arraySize(rhs._arraySize) {}
          Texture2DArray& operator=(const Texture2DArray&) = delete;
          Texture2DArray& operator=(Texture2DArray&& rhs) noexcept {
            Texture2D::operator=(std::move(rhs));
            _arraySize = rhs._arraySize;
            return *this;
          }
          ~Texture2DArray() noexcept = default;
          
          inline uint8_t arraySize() const noexcept { return this->_arraySize; } ///< Get number of sub-textures in array
          
        private:
          uint8_t _arraySize = 0;
        };
        
        
        // -- texture render-targets --
        
        /// @class TextureTarget2D
        /// @brief Texture resource used as a render-target (examples: mini-map, image for TV mesh, screenshots...).
        /// @warning Texture render-targets do NOT support 'staging' usage mode.
        /// @remarks To use a texture target as a polygon texture, copy content (after rendering) in a Texture2D with static/dynamic usage and bind it to renderer.
        /// @remarks To use a texture target for screenshots, copy content (after rendering) in a Texture2D with 'staging' usage and read from it.
        class TextureTarget2D final {
        public:
          /// @brief Create texture resource and view from params
          /// @param isShaderResource Allow directly using the rendered texture target also as a shader texture resource (for some polygons, for example).
          ///                         Without this, a copy to another texture (shader resource, not render-target) is required, and this texture can be used with polygons.
          ///                         This is also necessary to auto-generate mip levels with the renderer.
          ///                         Note: allowing direct use as shader resource may lead to poor memory optimization and slower access.
          /// @warning - Usage for TextureTarget2D can't be 'immutable' nor 'staging'. Default 'staticGpu' usage recommended.
          ///          - 'initData' is an array: if arraySize > 1, it must contain one index for each sub-resource.
          ///          - To only initialize one item, set 'initData' to NULL and use TextureWriter.
          ///          - If mip level is different from '1' in params, 'initData' must be NULL.
          ///          - The length of each entry in 'initData' must be rowBytes*height (no mips).
          /// @throws runtime_error on creation failure
          inline TextureTarget2D(Renderer& renderer, Texture2DParams& params, const uint8_t** initData = nullptr, bool isShaderResource = false)
            : TextureTarget2D(renderer, params.descriptor(), isShaderResource ? &(params.viewDescriptor()) : nullptr, 
                              params.texelBytes(), initData) {}
          
          TextureTarget2D() noexcept = default;
          TextureTarget2D(const TextureTarget2D&) = delete;
          TextureTarget2D(TextureTarget2D&& rhs) noexcept
            : _texture(rhs._texture), _resourceView(rhs._resourceView), _writeMode(rhs._writeMode), _rowBytes(rhs._rowBytes),
              _width(rhs._width), _height(rhs._height), _mipLevels(rhs._mipLevels) { rhs._texture=nullptr; rhs._renderTargetView=nullptr; rhs._resourceView=nullptr; }
          TextureTarget2D& operator=(const TextureTarget2D&) = delete;
          TextureTarget2D& operator=(TextureTarget2D&& rhs) noexcept {
            release();
            _texture=rhs._texture; _resourceView=rhs._resourceView;
            _writeMode=rhs._writeMode; _rowBytes=rhs._rowBytes; _width=rhs._width; _height=rhs._height; _mipLevels=rhs._mipLevels;
            rhs._texture=nullptr; rhs._renderTargetView=nullptr; rhs._resourceView=nullptr;
            return *this;
          }
          inline ~TextureTarget2D() noexcept { release(); }
          void release() noexcept; ///< Destroy resources
          
          inline TextureHandle2D handle() const noexcept { return this->_texture; } ///< Get Direct3D texture handle
          inline TextureView resourceView() const noexcept { return this->_resourceView; } ///< Get texture resource view handle (or NULL if isShaderResource was false)
          inline RenderTargetView getRenderTargetView() const noexcept { return this->_renderTargetView; } ///< Get texture render-target view
        
          inline bool isEmpty() const noexcept { return (this->_texture == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          inline uint32_t rowBytes() const noexcept { return this->_rowBytes; }        ///< Get byte size of one row of texels
          inline uint32_t width() const noexcept { return this->_width; }              ///< Get render-target width
          inline uint32_t height() const noexcept { return this->_height; }            ///< Get render-target height (==number of rows per texture)
          inline uint8_t mipLevels() const noexcept { return this->_mipLevels; }       ///< Get number of mip levels
          inline D3D11_MAP writeMode() const noexcept { return this->_writeMode; }     ///< Get mappable write mode (or 0 if static/immutable)
          
          /// @brief Generate texture mip-map levels (after rendering into it)
          /// @param renderer  Renderer used in constructor.
          /// @warning - Should not be called if 'isShaderResource' was set to 'false' in constructor.
          ///          - Useless if 'mipLevels' was set to '1' in Texture2DParams.
          inline void generateMips(Renderer& renderer) const noexcept { 
            if (this->_resourceView != nullptr)
              renderer.context()->GenerateMips(this->_resourceView); 
          }
          
        private:
          TextureTarget2D(Renderer& renderer, D3D11_TEXTURE2D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC* viewDescriptor,
                          uint32_t texelBytes, const uint8_t** initData);
        private:
          TextureHandle2D _texture = nullptr;
          RenderTargetView _renderTargetView = nullptr;
          TextureView _resourceView = nullptr;
          D3D11_MAP _writeMode = (D3D11_MAP)0;
          uint32_t _rowBytes = 0;
          uint32_t _width = 0;
          uint32_t _height = 0;
          uint8_t _mipLevels = 0;
        };
      }
    }
  }

#endif
