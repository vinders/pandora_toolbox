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
# ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable : 26451)
# endif
# include <gtest/gtest.h>
# include <memory>
# include <video/d3d11/renderer.h>
# include <video/d3d11/texture.h>

  using namespace pandora::video::d3d11;
  using namespace pandora::video;

  class D3d11TextureTest : public testing::Test {
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
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine128_B() noexcept {
    static int8_t line[] = {
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
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
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


  // -- texture params/buffer --

  TEST_F(D3d11TextureTest, textureParamsAccessors) {
    EXPECT_EQ((uint32_t)1u, Texture1DParams::maxMipLevels(1u));
    EXPECT_EQ((uint32_t)2u, Texture1DParams::maxMipLevels(2u));
    EXPECT_EQ((uint32_t)4u, Texture1DParams::maxMipLevels(8u));
    EXPECT_EQ((uint32_t)6u, Texture1DParams::maxMipLevels(32u));
    EXPECT_EQ((uint32_t)8u, Texture1DParams::maxMipLevels(128u));

    Texture1DParams tex1D(128u, DataFormat::rgba8_sRGB, 1u, 2u, 1u, ResourceUsage::immutable);
    EXPECT_EQ((UINT)128u, tex1D.descriptor().Width);
    EXPECT_EQ(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, tex1D.descriptor().Format);
    EXPECT_EQ((UINT)1u, tex1D.descriptor().ArraySize);
    EXPECT_EQ((UINT)2u, tex1D.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURE1D, tex1D.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)1u, tex1D.viewDescriptor().Texture1D.MostDetailedMip);
    EXPECT_EQ(D3D11_USAGE_IMMUTABLE, tex1D.descriptor().Usage);
    EXPECT_EQ((UINT)0, tex1D.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)4u, tex1D.texelBytes());

    Texture2DParams tex2D(128u, 64u, DataFormat::bgra8_unorm, 2u, 1u, 0u, ResourceUsage::dynamicCpu);
    EXPECT_EQ((UINT)128u, tex2D.descriptor().Width);
    EXPECT_EQ((UINT)64u, tex2D.descriptor().Height);
    EXPECT_EQ(DXGI_FORMAT_B8G8R8A8_UNORM, tex2D.descriptor().Format);
    EXPECT_EQ((UINT)2u, tex2D.descriptor().ArraySize);
    EXPECT_EQ((UINT)1u, tex2D.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURE2DARRAY, tex2D.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)0, tex2D.viewDescriptor().Texture2DArray.MostDetailedMip);
    EXPECT_EQ((UINT)2u, tex2D.viewDescriptor().Texture2DArray.ArraySize);
    EXPECT_EQ(D3D11_USAGE_DYNAMIC, tex2D.descriptor().Usage);
    EXPECT_EQ((UINT)0, tex2D.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)4u, tex2D.texelBytes());

    TextureCube2DParams texCube(128u, 64u, DataFormat::r32_f, 3u, 4u, 0u, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, texCube.descriptor().Width);
    EXPECT_EQ((UINT)64u, texCube.descriptor().Height);
    EXPECT_EQ(DXGI_FORMAT_R32_FLOAT, texCube.descriptor().Format);
    EXPECT_EQ((UINT)18u, texCube.descriptor().ArraySize);
    EXPECT_EQ((UINT)4u, texCube.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURECUBEARRAY, texCube.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)0, texCube.viewDescriptor().TextureCubeArray.MostDetailedMip);
    EXPECT_EQ((UINT)3u, texCube.viewDescriptor().TextureCubeArray.NumCubes);
    EXPECT_EQ(D3D11_USAGE_DEFAULT, texCube.descriptor().Usage);
    EXPECT_EQ((UINT)D3D11_RESOURCE_MISC_TEXTURECUBE, texCube.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)4u, texCube.texelBytes());

    Texture3DParams tex3D(128u, 64u, 32u, DataFormat::rgba16_f_scRGB, 2u, 1u, ResourceUsage::staging);
    EXPECT_EQ((UINT)128u, tex3D.descriptor().Width);
    EXPECT_EQ((UINT)64u, tex3D.descriptor().Height);
    EXPECT_EQ((UINT)32u, tex3D.descriptor().Depth);
    EXPECT_EQ(DXGI_FORMAT_R16G16B16A16_FLOAT, tex3D.descriptor().Format);
    EXPECT_EQ((UINT)2u, tex3D.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURE3D, tex3D.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)1u, tex3D.viewDescriptor().Texture3D.MostDetailedMip);
    EXPECT_EQ(D3D11_USAGE_STAGING, tex3D.descriptor().Usage);
    EXPECT_EQ((UINT)0, tex3D.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)8u, tex3D.texelBytes());

    tex1D.size(256u).texelFormat(DataFormat::rgba16_f_scRGB).arrayLength(2u, 4u, 0u).usage(ResourceUsage::staging).flags(true);
    EXPECT_EQ((UINT)256u, tex1D.descriptor().Width);
    EXPECT_EQ(DXGI_FORMAT_R16G16B16A16_FLOAT, tex1D.descriptor().Format);
    EXPECT_EQ((UINT)2u, tex1D.descriptor().ArraySize);
    EXPECT_EQ((UINT)4u, tex1D.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURE1DARRAY, tex1D.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)0u, tex1D.viewDescriptor().Texture1DArray.MostDetailedMip);
    EXPECT_EQ((UINT)2u, tex1D.viewDescriptor().Texture1DArray.ArraySize);
    EXPECT_EQ(D3D11_USAGE_STAGING, tex1D.descriptor().Usage);
    EXPECT_EQ((UINT)D3D11_RESOURCE_MISC_SHARED, tex1D.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)8u, tex1D.texelBytes());

    tex2D.size(256u, 128u).texelFormat(DataFormat::rgba16_f_scRGB).arrayLength(1u, 4u, 1u).usage(ResourceUsage::staging).flags(true);
    EXPECT_EQ((UINT)256u, tex2D.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex2D.descriptor().Height);
    EXPECT_EQ(DXGI_FORMAT_R16G16B16A16_FLOAT, tex2D.descriptor().Format);
    EXPECT_EQ((UINT)1u, tex2D.descriptor().ArraySize);
    EXPECT_EQ((UINT)4u, tex2D.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURE2D, tex2D.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)1u, tex2D.viewDescriptor().Texture2D.MostDetailedMip);
    EXPECT_EQ(D3D11_USAGE_STAGING, tex2D.descriptor().Usage);
    EXPECT_EQ((UINT)D3D11_RESOURCE_MISC_SHARED, tex2D.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)8u, tex2D.texelBytes());

    texCube.size(256u, 128u).texelFormat(DataFormat::rgba16_f_scRGB).arrayLength(1u, 4u, 1u).usage(ResourceUsage::staging).flags(true);
    EXPECT_EQ((UINT)256u, texCube.descriptor().Width);
    EXPECT_EQ((UINT)128u, texCube.descriptor().Height);
    EXPECT_EQ(DXGI_FORMAT_R16G16B16A16_FLOAT, texCube.descriptor().Format);
    EXPECT_EQ((UINT)6u, texCube.descriptor().ArraySize);
    EXPECT_EQ((UINT)4u, texCube.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURECUBE, texCube.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)1u, texCube.viewDescriptor().Texture2D.MostDetailedMip);
    EXPECT_EQ(D3D11_USAGE_STAGING, texCube.descriptor().Usage);
    EXPECT_EQ((UINT)(D3D11_RESOURCE_MISC_SHARED|D3D11_RESOURCE_MISC_TEXTURECUBE), texCube.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)8u, texCube.texelBytes());

    tex3D.size(256u, 128u, 64u).texelFormat(DataFormat::bgra8_sRGB).mips(4u, 0u).usage(ResourceUsage::staticGpu).flags(true);
    EXPECT_EQ((UINT)256u, tex3D.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex3D.descriptor().Height);
    EXPECT_EQ((UINT)64u, tex3D.descriptor().Depth);
    EXPECT_EQ(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, tex3D.descriptor().Format);
    EXPECT_EQ((UINT)4u, tex3D.descriptor().MipLevels);
    EXPECT_EQ(D3D11_SRV_DIMENSION_TEXTURE3D, tex3D.viewDescriptor().ViewDimension);
    EXPECT_EQ((UINT)0u, tex3D.viewDescriptor().Texture3D.MostDetailedMip);
    EXPECT_EQ(D3D11_USAGE_DEFAULT, tex3D.descriptor().Usage);
    EXPECT_EQ((UINT)D3D11_RESOURCE_MISC_SHARED, tex3D.descriptor().MiscFlags);
    EXPECT_EQ((uint32_t)4u, tex3D.texelBytes());
  }

  TEST_F(D3d11TextureTest, textureNoInit) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    Texture1DParams tex1Dparams(128u, DataFormat::rgba8_sRGB, 1u, 0, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex1Dparams.descriptor().Width);
    EXPECT_EQ((uint32_t)4u, tex1Dparams.texelBytes());
    Texture1D empty1D;
    EXPECT_TRUE(empty1D.isEmpty());
    Texture1D tex1D(renderer, tex1Dparams);
    EXPECT_FALSE(tex1D.isEmpty());
    EXPECT_TRUE(tex1D.handle() != nullptr);
    EXPECT_TRUE(tex1D.resourceView() != nullptr);
    EXPECT_EQ((uint32_t)512u, tex1D.rowBytes());
    EXPECT_EQ(Texture1DParams::maxMipLevels(128u), tex1D.mipLevels());
    EXPECT_EQ((D3D11_MAP)0, tex1D.writeMode());
    tex1Dparams.usage(ResourceUsage::staging);
    Texture1D tex1Ds(renderer, tex1Dparams);
    EXPECT_FALSE(tex1Ds.isEmpty());
    EXPECT_TRUE(tex1Ds.handle() != nullptr);
    EXPECT_TRUE(tex1Ds.resourceView() == nullptr);
    EXPECT_EQ((uint32_t)512u, tex1Ds.rowBytes());
    EXPECT_EQ(Texture1DParams::maxMipLevels(128u), tex1Ds.mipLevels());
    EXPECT_NE((D3D11_MAP)0, tex1Ds.writeMode());

    Texture2DParams tex2Dparams(128u, 128u, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::dynamicCpu);
    EXPECT_EQ((UINT)128u, tex2Dparams.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex2Dparams.descriptor().Height);
    EXPECT_EQ((uint32_t)4u, tex2Dparams.texelBytes());
    Texture2D empty2D;
    EXPECT_TRUE(empty2D.isEmpty());
    Texture2D tex2D(renderer, tex2Dparams);
    EXPECT_FALSE(tex2D.isEmpty());
    EXPECT_TRUE(tex2D.handle() != nullptr);
    EXPECT_TRUE(tex2D.resourceView() != nullptr);
    EXPECT_EQ((uint32_t)512u, tex2D.rowBytes());
    EXPECT_EQ((uint32_t)1u, tex2D.mipLevels());
    EXPECT_NE((D3D11_MAP)0, tex2D.writeMode());
    tex2Dparams.usage(ResourceUsage::staging).arrayLength(1u, 0, 0);
    Texture2D tex2Ds(renderer, tex2Dparams);
    EXPECT_FALSE(tex2Ds.isEmpty());
    EXPECT_TRUE(tex2Ds.handle() != nullptr);
    EXPECT_TRUE(tex2Ds.resourceView() == nullptr);
    EXPECT_EQ((uint32_t)512u, tex2Ds.rowBytes());
    EXPECT_EQ(Texture2DParams::maxMipLevels(128u, 128u), tex2Ds.mipLevels());
    EXPECT_NE((D3D11_MAP)0, tex2Ds.writeMode());

    Texture3DParams tex3Dparams(128u, 128u, 128u, DataFormat::rgba8_sRGB, 1u, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex3Dparams.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex3Dparams.descriptor().Height);
    EXPECT_EQ((UINT)128u, tex3Dparams.descriptor().Depth);
    EXPECT_EQ((uint32_t)4u, tex3Dparams.texelBytes());
    Texture3D empty3D;
    EXPECT_TRUE(empty3D.isEmpty());
    Texture3D tex3D(renderer, tex3Dparams);
    EXPECT_FALSE(tex3D.isEmpty());
    EXPECT_TRUE(tex3D.handle() != nullptr);
    EXPECT_TRUE(tex3D.resourceView() != nullptr);
    EXPECT_EQ((uint32_t)512u, tex3D.rowBytes());
    EXPECT_EQ((uint32_t)1u, tex3D.mipLevels());
    EXPECT_EQ((D3D11_MAP)0, tex3D.writeMode());
    tex3Dparams.usage(ResourceUsage::staging);
    Texture3D tex3Ds(renderer, tex3Dparams);
    EXPECT_FALSE(tex3Ds.isEmpty());
    EXPECT_TRUE(tex3Ds.handle() != nullptr);
    EXPECT_TRUE(tex3Ds.resourceView() == nullptr);
    EXPECT_EQ((uint32_t)512u, tex3Ds.rowBytes());
    EXPECT_EQ((uint32_t)1u, tex3Ds.mipLevels());
    EXPECT_NE((D3D11_MAP)0, tex3Ds.writeMode());

    TextureCube2DParams texCubeparams(128u, 128u, DataFormat::rgba8_sRGB, 1u, 0, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, texCubeparams.descriptor().Width);
    EXPECT_EQ((UINT)128u, texCubeparams.descriptor().Height);
    EXPECT_EQ((uint32_t)4u, texCubeparams.texelBytes());
    Texture2DArray emptyCube2D;
    EXPECT_TRUE(emptyCube2D.isEmpty());
    Texture2DArray texCube2D(renderer, texCubeparams);
    EXPECT_FALSE(texCube2D.isEmpty());
    EXPECT_TRUE(texCube2D.handle() != nullptr);
    EXPECT_TRUE(texCube2D.resourceView() != nullptr);
    EXPECT_EQ((uint32_t)512u, texCube2D.rowBytes());
    EXPECT_EQ(Texture2DParams::maxMipLevels(128u, 128u), texCube2D.mipLevels());
    EXPECT_EQ((D3D11_MAP)0, texCube2D.writeMode());
    EXPECT_EQ((uint8_t)6u, texCube2D.arraySize());
    texCubeparams.usage(ResourceUsage::staging);
    Texture2DArray texCube2Ds(renderer, texCubeparams);
    EXPECT_FALSE(texCube2Ds.isEmpty());
    EXPECT_TRUE(texCube2Ds.handle() != nullptr);
    EXPECT_TRUE(texCube2Ds.resourceView() == nullptr);
    EXPECT_EQ((uint32_t)512u, texCube2Ds.rowBytes());
    EXPECT_EQ(Texture2DParams::maxMipLevels(128u, 128u), texCube2Ds.mipLevels());
    EXPECT_NE((D3D11_MAP)0, texCube2Ds.writeMode());
    EXPECT_EQ((uint8_t)6u, texCube2D.arraySize());

    Texture2DParams tex2DArrayParams(128u, 128u, DataFormat::rgba8_sRGB, 2u, 1u, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex2DArrayParams.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex2DArrayParams.descriptor().Height);
    EXPECT_EQ((uint32_t)4u, tex2DArrayParams.texelBytes());
    Texture2DArray tex2DArray(renderer, tex2DArrayParams);
    EXPECT_FALSE(tex2DArray.isEmpty());
    EXPECT_TRUE(tex2DArray.handle() != nullptr);
    EXPECT_TRUE(tex2DArray.resourceView() != nullptr);
    EXPECT_EQ((uint32_t)512u, tex2DArray.rowBytes());
    EXPECT_EQ((uint32_t)1u, tex2DArray.mipLevels());
    EXPECT_EQ((D3D11_MAP)0, tex2DArray.writeMode());
    EXPECT_EQ((uint8_t)2u, tex2DArray.arraySize());
  }

  TEST_F(D3d11TextureTest, textureWithInit) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    const uint8_t* image1Ddata[] = { textureLine128_A(), textureLine64_A() };

    Texture1DParams tex1Dparams(128u, DataFormat::rgba8_sRGB, 1u, 2u, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex1Dparams.descriptor().Width);
    EXPECT_EQ((uint32_t)4u, tex1Dparams.texelBytes());
    Texture1D tex1D(renderer, tex1Dparams, image1Ddata);
    EXPECT_TRUE(tex1D.handle() != nullptr);
    EXPECT_TRUE(tex1D.resourceView() != nullptr);

    Texture1DParams tex1Dparams2(128u, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::dynamicCpu);
    EXPECT_EQ((UINT)128u, tex1Dparams2.descriptor().Width);
    EXPECT_EQ((uint32_t)4u, tex1Dparams2.texelBytes());
    Texture1D tex1D2(renderer, tex1Dparams2, image1Ddata);
    EXPECT_TRUE(tex1D2.handle() != nullptr);
    EXPECT_TRUE(tex1D2.resourceView() != nullptr);

    auto image2D = std::unique_ptr<uint8_t[]>(new uint8_t[128 * 128 * 4]());
    for (int i = 0; i < 128; ++i)
      memcpy(&image2D[128*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), 128*4);
    auto image2Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[64 * 64 * 4]());
    for (int i = 0; i < 64; ++i)
      memcpy(&image2Dmip[64*4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), 64*4);
    const uint8_t* image2Ddata[] = { &image2D[0], &image2Dmip[0] };

    Texture2DParams tex2Dparams(128u, 128u, DataFormat::rgba8_sRGB, 1u, 1u, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex2Dparams.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex2Dparams.descriptor().Height);
    EXPECT_EQ((uint32_t)4u, tex2Dparams.texelBytes());
    Texture2D tex2D(renderer, tex2Dparams, image2Ddata);
    EXPECT_TRUE(tex2D.handle() != nullptr);
    EXPECT_TRUE(tex2D.resourceView() != nullptr);

    Texture2DParams tex2Dparams2(128u, 128u, DataFormat::rgba8_sRGB, 1u, 2u, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex2Dparams2.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex2Dparams2.descriptor().Height);
    EXPECT_EQ((uint32_t)4u, tex2Dparams2.texelBytes());
    Texture2D tex2D2(renderer, tex2Dparams2, image2Ddata);
    EXPECT_TRUE(tex2D2.handle() != nullptr);
    EXPECT_TRUE(tex2D2.resourceView() != nullptr);

    tex2Dparams2.arrayLength(2u, 2u, 0);
    const uint8_t* image2Darraydata[] = { &image2D[0], &image2Dmip[0], &image2D[0], &image2Dmip[0] };
    Texture2D tex2D3(renderer, tex2Dparams2, image2Darraydata);
    EXPECT_TRUE(tex2D3.handle() != nullptr);
    EXPECT_TRUE(tex2D3.resourceView() != nullptr);

    auto image3D = std::unique_ptr<uint8_t[]>(new uint8_t[128 * 128 * 32 * 4]());
    for (int i = 0; i < 128*32; ++i)
      memcpy(&image3D[128*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), 128*4);
    auto image3Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[128 * 64 * 16 * 4]());
    for (int i = 0; i < 64*16; ++i)
      memcpy(&image3Dmip[64*4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), 64*4);
    const uint8_t* image3Ddata[] = { &image3D[0], &image3Dmip[0] };

    Texture3DParams tex3Dparams(128u, 128u, 32u, DataFormat::rgba8_sRGB, 2u, 0, ResourceUsage::staticGpu);
    EXPECT_EQ((UINT)128u, tex3Dparams.descriptor().Width);
    EXPECT_EQ((UINT)128u, tex3Dparams.descriptor().Height);
    EXPECT_EQ((UINT)32u, tex3Dparams.descriptor().Depth);
    EXPECT_EQ((uint32_t)4u, tex3Dparams.texelBytes());
    Texture3D tex3D(renderer, tex3Dparams, image3Ddata);
    EXPECT_TRUE(tex3D.handle() != nullptr);
    EXPECT_TRUE(tex3D.resourceView() != nullptr);
  }

# ifdef _MSC_VER
#   pragma warning(pop)
# endif
#endif
