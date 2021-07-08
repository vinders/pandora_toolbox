/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- texture reader -- --------------------------------------------------------

  void TextureReader::_readMapped1D(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, char* outputData) noexcept {
    memcpy(outputData, mapped.pData, (size_t)rowBytes);
  }
  
  void TextureReader::_readMapped2D(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, uint32_t sliceBytes, char* outputData) noexcept {
    if (mapped.RowPitch > rowBytes) { // resource extra row padding
      char* outEnd = outputData + sliceBytes;
      for (const char* src = (const char*)mapped.pData; outputData < outEnd; outputData += rowBytes, src += mapped.RowPitch)
        memcpy(outputData, src, (size_t)rowBytes);
    }
    else { // no padding -> straight copy
      if (rowBytes > mapped.RowPitch)
        rowBytes = mapped.RowPitch; // protect against user calculation errors -> avoids crashes
      memcpy(outputData, mapped.pData, (size_t)sliceBytes);
    }
  }
  
  void TextureReader::_readMapped3D(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, uint32_t sliceBytes, uint32_t depthSliceCount, char* outputData) noexcept {
    if (mapped.RowPitch > rowBytes) { // resource extra row padding
      if (sliceBytes > mapped.DepthPitch)
        sliceBytes = (mapped.DepthPitch / mapped.RowPitch) * rowBytes; // protect against user calculation errors -> avoids crashes
      uint32_t depthPadding = (uint32_t)mapped.DepthPitch - sliceBytes;
      
      for (const char* src = (const char*)mapped.pData; depthSliceCount; --depthSliceCount, src += depthPadding) {
        for (const char* outEnd = outputData + sliceBytes; outputData < outEnd; outputData += rowBytes, src += mapped.RowPitch)
          memcpy(outputData, src, (size_t)rowBytes);
      }
    }
    else if (mapped.DepthPitch > sliceBytes) { // resource padding per slice
      const char* outEnd = outputData + sliceBytes*depthSliceCount;
      for (const char* src = (const char*)mapped.pData; outputData < outEnd; outputData += sliceBytes, src += mapped.DepthPitch)
        memcpy(outputData, src, (size_t)rowBytes);
    }
    else { // no padding -> straight copy
      if (sliceBytes > mapped.DepthPitch)
        sliceBytes = mapped.DepthPitch; // protect against user calculation errors -> avoids crashes
      memcpy(outputData, mapped.pData, static_cast<size_t>(sliceBytes*depthSliceCount));
    }
  }
  
  // ---
  
  void TextureReader::_readRegionMapped(const D3D11_MAPPED_SUBRESOURCE& mapped, uint32_t rowBytes, const TextureCoords& sourceCoords, char* outputData) noexcept {
    if (rowBytes > mapped.RowPitch)
      rowBytes = mapped.RowPitch; // protect against user calculation errors -> avoids crashes
    uint32_t texelBytes = rowBytes/(sourceCoords.coords().right - sourceCoords.coords().left);
    
    if (sourceCoords.coords().back > 1) { // 3D
      const char* srcEndSlice = (const char*)mapped.pData + sourceCoords.coords().back*mapped.DepthPitch;
      for (const char* srcSlice = (const char*)mapped.pData + sourceCoords.coords().front*mapped.DepthPitch; srcSlice < srcEndSlice; srcSlice += mapped.DepthPitch) {
        
        const char* srcEndRow = srcSlice + sourceCoords.coords().bottom*mapped.RowPitch;
        for (const char* srcRow = srcSlice + sourceCoords.coords().top*mapped.RowPitch; srcRow < srcEndRow; srcRow += mapped.RowPitch, outputData += rowBytes)
          memcpy(outputData, srcRow + sourceCoords.coords().left*texelBytes, (size_t)rowBytes);
      }
    }
    else { // 2D
      const char* srcEndRow = (const char*)mapped.pData + sourceCoords.coords().bottom*mapped.RowPitch;
      for (const char* srcRow = (const char*)mapped.pData + sourceCoords.coords().top*mapped.RowPitch; srcRow < srcEndRow; srcRow += mapped.RowPitch, outputData += rowBytes)
        memcpy(outputData, srcRow + sourceCoords.coords().left*texelBytes, (size_t)rowBytes);
    }
  }


// -- texture writer -- ----------------------------------------------------

  void TextureWriter::_writeMapped1D(const char* sourceData, uint32_t rowBytes, D3D11_MAPPED_SUBRESOURCE& mapped) noexcept {
    memcpy(mapped.pData, sourceData, (size_t)rowBytes);
  }
  
  void TextureWriter::_writeMapped2D(const char* sourceData, uint32_t rowBytes, uint32_t sliceBytes, 
                                     D3D11_MAPPED_SUBRESOURCE& mapped) noexcept {
    if (mapped.RowPitch > rowBytes) { // resource extra row padding
      const char* srcEnd = sourceData + sliceBytes;
      for (char* dest = (char*)mapped.pData; sourceData < srcEnd; sourceData += rowBytes, dest += mapped.RowPitch)
        memcpy(dest, sourceData, (size_t)rowBytes);
    }
    else { // no padding -> straight copy
      if (rowBytes > mapped.RowPitch)
        rowBytes = mapped.RowPitch; // protect against user calculation errors -> avoids crashes
      memcpy(mapped.pData, sourceData, (size_t)sliceBytes);
    }
  }
  
  void TextureWriter::_writeMapped3D(const char* sourceData, uint32_t rowBytes, uint32_t sliceBytes, 
                                     uint32_t depthSliceCount, D3D11_MAPPED_SUBRESOURCE& mapped) noexcept {
    if (mapped.RowPitch > rowBytes) { // resource extra row padding
      if (sliceBytes > mapped.DepthPitch)
        sliceBytes = (mapped.DepthPitch / mapped.RowPitch) * rowBytes; // protect against user calculation errors -> avoids crashes
      uint32_t depthPadding = (uint32_t)mapped.DepthPitch - sliceBytes;
      
      for (char* dest = (char*)mapped.pData; depthSliceCount; --depthSliceCount, dest += depthPadding) {
        for (const char* srcEnd = sourceData + sliceBytes; sourceData < srcEnd; sourceData += rowBytes, dest += mapped.RowPitch)
          memcpy(dest, sourceData, (size_t)rowBytes);
      }
    }
    else if (mapped.DepthPitch > sliceBytes) { // resource padding per slice
      const char* srcEnd = sourceData + sliceBytes*depthSliceCount;
      for (char* dest = (char*)mapped.pData; sourceData < srcEnd; sourceData += sliceBytes, dest += mapped.DepthPitch)
        memcpy(dest, sourceData, (size_t)rowBytes);
    }
    else { // no padding -> straight copy
      if (sliceBytes > mapped.DepthPitch)
        sliceBytes = mapped.DepthPitch; // protect against user calculation errors -> avoids crashes
      memcpy(mapped.pData, sourceData, static_cast<size_t>(sliceBytes*depthSliceCount));
    }
  }

#endif
