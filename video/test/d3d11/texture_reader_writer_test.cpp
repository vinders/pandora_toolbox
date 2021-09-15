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
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <memory>
# include <video/d3d11/renderer.h>
# include <video/d3d11/texture.h>
# include <video/d3d11/texture_reader.h>
# include <video/d3d11/texture_writer.h>

  using namespace pandora::video::d3d11;
  using namespace pandora::video;

  class D3d11TextureReaderWriterTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- data helpers --

  static const uint8_t* textureLine128_A() noexcept {
    static int8_t line[] = {
      '\x44','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\x77','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\x55','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine128_B() noexcept {
    static int8_t line[] = {
      '\xA0','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xAF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\x55','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine64_A() noexcept {
    static int8_t line[] = {
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine64_B() noexcept {
    static int8_t line[] = {
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF' };
    return (const uint8_t*)line;
  }


  // -- texture sizes + readers --

# define _TEXTURE_WIDTH  128
# define _TEXTURE_HEIGHT 128
# define _TEXTURE_DEPTH  64
# define _REGION_SIZE     32
# define _REGION_SRC_OFFSET_X 8
# define _REGION_SRC_OFFSET_Y 4
# define _REGION_SRC_OFFSET_Z 0
# define _REGION_OUT_OFFSET_X 4
# define _REGION_OUT_OFFSET_Y 8
# define _REGION_OUT_OFFSET_Z 0
# define _MAX_MIP_LEVELS      2

  static auto tex1Dreader = [](DeviceContext context, const uint8_t** imageArray, Texture1D& staging, uint32_t index, uint32_t mipLevel, bool isRegion) {
    if (isRegion) {
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_REGION_SIZE*4]());
      TextureReader::readRegionMapped(context, staging.handle(), index, _REGION_SIZE*4u, TextureCoords(_REGION_OUT_OFFSET_X, _REGION_SIZE), &output[0]);
      return (memcmp(&output[0], imageArray[mipLevel] + _REGION_SRC_OFFSET_X*4u, (size_t)_REGION_SIZE*4u) == 0);
    }
    else {
      uint32_t width = (_TEXTURE_WIDTH >> mipLevel);
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[width * 4]());
      TextureReader::readMapped(context, staging, index, width * 4u, &output[0]);
      return (memcmp(&output[0], imageArray[mipLevel], (size_t)width * 4u) == 0);
    }
  };
  static auto tex2Dreader = [](DeviceContext context, const uint8_t** imageArray, Texture2D& staging, uint32_t index, uint32_t mipLevel, bool isRegion) {
    if (isRegion) {
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_REGION_SIZE*_REGION_SIZE*4]());
      TextureReader::readRegionMapped(context, staging.handle(), index, _REGION_SIZE*4u, 
        TextureCoords(_REGION_OUT_OFFSET_X, _REGION_OUT_OFFSET_Y, _REGION_SIZE, _REGION_SIZE), &output[0]);
      for (int line = 0; line < _REGION_SIZE; ++line)
        if (memcmp(&output[0] + line*_REGION_SIZE*4, imageArray[mipLevel] + (_REGION_SRC_OFFSET_Y+line)*(_TEXTURE_WIDTH >> mipLevel)*4 + _REGION_SRC_OFFSET_X*4, (size_t)_REGION_SIZE*4u) != 0)
          return false;
      return true;
    }
    else {
      uint32_t width = (_TEXTURE_WIDTH >> mipLevel);
      uint32_t height = (_TEXTURE_HEIGHT >> mipLevel);
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[width * height * 4]());
      TextureReader::readMapped(context, staging, index, width * 4u, width * height * 4u, &output[0]);
      return (memcmp(&output[0], imageArray[mipLevel], (size_t)width * (size_t)height * 4u) == 0);
    }
  };
  static auto tex3Dreader = [](DeviceContext context, const uint8_t** imageArray, Texture3D& staging, uint32_t index, uint32_t mipLevel, bool isRegion) {
    if (isRegion) {
      uint32_t height = (_TEXTURE_HEIGHT >> mipLevel);
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_REGION_SIZE*_REGION_SIZE*_REGION_SIZE*4]());
      TextureReader::readRegionMapped(context, staging.handle(), index, _REGION_SIZE*4u, 
        TextureCoords(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y,_REGION_OUT_OFFSET_Z, _REGION_SIZE,_REGION_SIZE,_REGION_SIZE), &output[0]);
      for (int line = 0; line < _REGION_SIZE*_REGION_SIZE; ++line)
        if (memcmp(&output[0] + line*_REGION_SIZE*4, imageArray[mipLevel] + (_REGION_SRC_OFFSET_Z*height + _REGION_SRC_OFFSET_Y+line)*(_TEXTURE_WIDTH >> mipLevel)*4 + _REGION_SRC_OFFSET_X*4, (size_t)_REGION_SIZE*4u) != 0)
          return false;
      return true;
    }
    else {
      uint32_t width = (_TEXTURE_WIDTH >> mipLevel);
      uint32_t height = (_TEXTURE_HEIGHT >> mipLevel);
      uint32_t depth = (_TEXTURE_DEPTH >> mipLevel);
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[width * height * depth * 4]());
      TextureReader::readMapped(context, staging, index, width * 4u, width * height * 4u, depth, &output[0]);
      return (memcmp(&output[0], imageArray[mipLevel], (size_t)width * (size_t)height * (size_t)depth * 4u) == 0);
    }
  };
  static auto tex1DarrayReader = [](DeviceContext context, const uint8_t** imageArray, Texture1DArray& staging, uint32_t index, bool isRegion) {
    if (isRegion) {
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_REGION_SIZE*4]());
      TextureReader::readRegionMapped(context, staging.handle(), index, _REGION_SIZE*4u, TextureCoords(_REGION_OUT_OFFSET_X, _REGION_SIZE), &output[0]);
      return (memcmp(&output[0], imageArray[index] + _REGION_SRC_OFFSET_X*4u, (size_t)_REGION_SIZE*4u) == 0);
    }
    else {
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH * 4]());
      TextureReader::readMapped(context, staging, index, _TEXTURE_WIDTH * 4u, &output[0]);
      return (memcmp(&output[0], imageArray[index], (size_t)_TEXTURE_WIDTH * 4u) == 0);
    }
  };
  static auto tex2DarrayReader = [](DeviceContext context, const uint8_t** imageArray, Texture2DArray& staging, uint32_t index, bool isRegion) {
    if (isRegion) {
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_REGION_SIZE*_REGION_SIZE*4]());
      TextureReader::readRegionMapped(context, staging.handle(), index, _REGION_SIZE*4u, 
        TextureCoords(_REGION_OUT_OFFSET_X, _REGION_OUT_OFFSET_Y, _REGION_SIZE, _REGION_SIZE), &output[0]);
      for (int line = 0; line < _REGION_SIZE; ++line)
        if (memcmp(&output[0] + line*_REGION_SIZE*4, imageArray[index] + (_REGION_SRC_OFFSET_Y+line)*_TEXTURE_WIDTH*4 + _REGION_SRC_OFFSET_X*4, (size_t)_REGION_SIZE*4u) != 0)
          return false;
      return true;
    }
    else {
      auto output = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH * _TEXTURE_HEIGHT * 4]());
      TextureReader::readMapped(context, staging, index, _TEXTURE_WIDTH * 4u, _TEXTURE_WIDTH * _TEXTURE_HEIGHT * 4u, &output[0]);
      return (memcmp(&output[0], imageArray[index], (size_t)_TEXTURE_WIDTH * (size_t)_TEXTURE_HEIGHT * 4u) == 0);
    }
  };


  // -- texture copy/read/write --

  template <typename _TexType, typename _Params>
  void _validateTextureCopy(Renderer& renderer, _Params&& params, const uint8_t** initData, 
                            const TexelPosition& regionDestOffset, const TextureCoords& region, std::function<bool(DeviceContext,const uint8_t**,_TexType&,uint32_t,uint32_t,bool)> reader) {
    _TexType texture(renderer, params, initData);
    EXPECT_TRUE(texture.handle() != nullptr);
    bool staticSrc = params.descriptor().Usage != D3D11_USAGE_DYNAMIC;

    params.usage(ResourceUsage::staging);
    _TexType staging(renderer, params);
    _TexType staging2(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);
    EXPECT_TRUE(staging2.handle() != nullptr);

    if (staticSrc) {
      params.descriptor().MipLevels = 1;
      params.usage(ResourceUsage::dynamicCpu);
    }
    else
      params.usage(ResourceUsage::staticGpu);
    _TexType otherUsage(renderer, params);
    EXPECT_TRUE(otherUsage.handle() != nullptr);
    
    if (texture.mipLevels() == 1u)
      TextureWriter::copy(renderer.context(), texture.handle(), otherUsage.handle()); // copy all resources (static<->dynamic)
    TextureWriter::copy(renderer.context(), texture.handle(), staging.handle()); // copy all resources (staging, to verify content)
    for (uint32_t mip = 0; mip < texture.mipLevels(); ++mip) {
      EXPECT_TRUE(reader(renderer.context(), initData, staging, TextureWriter::toSubResourceIndex(0, mip, texture.mipLevels()), mip, false));
    }
    TextureWriter::copy(renderer.context(), staging.handle(), texture.handle()); // copy back

    for (uint32_t mip = 0; mip < texture.mipLevels(); ++mip) { // copy resource by resource
      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(0, mip, texture.mipLevels());
      if (mip == 0) {
        TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, otherUsage.handle(), resourceIndex, TexelPosition(0,0,0));
      }
      TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging2.handle(), resourceIndex, TexelPosition(0,0,0));
      EXPECT_TRUE(reader(renderer.context(), initData, staging2, resourceIndex, mip, false));
      TextureWriter::copy(renderer.context(), staging2.handle(), resourceIndex, texture.handle(), resourceIndex, TexelPosition(0,0,0));
    }

    for (uint32_t mip = 0; mip < texture.mipLevels(); ++mip) { // copy region resource by resource
      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(0, mip, texture.mipLevels());
      if (mip == 0) {
        TextureWriter::copyRegion(renderer.context(), texture.handle(), resourceIndex, region, otherUsage.handle(), resourceIndex, regionDestOffset);
      }
      TextureWriter::copyRegion(renderer.context(), texture.handle(), resourceIndex, region, staging.handle(), resourceIndex, regionDestOffset);
      EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, mip, true));
    }
  }

  template <typename _TexType, typename _Params>
  void _validateTextureArrayCopy(Renderer& renderer, _Params&& params, const uint8_t** initData,
                                 const TexelPosition& regionDestOffset, const TextureCoords& region, std::function<bool(DeviceContext,const uint8_t**,_TexType&,uint32_t,bool)> reader) {
    _TexType texture(renderer, params, initData);
    EXPECT_TRUE(texture.handle() != nullptr);

    params.usage(ResourceUsage::staging);
    _TexType staging(renderer, params);
    _TexType staging2(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);
    EXPECT_TRUE(staging2.handle() != nullptr);

    TextureWriter::copy(renderer.context(), texture.handle(), staging.handle()); // copy all
    for (uint32_t i = 0; i < texture.arraySize(); ++i) {
      EXPECT_TRUE(reader(renderer.context(), initData, staging, TextureWriter::toSubResourceIndex(i, 0, texture.mipLevels()), false));
    }
    TextureWriter::copy(renderer.context(), staging.handle(), texture.handle()); // copy back

    for (uint32_t i = 0; i < texture.arraySize(); ++i) { // copy resource by resource
      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(i, 0, texture.mipLevels());
      TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging2.handle(), resourceIndex, TexelPosition(0,0,0));
      EXPECT_TRUE(reader(renderer.context(), initData, staging2, resourceIndex, false));
      TextureWriter::copy(renderer.context(), staging2.handle(), resourceIndex, texture.handle(), resourceIndex, TexelPosition(0,0,0));
    }

    for (uint32_t i = 0; i < texture.arraySize(); ++i) { // copy region resource by resource
      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(i, 0, texture.mipLevels());
      TextureWriter::copyRegion(renderer.context(), texture.handle(), resourceIndex, region, staging.handle(), resourceIndex, regionDestOffset);
      EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, true));
    }
  }

  // ---

  template <typename _TexType, typename _Params>
  void _validateTextureWriteStatic(Renderer& renderer, _Params&& params, uint32_t height, uint32_t depth, const uint8_t** initData, 
                                   const TextureCoords& regionOutCoords, std::function<bool(DeviceContext,const uint8_t**,_TexType&,uint32_t,uint32_t,bool)> reader) {
    _TexType texture(renderer, params);
    EXPECT_TRUE(texture.handle() != nullptr);

    params.usage(ResourceUsage::staging);
    _TexType staging(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);

    for (uint32_t mip = 0; mip < texture.mipLevels(); ++mip) { // copy resource by resource
      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(0, mip, texture.mipLevels());
      uint32_t rowBytes = texture.rowBytes() >> mip;
      TextureWriter::writeStatic(renderer.context(), initData[mip], rowBytes, (height > 1) ? (rowBytes*height) >> (mip) : rowBytes, texture.handle(), resourceIndex);
      TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging.handle(), resourceIndex, TexelPosition(0,0,0));
      EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, mip, false));
    }

    for (uint32_t mip = 0; mip < texture.mipLevels(); ++mip) { // copy region
      uint32_t bufferHeight = (height > 1) ? _REGION_SIZE : 1;
      uint32_t bufferDepth = (depth > 1) ? _REGION_SIZE : 1;
      uint32_t srcOffset = _REGION_SRC_OFFSET_X * 4;
      if (height > 1)
        srcOffset += (texture.rowBytes() >> mip) * _REGION_SRC_OFFSET_Y;
      if (depth > 1)
        srcOffset += ((texture.rowBytes() >> mip)*(height >> mip)) * _REGION_SRC_OFFSET_Z;
      auto imageRegion = std::unique_ptr<uint8_t[]>(new uint8_t[_REGION_SIZE* bufferHeight* bufferDepth * 4]());
      for (uint32_t line = 0; line < bufferHeight*bufferDepth; ++line)
        memcpy(&imageRegion[_REGION_SIZE*4*line], initData[mip] + srcOffset + (texture.rowBytes() >> mip)*line, _REGION_SIZE*4);

      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(0, mip, texture.mipLevels());
      TextureWriter::writeRegionStatic(renderer.context(), &imageRegion[0], 
                                       _REGION_SIZE*4, (height > 1) ? (_REGION_SIZE*_REGION_SIZE*4) >> mip : _REGION_SIZE*4,
                                       texture.handle(), resourceIndex, regionOutCoords);
      TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging.handle(), resourceIndex, TexelPosition(0,0,0));
      EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, mip, true));
    }
  }

  void _validateTextureWriteDynamic(Renderer& renderer, Texture1DParams&& params, const uint8_t** initData,
                                    std::function<bool(DeviceContext,const uint8_t**,Texture1D&,uint32_t,uint32_t,bool)> reader) {
    Texture1D texture(renderer, params);
    EXPECT_TRUE(texture.handle() != nullptr);

    params.usage(ResourceUsage::staging);
    Texture1D staging(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);

    // copy resource
    uint32_t resourceIndex = 0;
    uint32_t rowBytes = texture.rowBytes();
    TextureWriter::writeMapped(renderer.context(), initData[0], rowBytes, texture, resourceIndex);
    TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging.handle(), resourceIndex, TexelPosition(0,0,0));
    EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, 0, false));
  }
  void _validateTextureWriteDynamic(Renderer& renderer, Texture2DParams&& params, const uint8_t** initData,
                                    std::function<bool(DeviceContext,const uint8_t**,Texture2D&,uint32_t,uint32_t,bool)> reader) {
    Texture2D texture(renderer, params);
    EXPECT_TRUE(texture.handle() != nullptr);

    params.usage(ResourceUsage::staging);
    Texture2D staging(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);

    // copy resource
    uint32_t resourceIndex = 0;
    uint32_t rowBytes = texture.rowBytes();
    TextureWriter::writeMapped(renderer.context(), initData[0], rowBytes, rowBytes*texture.height(), texture, resourceIndex);
    TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging.handle(), resourceIndex, TexelPosition(0,0,0));
    EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, 0, false));
  }
  void _validateTextureWriteDynamic(Renderer& renderer, Texture3DParams&& params, const uint8_t** initData, 
                                    std::function<bool(DeviceContext,const uint8_t**,Texture3D&,uint32_t,uint32_t,bool)> reader) {
    Texture3D texture(renderer, params);
    EXPECT_TRUE(texture.handle() != nullptr);

    params.usage(ResourceUsage::staging);
    Texture3D staging(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);

    // copy resource
    uint32_t resourceIndex = 0;
    uint32_t rowBytes = texture.rowBytes();
    TextureWriter::writeMapped(renderer.context(), initData[0], rowBytes, rowBytes*texture.height(), texture.depth(), texture, resourceIndex);
    TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging.handle(), resourceIndex, TexelPosition(0,0,0));
    EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, 0, false));
  }

  template <typename _TexType, typename _Params>
  void _validateTextureArrayWriteStatic(Renderer& renderer, _Params&& params, uint32_t height, const uint8_t** initData,
                                        std::function<bool(DeviceContext,const uint8_t**,_TexType&,uint32_t,bool)> reader) {
    _TexType texture(renderer, params);
    EXPECT_TRUE(texture.handle() != nullptr);

    params.usage(ResourceUsage::staging);
    _TexType staging(renderer, params);
    EXPECT_TRUE(staging.handle() != nullptr);

    for (uint32_t i = 0; i < texture.arraySize(); ++i) { // copy resource by resource
      uint32_t resourceIndex = TextureWriter::toSubResourceIndex(i, 0, texture.mipLevels());
      uint32_t rowBytes = texture.rowBytes();
      TextureWriter::writeStatic(renderer.context(), initData[i], rowBytes, rowBytes*height, texture.handle(), resourceIndex);
      TextureWriter::copy(renderer.context(), texture.handle(), resourceIndex, staging.handle(), resourceIndex, TexelPosition(0,0,0));
      EXPECT_TRUE(reader(renderer.context(), initData, staging, resourceIndex, false));
    }
  }

  // ---

  TEST_F(D3d11TextureReaderWriterTest, copyReadTexture) {
    const uint8_t* image1Ddata[] = { textureLine128_A(), textureLine64_A() };
    const uint8_t* image1Darray[] = { textureLine128_A(), textureLine128_A() };

    auto image2D = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT; ++i)
      memcpy(&image2D[_TEXTURE_WIDTH*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), _TEXTURE_WIDTH*4);
    auto image2Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT/4 * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT/2; ++i)
      memcpy(&image2Dmip[_TEXTURE_WIDTH/2 *4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), _TEXTURE_WIDTH/2 *4);
    const uint8_t* image2Ddata[] = { &image2D[0], &image2Dmip[0] };
    const uint8_t* image2Darray[] = { &image2D[0], &image2D[0] };
    const uint8_t* image2Dcube[] = { &image2D[0],  &image2D[0],  &image2D[0],  &image2D[0],  &image2D[0], &image2D[0] };

    auto image3D = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT*_TEXTURE_DEPTH * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT*_TEXTURE_DEPTH; ++i)
      memcpy(&image3D[_TEXTURE_WIDTH*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), _TEXTURE_WIDTH*4);
    auto image3Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT*_TEXTURE_DEPTH/8 * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT*_TEXTURE_DEPTH/4; ++i)
      memcpy(&image3Dmip[_TEXTURE_WIDTH/2 *4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), _TEXTURE_WIDTH/2 *4);
    const uint8_t* image3Ddata[] = { &image3D[0], &image3Dmip[0] };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    _validateTextureCopy<Texture1D>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::staticGpu), image1Ddata,
                                    TexelPosition(_REGION_OUT_OFFSET_X), TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SIZE), tex1Dreader);
    _validateTextureCopy<Texture1D>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 1u, _MAX_MIP_LEVELS, 0, ResourceUsage::staticGpu), image1Ddata,
                                    TexelPosition(_REGION_OUT_OFFSET_X), TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SIZE), tex1Dreader);
    _validateTextureCopy<Texture1D>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::dynamicCpu), image1Ddata,
                                    TexelPosition(_REGION_OUT_OFFSET_X), TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SIZE), tex1Dreader);
    _validateTextureArrayCopy<Texture1DArray>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 2u, 1u, 0, ResourceUsage::staticGpu), image1Darray,
                                              TexelPosition(_REGION_OUT_OFFSET_X), TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SIZE), tex1DarrayReader);

    _validateTextureCopy<Texture2D>(renderer, Texture2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB, 1u, _MAX_MIP_LEVELS, 0, ResourceUsage::staticGpu),
                                    image2Ddata, TexelPosition(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y),
                                    TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SRC_OFFSET_Y,_REGION_SIZE,_REGION_SIZE), tex2Dreader);
    _validateTextureCopy<Texture2D>(renderer, Texture2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::dynamicCpu),
                                    image2Ddata, TexelPosition(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y),
                                    TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SRC_OFFSET_Y,_REGION_SIZE,_REGION_SIZE), tex2Dreader);
    _validateTextureArrayCopy<Texture2DArray>(renderer, Texture2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB, 2u, 1u, 0, ResourceUsage::staticGpu),
                                              image2Darray, TexelPosition(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y),
                                              TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SRC_OFFSET_Y,_REGION_SIZE,_REGION_SIZE), tex2DarrayReader);
    _validateTextureArrayCopy<Texture2DArray>(renderer, TextureCube2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::staticGpu),
                                              image2Dcube, TexelPosition(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y),
                                              TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SRC_OFFSET_Y,_REGION_SIZE,_REGION_SIZE), tex2DarrayReader);

    _validateTextureCopy<Texture3D>(renderer, Texture3DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT,_TEXTURE_DEPTH, DataFormat::rgba8_sRGB, _MAX_MIP_LEVELS, 0, ResourceUsage::staticGpu),
                                    image3Ddata, TexelPosition(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y,_REGION_OUT_OFFSET_Z),
                                    TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SRC_OFFSET_Y,_REGION_SRC_OFFSET_Z,_REGION_SIZE,_REGION_SIZE,_REGION_SIZE), tex3Dreader);
    _validateTextureCopy<Texture3D>(renderer, Texture3DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT,_TEXTURE_DEPTH, DataFormat::rgba8_sRGB, 1u, 0, ResourceUsage::dynamicCpu),
                                    image3Ddata, TexelPosition(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y,_REGION_OUT_OFFSET_Z),
                                    TextureCoords(_REGION_SRC_OFFSET_X,_REGION_SRC_OFFSET_Y,_REGION_SRC_OFFSET_Z,_REGION_SIZE,_REGION_SIZE,_REGION_SIZE), tex3Dreader);
  }

  TEST_F(D3d11TextureReaderWriterTest, writeTexture) {
    const uint8_t* image1Ddata[] = { textureLine128_A(), textureLine64_A() };
    const uint8_t* image1Darray[] = { textureLine128_A(), textureLine128_A() };

    auto image2D = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT; ++i)
      memcpy(&image2D[_TEXTURE_WIDTH*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), _TEXTURE_WIDTH*4);
    auto image2Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT/4 * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT/2; ++i)
      memcpy(&image2Dmip[_TEXTURE_WIDTH/2 *4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), _TEXTURE_WIDTH/2 *4);
    const uint8_t* image2Ddata[] = { &image2D[0], &image2Dmip[0] };
    const uint8_t* image2Darray[] = { &image2D[0], &image2D[0] };
    const uint8_t* image2Dcube[] = { &image2D[0],  &image2D[0],  &image2D[0],  &image2D[0],  &image2D[0], &image2D[0] };

    auto image3D = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT*_TEXTURE_DEPTH * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT*_TEXTURE_DEPTH; ++i)
      memcpy(&image3D[_TEXTURE_WIDTH*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), _TEXTURE_WIDTH*4);
    auto image3Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[_TEXTURE_WIDTH*_TEXTURE_HEIGHT*_TEXTURE_DEPTH/8 * 4]());
    for (int i = 0; i < _TEXTURE_HEIGHT*_TEXTURE_DEPTH/4; ++i)
      memcpy(&image3Dmip[_TEXTURE_WIDTH/2 *4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), _TEXTURE_WIDTH/2 *4);
    const uint8_t* image3Ddata[] = { &image3D[0], &image3Dmip[0] };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    _validateTextureWriteStatic<Texture1D>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::staticGpu),
                                            1,1, image1Ddata, TextureCoords(_REGION_OUT_OFFSET_X,_REGION_SIZE), tex1Dreader);
    _validateTextureWriteStatic<Texture1D>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 1u, _MAX_MIP_LEVELS, 0, ResourceUsage::staticGpu), 
                                           1,1, image1Ddata, TextureCoords(_REGION_OUT_OFFSET_X,_REGION_SIZE), tex1Dreader);
    _validateTextureWriteDynamic(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::dynamicCpu), image1Ddata, tex1Dreader);
    _validateTextureArrayWriteStatic<Texture1DArray>(renderer, Texture1DParams(_TEXTURE_WIDTH, DataFormat::rgba8_sRGB, 2u, 1u, 0, ResourceUsage::staticGpu),
                                                     1, image1Darray, tex1DarrayReader);

    _validateTextureWriteStatic<Texture2D>(renderer, Texture2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB, 1u,
                                            _MAX_MIP_LEVELS, 0, ResourceUsage::staticGpu), _TEXTURE_HEIGHT,1, image2Ddata,
                                           TextureCoords(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y,_REGION_SIZE,_REGION_SIZE), tex2Dreader);
    _validateTextureWriteDynamic(renderer, Texture2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::dynamicCpu),
                                 image2Ddata, tex2Dreader);
    _validateTextureArrayWriteStatic<Texture2DArray>(renderer, Texture2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB,
                                                     2u, 1u, 0, ResourceUsage::staticGpu), _TEXTURE_HEIGHT, image2Darray, tex2DarrayReader);
    _validateTextureArrayWriteStatic<Texture2DArray>(renderer, TextureCube2DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT, DataFormat::rgba8_sRGB,
                                                     1u, 1u, 0, ResourceUsage::staticGpu), _TEXTURE_HEIGHT, image2Dcube, tex2DarrayReader);

    _validateTextureWriteStatic<Texture3D>(renderer, Texture3DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT,_TEXTURE_DEPTH, DataFormat::rgba8_sRGB,
                                                                     _MAX_MIP_LEVELS, 0, ResourceUsage::staticGpu),
                                           _TEXTURE_HEIGHT,_TEXTURE_DEPTH, image3Ddata,
                                           TextureCoords(_REGION_OUT_OFFSET_X,_REGION_OUT_OFFSET_Y,_REGION_OUT_OFFSET_Z,_REGION_SIZE,_REGION_SIZE,_REGION_SIZE), tex3Dreader);
    _validateTextureWriteDynamic(renderer, Texture3DParams(_TEXTURE_WIDTH,_TEXTURE_HEIGHT,_TEXTURE_DEPTH, DataFormat::rgba8_sRGB, 1u, 0, ResourceUsage::dynamicCpu),
                                 image3Ddata, tex3Dreader);
  }

#endif
