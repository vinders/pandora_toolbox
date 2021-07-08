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
        /// @class TextureWriter
        /// @brief Raw texture data writer
        class TextureWriter final {
        public:
          TextureWriter() = delete;

          /// @brief Compute texture sub-resource index (mip level, array index) for write/writeStatic/writeMapped calls
          /// @param arrayIndex    Array index (Texture1D/2DArray) / Depth-slice index (Texture3D) / Always 0 if not an array (Texture1D/2D).
          /// @param mipIndex      Mip-level index, from 0 (most detailed) to 'mipLevelCount - 1' / Always 0 if no mip-mapping.
          /// @param mipLevelCount Total number of mip levels of a texture ('texture.mipLevels()').
          /// @remarks If texture is not an array, not a 3D texture, and has no mip levels: resource index is 0 (no need to call this function).
          static inline uint32_t toSubResourceIndex(uint32_t arrayIndex, uint32_t mipIndex, uint32_t mipLevelCount) noexcept {
            return D3D11CalcSubresource(mipIndex, arrayIndex, mipLevelCount);
          }
          
          
          // -- copy resource / sub-resource --
          
          /// @brief Copy texture into other texture with compatible data format (same components, same byte size), same dimensions and same mip levels.
          /// @param context  Device context of renderer used to create textures (renderer.context()).
          /// @param source   Texture to copy.
          /// @param output   Destination texture (compatible format, same dimensions, same mips required!)
          /// @remarks Typically used to fill a staging texture with the content of a static/dynamic texture (or vice-versa).
          /// @warning - Won't work if one of the textures is currently mapped (concurrent write/writeMapped call).
          static inline void copy(DeviceContext context, TextureHandle source, TextureHandle output) noexcept {
            context->CopyResource(output, source);
          }
          /// @brief Copy texture sub-resource into other sub-resource with compatible data format (same components, same byte size)
          ///        and compatible dimensions (source.dim + outputOffset.dim == output.dim, where dim is sizeX,sizeY,sizeZ).
          /// @param context             Device context of renderer used to create textures (renderer.context()).
          /// @param source              Texture to copy.
          /// @param srcSubResourceIndex Source texture sub-resource index (mip level, array/slice index): see toSubResourceIndex.
          /// @param output              Destination texture (compatible format, same dimensions, same mips required!)
          /// @param outSubResourceIndex Output texture sub-resource index (mip level, array/slice index): see toSubResourceIndex.
          /// @param outputOffset        Offset of copy in output sub-resource.
          /// @warning Won't work if one of the textures is currently mapped (concurrent write/writeMapped call).
          static inline void copy(DeviceContext context, TextureHandle source, uint32_t srcSubResourceIndex,
                           TextureHandle output, uint32_t outSubResourceIndex, const TexelPosition& outputOffset) noexcept {
            context->CopySubresourceRegion(output, (UINT)outSubResourceIndex, outputOffset.getX(), outputOffset.getY(), outputOffset.getZ(), 
                                           source, (UINT)srcSubResourceIndex, nullptr);
          }

          /// @brief Copy region of texture sub-resource into other sub-resource with compatible data format (same components, same byte size)
          ///        and compatible dimensions (sourceCoords.dimEnd - sourceCoords.dimBegin + outputOffset.dim == output.dim, where dim is sizeX,sizeY,sizeZ).
          /// @param context             Device context of renderer used to create textures (renderer.context()).
          /// @param source              Texture to copy.
          /// @param srcSubResourceIndex Source texture sub-resource index (mip level, array/slice index): see toSubResourceIndex.
          /// @param sourceCoords        Coordinates of the location to copy in source sub-resource.
          /// @param output              Destination texture (compatible format, same dimensions, same mips required!)
          /// @param outSubResourceIndex Output texture sub-resource index (mip level, array/slice index): see toSubResourceIndex.
          /// @param outputOffset        Offset of copy in output sub-resource.
          /// @warning Won't work if one of the textures is currently mapped (concurrent write/writeMapped call).
          static inline void copyRegion(DeviceContext context, TextureHandle source, uint32_t srcSubResourceIndex, const TextureCoords& sourceCoords,
                                 TextureHandle output, uint32_t outSubResourceIndex, const TexelPosition& outputOffset) noexcept {
            context->CopySubresourceRegion(output, (UINT)outSubResourceIndex, outputOffset.getX(), outputOffset.getY(), outputOffset.getZ(), 
                                                      source, (UINT)srcSubResourceIndex, &(sourceCoords.coords()));
          }
          
          
          // -- write texture with static usage --
          
          /// @brief Update content of a sub-resource of a texture created with 'staticGpu' usage (not mappable) -- source size == output size
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param sourceData       Data to copy into texture sub-resource: must have 'sliceBytes' byte size.
          /// @param rowBytes         Byte size of one row of texels (in source data and output sub-resource): adjusted for mip level.
          /// @param sliceBytes       Total byte size: row bytes * number of rows (in source data and output sub-resource): adjusted for mip level.
          /// @param output           Output texture containing the sub-resource to update (the whole content is rewritten).
          /// @param subResourceIndex Output texture sub-resource index (mip level, array/slice index): see toSubResourceIndex.
          /// @remarks - Texture3D: this only updates 1 depth-slice at a time: to update entire texture, call it for each slice (from 0 to ((texture.depth()-1) >> mipLevel)).
          ///          - When targeting mip levels smaller than the most detailed one, adjust rowBytes/height/depth (value >> mip level):
          ///            * rowAdjBytes = tx.rowBytes() >> mipLevel;
          ///            * sliceAdjBytes = rowAdjBytes * (tx.height() >> mipLevel); (only for Texture2D/3D: for Texture1D, sliceAdjBytes=rowAdjBytes).
          ///            * depthSliceCount = tx.depth() >> mipLevel;                (only for Texture3D: for Texture1D/2D, depthSliceCount=1).
          static inline void writeStatic(DeviceContext context, const void* sourceData, uint32_t rowBytes, uint32_t sliceBytes,
                                         TextureHandle output, uint32_t subResourceIndex) noexcept {
            context->UpdateSubresource(output, (UINT)subResourceIndex, nullptr, sourceData, (UINT)rowBytes, (UINT)sliceBytes);
          }
          
          /// @brief Update region of a sub-resource of a texture created with 'staticGpu' usage (not mappable) -- source size <= output size
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param sourceData       Data to copy into texture sub-resource: must have 'sliceBytes' byte size.
          /// @param rowBytes         Byte size of one row of texels (in source data and in output region defined by 'outputCoords'): adjusted for mip level.
          /// @param sliceBytes       Total byte size: row bytes * number of rows (in source data and in output region defined by 'outputCoords'): adjusted for mip level.
          /// @param output           Output texture containing the sub-resource to update.
          /// @param subResourceIndex Output texture sub-resource index (mip level, array/slice index): see toSubResourceIndex.
          /// @param outputCoords     Coordinates of the location to update in output sub-resource.
          /// @remarks - Texture3D: this only updates 1 depth-slice at a time: to update entire texture, call it for each slice (from 0 to ((texture.depth()-1) >> mipLevel)).
          ///          - When targeting mip levels smaller than the most detailed one, adjust rowBytes/height/depth (value >> mip level):
          ///            * rowAdjBytes = tx.rowBytes() >> mipLevel;
          ///            * sliceAdjBytes = rowAdjBytes * (tx.height() >> mipLevel); (only for Texture2D/3D: for Texture1D, sliceAdjBytes=rowAdjBytes).
          ///            * depthSliceCount = tx.depth() >> mipLevel;                (only for Texture3D: for Texture1D/2D, depthSliceCount=1).
          static inline void writeRegionStatic(DeviceContext context, const void* sourceData, uint32_t rowBytes, uint32_t sliceBytes,
                                               TextureHandle output, uint32_t subResourceIndex, const TextureCoords& outputCoords) noexcept {
            context->UpdateSubresource(output, (UINT)subResourceIndex, &(outputCoords.coords()), sourceData, (UINT)rowBytes, (UINT)sliceBytes);
          }

          
          // -- write texture with dynamic/staging usage --
          
          /// @brief Update content of a sub-resource of a 1D texture created with 'dynamicCpu'/'staging' usage (mappable) -- source size == output size
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param sourceData       Data to copy into texture sub-resource: must have 'rowBytes' byte size.
          /// @param rowBytes         Total byte size of row of texels (in source data): adjusted for mip level.
          /// @param output           Output 1D texture containing the sub-resource to update.
          /// @param subResourceIndex Output texture sub-resource index (mip level, array index): see toSubResourceIndex.
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust rowBytes (value >> mip level):
          ///          * rowAdjBytes = tx.rowBytes() >> mipLevel;
          static inline bool writeMapped(DeviceContext context, const void* sourceData, uint32_t rowBytes,
                                        Texture1D& output, uint32_t subResourceIndex) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(output.handle(), (UINT)subResourceIndex, output.writeMode(), 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _writeMapped1D((const char*)sourceData, rowBytes, mapped);
            context->Unmap(output.handle(), (UINT)subResourceIndex);
            return true;
          }
          /// @brief Update content of a sub-resource of a 2D texture created with 'dynamicCpu'/'staging' usage (mappable) -- source size == output size
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param sourceData       Data to copy into texture sub-resource: must have 'rowBytes' byte size.
          /// @param rowBytes         Byte size of one row of texels (in source data): adjusted for mip level.
          /// @param sliceBytes       Byte size of one depth-slice of texels: row bytes * number of rows (in source data): adjusted for mip level.
          /// @param output           Output 2D texture containing the sub-resource to update.
          /// @param subResourceIndex Output texture sub-resource index (mip level, array index): see toSubResourceIndex.
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust rowBytes/height (value >> mip level):
          ///          * rowAdjBytes = tx.rowBytes() >> mipLevel;
          ///          * sliceAdjBytes = rowAdjBytes * (tx.height() >> mipLevel);
          static inline bool writeMapped(DeviceContext context, const void* sourceData, uint32_t rowBytes, uint32_t sliceBytes,
                                        Texture2D& output, uint32_t subResourceIndex) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(output.handle(), (UINT)subResourceIndex, output.writeMode(), 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _writeMapped2D((const char*)sourceData, rowBytes, sliceBytes, mapped);
            context->Unmap(output.handle(), (UINT)subResourceIndex);
            return true;
          }
          /// @brief Update content of a sub-resource of a 3D texture created with 'dynamicCpu'/'staging' usage (mappable) -- source size == output size
          /// @param context          Device context of renderer used to create texture (renderer.context()).
          /// @param sourceData       Data to copy into texture sub-resource: must have 'rowBytes' byte size.
          /// @param rowBytes         Byte size of one row of texels (in source data): adjusted for mip level.
          /// @param sliceBytes       Byte size of one depth-slice of texels: row bytes * number of rows (in source data): adjusted for mip level.
          /// @param depthSliceCount  Total number of depth-slices (in source data and output texture): adjusted for mip level.
          /// @param output           Output 3D texture containing the sub-resource to update.
          /// @param subResourceIndex Output texture sub-resource index (mip level): see toSubResourceIndex.
          /// @remarks When targeting mip levels smaller than the most detailed one, adjust rowBytes/height/depth (value >> mip level):
          ///          * rowAdjBytes = tx.rowBytes() >> mipLevel;
          ///          * sliceAdjBytes = rowAdjBytes * (tx.height() >> mipLevel);
          ///          * depthSliceCount = tx.depth() >> mipLevel;
          static inline bool writeMapped(DeviceContext context, const void* sourceData, uint32_t rowBytes, uint32_t sliceBytes,
                                        uint32_t depthSliceCount, Texture3D& output, uint32_t subResourceIndex) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped; // lock GPU access
            ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
            if (FAILED(context->Map(output.handle(), (UINT)subResourceIndex, output.writeMode(), 0, &mapped)) || mapped.pData == nullptr)
              return false;

            _writeMapped3D((const char*)sourceData, rowBytes, sliceBytes, depthSliceCount, mapped);
            context->Unmap(output.handle(), (UINT)subResourceIndex);
            return true;
          }
          
        private:
          static void _writeMapped1D(const char* sourceData, uint32_t rowBytes, D3D11_MAPPED_SUBRESOURCE& mapped) noexcept;
          static void _writeMapped2D(const char* sourceData, uint32_t rowBytes, uint32_t sliceBytes, D3D11_MAPPED_SUBRESOURCE& mapped) noexcept;
          static void _writeMapped3D(const char* sourceData, uint32_t rowBytes, uint32_t sliceBytes, uint32_t depthSliceCount, D3D11_MAPPED_SUBRESOURCE& mapped) noexcept;
        };
      }
    }
  }

#endif
