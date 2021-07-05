/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./renderer.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
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
          inline Texture1DParams(uint32_t width, DataFormat format = DataFormat::rgba8_sRGB, uint32_t arraySize = 1u, 
                                 uint32_t mipLevels = 0, uint32_t mostDetailedViewedMip = 0, ResourceUsage usageType = ResourceUsage::staticGpu) noexcept {
            ZeroMemory(&_params, sizeof(D3D11_TEXTURE1D_DESC));
            ZeroMemory(&_viewParams, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            
            size(width);
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams);
            arrayLength(arraySize, mipLevels, mostDetailedViewedMip);
            _setTextureUsage(usageType, _params);
          }
          
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
          inline Texture1DParams& arrayLength(uint32_t arraySize, uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept {
            _params.ArraySize = (UINT)arraySize;
            _params.MipLevels = (UINT)mipLevels;
            if (arraySize <= 1u) {
              _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
              _viewParams.Texture1D.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
              _viewParams.Texture1D.MostDetailedMip = (UINT)mostDetailedViewedMip;
            }
            else {
              _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
              _viewParams.Texture1DArray.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
              _viewParams.Texture1DArray.MostDetailedMip = (UINT)mostDetailedViewedMip;
              _viewParams.Texture1DArray.ArraySize = _params.ArraySize;
            }
            return *this;
          }
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
          inline Texture2DParams(uint32_t width, uint32_t height, DataFormat format = DataFormat::rgba8_sRGB,
                                 uint32_t arraySize = 1u, uint32_t mipLevels = 0, uint32_t mostDetailedViewedMip = 0,
                                 ResourceUsage usageType = ResourceUsage::staticGpu) noexcept {
            ZeroMemory(&_params, sizeof(D3D11_TEXTURE2D_DESC));
            ZeroMemory(&_viewParams, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            _params.SampleDesc.Count = 1;
            
            size(width, height);
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams);
            arrayLength(arraySize, mipLevels, mostDetailedViewedMip);
            _setTextureUsage(usageType, _params);
          }
          
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
          
          /// @brief Set number of sub-textures: array length + mip-map level
          /// @param arraySize  Number of sub-textures in texture array (if value > 1, params only usable to create Texture2DArray instances)
          /// @param mipLevels  Generated mip-map levels: value from 1 (none) to N (1 + floor(log2(max_dimension))), or 0 to use max available levels.
          /// @param mostDetailedViewedMip  Highest detail mip level allowed when viewing texture (from 0 (highest) to 'mipLevels'-1 (lowest)).
          inline Texture2DParams& arrayLength(uint32_t arraySize, uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept {
            _params.ArraySize = (UINT)arraySize;
            _params.MipLevels = (UINT)mipLevels;
            if (arraySize <= 1u) {
              _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
              _viewParams.Texture2D.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
              _viewParams.Texture2D.MostDetailedMip = (UINT)mostDetailedViewedMip;
            }
            else {
              _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
              _viewParams.Texture2DArray.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
              _viewParams.Texture2DArray.MostDetailedMip = (UINT)mostDetailedViewedMip;
              _viewParams.Texture2DArray.ArraySize = _params.ArraySize;
            }
            return *this;
          }
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
          inline TextureCube2DParams(uint32_t width, uint32_t height, DataFormat format = DataFormat::rgba8_sRGB,
                                     uint32_t nbCubes = 1u, uint32_t mipLevels = 0, uint32_t mostDetailedViewedMip = 0,
                                     ResourceUsage usageType = ResourceUsage::staticGpu) noexcept {
            ZeroMemory(&_params, sizeof(D3D11_TEXTURE2D_DESC));
            ZeroMemory(&_viewParams, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            _params.SampleDesc.Count = 1;
            
            size(width, height);
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams);
            arrayLength(nbCubes, mipLevels, mostDetailedViewedMip);
            _setTextureUsage(usageType, _params);
          }
          
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
          inline TextureCube2DParams& arrayLength(uint32_t nbCubes, uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept {
            _params.ArraySize = (UINT)nbCubes * 6u;
            _params.MipLevels = (UINT)mipLevels;
            if (nbCubes <= 1u) {
              _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
              _viewParams.TextureCube.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
              _viewParams.TextureCube.MostDetailedMip = (UINT)mostDetailedViewedMip;
            }
            else {
              _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
              _viewParams.TextureCubeArray.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
              _viewParams.TextureCubeArray.MostDetailedMip = (UINT)mostDetailedViewedMip;
              _viewParams.TextureCubeArray.NumCubes = (UINT)nbCubes;
            }
            return *this;
          }
          /// @brief Compute max available mip level (1 + floor(log2(max_dimension)))
          static inline uint32_t maxMipLevels(uint32_t width, uint32_t height) noexcept { return Texture2DParams::maxMipLevels(width, height); }

          /// @brief Choose memory usage for texture: 'staticGpu' (or 'immutable') recommended, unless the texture data is updated regularly.
          ///        With 'immutable', resource data can only be set when building texture (texture constructor with 'initData').
          ///        Staging mode allows CPU read/write operations, but the texture can't be used for display in shaders
          ///        (staging content is typically copied to/from other 'staticGpu' texture).
          inline TextureCube2DParams& usage(ResourceUsage type) noexcept { _setTextureUsage(type, _params); return *this; }
          /// @brief Texture resource data can be shared between multiple Renderer/devices (should only be enabled for multi-device rendering).
          inline TextureCube2DParams& flags(bool isShared) noexcept { _params.MiscFlags = isShared ? (UINT)D3D11_RESOURCE_MISC_SHARED : 0; return *this; }
          
          
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
          inline Texture3DParams(uint32_t width, uint32_t height, uint32_t depth, DataFormat format = DataFormat::rgba8_sRGB, 
                                 uint32_t mipLevels = 0, uint32_t mostDetailedViewedMip = 0, ResourceUsage usageType = ResourceUsage::staticGpu) noexcept {
            ZeroMemory(&_params, sizeof(D3D11_TEXTURE3D_DESC));
            ZeroMemory(&_viewParams, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            _viewParams.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            
            size(width, height, depth);
            this->_texelBytes = _setTextureFormat(format, _params, _viewParams);
            mips(mipLevels, mostDetailedViewedMip);
            _setTextureUsage(usageType, _params);
          }
          
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
          inline Texture3DParams& mips(uint32_t mipLevels, uint32_t mostDetailedViewedMip = 0) noexcept {
            _params.MipLevels = (UINT)mipLevels;
            _viewParams.Texture3D.MipLevels = _params.MipLevels ? (UINT)mipLevels - 1 : UINT(-1);
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
        /// @warning For texture arrays, prefer Texture1DArray instead (to be able to write data per sub-texture).
        class Texture1D {
        public:
          /// @brief Create texture resource and view from params
          /// @warning 'initData' is required with 'immutable' usage
          /// @throws runtime_error on creation failure
          inline Texture1D(Renderer& renderer, const Texture1DParams& params, const void* initData = nullptr)
            : Texture1D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData) {}
          
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
                    uint32_t texelBytes, const void* initData);
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
        /// @warning For texture arrays, prefer Texture2DArray instead (to be able to write data per sub-texture).
        class Texture2D {
        public:
          /// @brief Create texture resource and view from params
          /// @warning 'initData' is required with 'immutable' usage
          /// @throws runtime_error on creation failure
          inline Texture2D(Renderer& renderer, const Texture2DParams& params, const void* initData = nullptr)
            : Texture2D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData) {}
          
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
          Texture2D(DeviceHandle device, const D3D11_TEXTURE2D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor,
                    uint32_t texelBytes, const void* initData);
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
          /// @warning 'initData' is required with 'immutable' usage
          /// @throws runtime_error on creation failure
          inline Texture3D(Renderer& renderer, const Texture3DParams& params, const void* initData = nullptr)
            : Texture3D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData) {}
          
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
                    uint32_t texelBytes, const void* initData);
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
          /// @warning 'initData' is required with 'immutable' usage
          /// @throws runtime_error on creation failure
          inline Texture1DArray(Renderer& renderer, const Texture1DParams& params, const void* initData = nullptr)
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
          /// @warning 'initData' is required with 'immutable' usage
          /// @throws runtime_error on creation failure
          inline Texture2DArray(Renderer& renderer, const Texture2DParams& params, const void* initData = nullptr)
            : Texture2D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData),
              _arraySize((uint8_t)params.descriptor().ArraySize) {}
          /// @brief Create texture resource array and view from params - 2D cube texture (or array of 2D cubes)
          /// @warning 'initData' is required with 'immutable' usage
          /// @throws runtime_error on creation failure
          inline Texture2DArray(Renderer& renderer, const TextureCube2DParams& params, const void* initData = nullptr)
            : Texture2D(renderer.device(), params.descriptor(), params.viewDescriptor(), params.texelBytes(), initData),
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
          ///                         Note: allowing direct use as shader resource can lead to poor memory optimization and slow access -> not recommended.
          /// @warning Usage for TextureTarget2D can't be 'immutable' nor 'staging'. Default 'staticGpu' usage recommended.
          /// @throws runtime_error on creation failure
          inline TextureTarget2D(Renderer& renderer, Texture2DParams& params, const void* initData = nullptr, bool isShaderResource = false)
            : TextureTarget2D(renderer.device(), params.descriptor(), isShaderResource ? &(params.viewDescriptor()) : nullptr, 
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
          TextureTarget2D(DeviceHandle device, D3D11_TEXTURE2D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC* viewDescriptor,
                          uint32_t texelBytes, const void* initData);
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
