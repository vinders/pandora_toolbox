/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include "./_private/_texture_coords.h" // includes D3D11
# include "./texture.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class TextureReader
        /// @brief Raw staging texture data reader
        class TextureReader final {
        public:
          TextureReader() = delete;
          
          /// @brief Compute texture sub-resource index (mip level, array index) for readMapped calls
          /// @param arrayIndex    Array index (Texture1D/2DArray) / Depth-slice index (Texture3D) / Always 0 if not an array (Texture1D/2D).
          /// @param mipIndex      Mip-level index, from 0 (most detailed) to 'mipLevelCount - 1' / Always 0 if no mip-mapping.
          /// @param mipLevelCount Total number of mip levels of a texture ('texture.mipLevels()').
          /// @remarks If texture is not an array, not a 3D texture, and has no mip levels: resource index is 0 (no need to call this function).
          static inline uint32_t toSubResourceIndex(uint32_t arrayIndex, uint32_t mipIndex, uint32_t mipLevelCount) noexcept {
            return D3D11CalcSubresource(mipIndex, arrayIndex, mipLevelCount);
          }
          
          
          // -- read texture with staging usage --
        
          /// @brief Read content of a sub-resource of a 1D texture created with 'staging' usage (readable/mappable)
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param source           Source 1D texture containing the sub-resource to read.
          /// @param subResourceIndex Source texture sub-resource index (mip level, array index): see toSubResourceIndex.
          /// @param rowBytes         Total byte size of row of texels (in source data): adjusted for mip level.
          /// @param outputData       Data output: must have 'rowBytes' byte size.
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust rowBytes (value >> mip level):
          ///          * rowAdjBytes = tx.rowBytes() >> mipLevel;
          static inline bool readMapped(DeviceContext context, Texture1D& source, uint32_t subResourceIndex, uint32_t rowBytes,
                                        void* outputData) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(source.handle(), (UINT)subResourceIndex, D3D11_MAP_READ, 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _readMapped1D(mapped, rowBytes, (char*)outputData);
            context->Unmap(source.handle(), (UINT)subResourceIndex);
            return true;
          }
          /// @brief Read content of a sub-resource of a 2D texture created with 'staging' usage (readable/mappable)
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param source           Source 2D texture containing the sub-resource to read.
          /// @param subResourceIndex Source texture sub-resource index (mip level, array index): see toSubResourceIndex.
          /// @param rowBytes         Byte size of one row of texels: adjusted for mip level.
          /// @param sliceBytes       Byte size of one depth-slice of texels: row bytes * number of rows (in source data): adjusted for mip level.
          /// @param outputData       Data output: must have 'sliceBytes' byte size.
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust rowBytes/height (value >> mip level):
          ///          * rowAdjBytes = tx.rowBytes() >> mipLevel;
          ///          * sliceAdjBytes = rowAdjBytes * (tx.height() >> mipLevel);
          static inline bool readMapped(DeviceContext context, Texture2D& source, uint32_t subResourceIndex,
                                        uint32_t rowBytes, uint32_t sliceBytes, void* outputData) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(source.handle(), (UINT)subResourceIndex, D3D11_MAP_READ, 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _readMapped2D(mapped, rowBytes, sliceBytes, (char*)outputData);
            context->Unmap(source.handle(), (UINT)subResourceIndex);
            return true;
          }
          /// @brief Read content of a sub-resource of a 3D texture created with 'staging' usage (readable/mappable)
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param source           Source 3D texture containing the sub-resource to read.
          /// @param subResourceIndex Source texture sub-resource index (mip level, array index): see toSubResourceIndex.
          /// @param rowBytes         Byte size of one row of texels: adjusted for mip level.
          /// @param sliceBytes       Byte size of one depth-slice of texels: row bytes * number of rows: adjusted for mip level.
          /// @param depthSliceCount  Total number of depth-slices: adjusted for mip level.
          /// @param outputData       Data output: must have 'sliceBytes*depthSliceCount' byte size.
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust rowBytes/height/depth (value >> mip level):
          ///          * rowAdjBytes = tx.rowBytes() >> mipLevel;
          ///          * sliceAdjBytes = rowAdjBytes * (tx.height() >> mipLevel);
          ///          * depthSliceCount = tx.depth() >> mipLevel;
          static inline bool readMapped(DeviceContext context, Texture3D& source, uint32_t subResourceIndex, uint32_t rowBytes,
                                        uint32_t sliceBytes, uint32_t depthSliceCount, void* outputData) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(source.handle(), (UINT)subResourceIndex, D3D11_MAP_READ, 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _readMapped3D(mapped, rowBytes, sliceBytes, depthSliceCount, (char*)outputData);
            context->Unmap(source.handle(), (UINT)subResourceIndex);
            return true;
          }
          
          // ---
          
          /// @brief Read region of a sub-resource of a texture created with 'staging' usage (readable/mappable)
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param source           Source 1D/2D/3D texture containing the sub-resource to read.
          /// @param subResourceIndex Source texture sub-resource index (mip level, array index): see toSubResourceIndex.
          /// @param regionRowBytes   Byte size of one region-row of texels (based on reading width specified in 'sourceCoords').
          /// @param outputData       Data output: must have 'sliceBytes' byte size: adjusted for mip level.
          /// @param sourceCoords     X/Y/Z offsets and sizes to read in 'source': adjusted for mip level.
          /// @warning - Boundaries aren't verified: make sure x+width, y+height and z+depth aren't above texture size.
          ///          - outputData must be rowBytes*height*depth bytes.
          ///          - width in 'sourceCoords' must be the same as adjusted texture width (readRowBytes / bytes per texel).
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust readRowBytes/height (value >> mip level):
          ///          * regionRowAdjBytes = (tx.rowBytes()/fullWidth*readWidth) >> mipLevel;
          ///          * adjHeight = height >> mipLevel; (only for 2D/3D textures: for 1D, adjHeight=1).
          ///          * adjDepth = depth >> mipLevel; (only for 3D textures: for 1D/2D, adjDepth=1).
          static inline bool readRegionMapped(DeviceContext context, TextureHandle source, uint32_t subResourceIndex,
                                              uint32_t regionRowBytes, const TextureCoords& sourceCoords, void* outputData) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(source, (UINT)subResourceIndex, D3D11_MAP_READ, 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _readRegionMapped(mapped, regionRowBytes, sourceCoords, (char*)outputData);
            context->Unmap(source, (UINT)subResourceIndex);
            return true;
          }

          
        private:
          static void _readMapped1D(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, char* outputData) noexcept;
          static void _readMapped2D(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, uint32_t sliceBytes, char* outputData) noexcept;
          static void _readMapped3D(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, uint32_t sliceBytes, uint32_t depthSliceCount, char* outputData) noexcept;
          static void _readRegionMapped(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, const TextureCoords& sourceCoords, char* outputData) noexcept;
        };
      }
    }
  }

#endif
